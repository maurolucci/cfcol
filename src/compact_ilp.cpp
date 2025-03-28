#include "compact_ilp.hpp"

#include <map>
#include <string>

#include <ilcplex/cplex.h>
#include <ilcplex/ilocplex.h>

#define TIMELIMIT 900

Stats solve_ilp(Graph graph, size_t ncolors) {

  // Initialize cplex enviroment
  IloEnv cxenv;
  IloModel cxmodel(cxenv);
  IloArray<IloNumVarArray> x(cxenv, num_vertices(graph));
  IloNumVarArray w(cxenv, ncolors);
  IloConstraintArray cxcons(cxenv);

  // Define variables
  for (size_t v = 0; v < num_vertices(graph); ++v) {
    x[v] = IloNumVarArray(cxenv, ncolors);
    for (size_t k = 0; k < ncolors; ++k) {
      char name[100];
      snprintf(name, sizeof(name), "x_%ld_%ld", v, k);
      x[v][k] = IloBoolVar(cxenv, name);
    }
  }
  for (size_t k = 0; k < ncolors; ++k) {
    char name[100];
    snprintf(name, sizeof(name), "w_%ld", k);
    w[k] = IloBoolVar(cxenv, name);
  }

  // Define objective
  IloExpr fobj(cxenv, 0);
  for (size_t k = 0; k < ncolors; ++k)
    fobj += w[k];
  cxmodel.add(IloMinimize(cxenv, fobj));

  // Define snd: A -> P(AxB) / snd(a) = {(a',b) \in V: a = a'}
  std::map<int, std::vector<int>> snd;
  for (size_t v : boost::make_iterator_range(vertices(graph)))
    snd[graph[v].first].push_back(v);

  // Constraints

  // \sum_{(a,b) \in V} \sum_{k \in C} x_(a,b)_k \geq 1, forall a \in A
  for (auto &[a, vec] : snd) {
    IloExpr restr(cxenv);
    for (size_t v : vec)
      for (size_t k = 0; k < ncolors; ++k)
        restr += x[v][k];
    cxcons.add(restr >= 1);
  }

  // x_(a,b)_k + x_(a',b)_k' \leq 1, forall (a,b),(a',b) \in V with a != a',
  //                                         k, k' \in C with k != k'
  for (auto v1 : boost::make_iterator_range(vertices(graph))) {
    auto [a1, b1] = graph[v1];
    for (auto v2 : boost::make_iterator_range(vertices(graph))) {
      auto [a2, b2] = graph[v2];
      if ((b1 != b2) || (a1 == a2))
        continue;
      for (size_t k1 = 0; k1 < ncolors; ++k1)
        for (size_t k2 = 0; k2 < ncolors; ++k2) {
          if (k1 == k2)
            continue;
          IloExpr restr(cxenv);
          restr += x[v1][k1] + x[v2][k2];
          cxcons.add(restr <= 1);
        }
    }
  }

  // x_(a,b)_k + x_(a',b')_k \leq w_k, forall ((a,b),(a',b')) \in E, k \in C
  for (auto e : boost::make_iterator_range(edges(graph))) {
    auto u = source(e, graph);
    auto v = target(e, graph);
    for (size_t k = 0; k < ncolors; ++k) {
      IloExpr restr(cxenv);
      restr += x[u][k] + x[v][k] - w[k];
      cxcons.add(restr <= 0);
    }
  }

  cxmodel.add(cxcons);

  // Solve model
  IloCplex cplex(cxmodel);

  // Set parameters
  cplex.setDefaults();
  cplex.setParam(IloCplex::Param::TimeLimit, TIMELIMIT);

  // Solve
  cplex.solve();

  // Get final state
  STATE state;
  switch (cplex.getCplexStatus()) {
  case IloCplex::CplexStatus::Optimal:
    state = OPTIMAL;
    break;
  case IloCplex::CplexStatus::Feasible:
    state = FEASIBLE;
    break;
  case IloCplex::CplexStatus::Infeasible:
    state = INFEASIBLE;
    break;
  case IloCplex::CplexStatus::AbortTimeLim:
    state = TIME_OR_MEM_LIMIT;
    break;
  default:
    state = UNKNOWN;
    break;
  }

  // Print statics
  std::cout << std::endl << "*** Resultados ***" << std::endl;
  std::cout << "Numero de variables: " << cplex.getNcols() << std::endl;
  std::cout << "Numero de restricciones: " << cplex.getNrows() << std::endl;
  std::cout << "Estado: " << state << std::endl;
  std::cout << "Tiempo: " << cplex.getTime() << std::endl;
  std::cout << "Nodos: " << cplex.getNnodes() << std::endl;
  std::cout << "Cota inferior: " << cplex.getBestObjValue() << std::endl;
  if (state == IloCplex::CplexStatus::Optimal ||
      state == IloCplex::CplexStatus::Feasible) {
    std::cout << "Cota superior: " << cplex.getObjValue() << std::endl;
    std::cout << "Gap: " << cplex.getMIPRelativeGap() << std::endl;
  }

  Stats stats = {
      cplex.getNcols(),  cplex.getNrows(),        state, cplex.getTime(),
      cplex.getNnodes(), cplex.getBestObjValue(), 0,     0};
  if (state == IloCplex::CplexStatus::Optimal ||
      state == IloCplex::CplexStatus::Feasible) {
    stats.up = cplex.getObjValue();
    stats.gap = cplex.getMIPRelativeGap();
  }
}