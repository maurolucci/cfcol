#include "pricing.hpp"
#include "random.hpp"
#include <limits>

void ThresholdCallback::check_thresolhd(
    const IloCplex::Callback::Context &context) {
  if (context.isCandidatePoint())
    if (context.getCandidateObjective() > THRESHOLD) {
      // Save stable set
      IloNumArray valY(context.getEnv(), num_vertices(in.graph));
      context.getCandidatePoint(y, valY);
      IloNumArray valWa(context.getEnv(), in.nA);
      context.getCandidatePoint(wa, valWa);
      IloNumArray valWb(context.getEnv(), in.nB);
      context.getCandidatePoint(wb, valWb);
      for (auto v : boost::make_iterator_range(vertices(in.graph)))
        if (valY[v] > 0.5)
          stab.stable.push_back(v);
      for (size_t iA = 0; iA < in.nA; ++iA)
        if (valWa[iA] > 0.5)
          stab.as.insert(in.idA2TyA[iA]);
      for (size_t iB = 0; iB < in.nB; ++iB)
        if (valWb[iB] > 0.5)
          stab.bs.insert(in.idB2TyB[iB]);
      stab.cost = context.getCandidateObjective();
      // Abort execution
      context.abort();
    }
}

// Implementation of the invoke method.
//
// This is the method that we have to implement to fulfill the
// generic callback contract. CPLEX will call this method during
// the solution process at the places that we asked for.
void ThresholdCallback::invoke(const IloCplex::Callback::Context &context) {
  if (context.inCandidate())
    check_thresolhd(context);
}

PricingEnv::PricingEnv(GraphEnv &in)
    : in(in), stab(), cxenv(), cxmodel(cxenv), y(cxenv, num_vertices(in.graph)),
      wa(cxenv, in.nA), wb(cxenv, in.nB), cxcons(cxenv), cplex(cxenv),
      cb(in, stab, y, wa, wb), contextMask(0) {
  exact_init();
}

PricingEnv::~PricingEnv() {
  // End CPLEX variables
  cplex.end();
  cxcons.end();
  y.end();
  wa.end();
  wb.end();
  cxobj.end();
  cxmodel.end();
  cxenv.end();
}

void PricingEnv::exact_init() {
  // Variables
  for (auto v : boost::make_iterator_range(vertices(in.graph))) {
    char name[100];
    snprintf(name, sizeof(name), "y_%ld_%ld", in.graph[v].first,
             in.graph[v].second);
    y[v] = IloBoolVar(cxenv, name);
  }
  for (size_t iA = 0; iA < in.nA; ++iA) {
    char name[100];
    snprintf(name, sizeof(name), "wa_%ld", iA);
    wa[iA] = IloBoolVar(cxenv, name);
  }
  for (size_t iB = 0; iB < in.nB; ++iB) {
    char name[100];
    snprintf(name, sizeof(name), "wb_%ld", iB);
    wb[iB] = IloBoolVar(cxenv, name);
  }

  // Objective
  IloExpr obj(cxenv);
  for (size_t iA = 0; iA < in.nA; ++iA)
    obj += wa[iA] * 1.0;
  for (size_t iB = 0; iB < in.nB; ++iB)
    obj += wb[iB] * (-1.0);
  cxobj = IloMaximize(cxenv, obj);
  cxmodel.add(cxobj);
  obj.end();

  // Constraints
  // (1) \sum_{b \in B: (a,b) \in V} y_a_b <= 1, for all a \in A
  for (size_t ia = 0; ia < in.nA; ++ia) {
    IloExpr restr(cxenv);
    for (size_t v : in.snd[ia])
      restr += y[v];
    cxcons.add(restr <= 1);
    restr.end();
  }

  // (2) y_a_b + y_a'_b' <= 1, for all ((a,b),(a',b')) \in E such that a != a'
  for (auto e : boost::make_iterator_range(edges(in.graph))) {
    auto u = source(e, in.graph);
    auto v = target(e, in.graph);
    if (in.graph[u].first == in.graph[v].first)
      continue;
    IloExpr restr(cxenv);
    restr += y[u] + y[v];
    cxcons.add(restr <= 1);
    restr.end();
  }

  // (3a) y_a_b <= wa_a, for all (a,b) \in V
  for (auto v : boost::make_iterator_range(vertices(in.graph))) {
    IloExpr restr(cxenv);
    restr += y[v] - wa[in.tyA2idA[in.graph[v].first]];
    cxcons.add(restr <= 0);
    restr.end();
  }
  for (size_t iA = 0; iA < in.nA; ++iA) {
    IloExpr restr(cxenv);
    restr += wa[iA];
    for (auto v : in.snd[iA])
      restr -= y[v];
    cxcons.add(restr <= 0);
    restr.end();
  }

  // (3b) y_a_b <= wb_b, for all (a,b) \in V
  for (auto v : boost::make_iterator_range(vertices(in.graph))) {
    IloExpr restr(cxenv);
    restr += y[v] - wb[in.tyB2idB[in.graph[v].second]];
    cxcons.add(restr <= 0);
    restr.end();
  }
  for (size_t iB = 0; iB < in.nB; ++iB) {
    IloExpr restr(cxenv);
    restr += wb[iB];
    for (auto v : in.fst[iB])
      restr -= y[v];
    cxcons.add(restr <= 0);
    restr.end();
  }

  cxmodel.add(cxcons);

  // Re-export model
  cplex.extract(cxmodel);

  // Set parameters
  cplex.setDefaults();
  cplex.setParam(IloCplex::Param::Parallel, 1); // Deterministic mode
  cplex.setParam(IloCplex::Param::Threads, 1);  // Single thread
  cplex.setOut(cxenv.getNullStream());
  cplex.setWarning(cxenv.getNullStream());

  // Now we get to setting up the generic callback.
  // We instantiate a ThresholdCallback and set the contextMask parameter.
  contextMask |= IloCplex::Callback::Context::Id::Candidate;

  // If contextMask is not zero we add the callback.
  if (contextMask != 0)
    cplex.use(&cb, contextMask);
}

std::pair<StableEnv, PRICING_STATE> PricingEnv::exact_solve(IloNumArray &dualsA,
                                                            IloNumArray &dualsB,
                                                            double timelimit) {
  // Update objective coefficients
  cxobj.setLinearCoefs(wa, dualsA);
  cxobj.setLinearCoefs(wb, dualsB);
  cxmodel.add(cxobj);

  // Reset stable
  stab.stable.clear();
  stab.as.clear();
  stab.bs.clear();

  // Solve
  cplex.extract(cxmodel);
  cplex.setParam(IloCplex::Param::TimeLimit, timelimit);
  cplex.solve();

  // Get final state
  PRICING_STATE state;
  switch (cplex.getCplexStatus()) {
  case IloCplex::CplexStatus::Optimal:
    state = PRICING_OPTIMAL;
    break;
  case IloCplex::CplexStatus::AbortUser:
    state = PRICING_FEASIBLE;
    break;
  case IloCplex::CplexStatus::AbortTimeLim:
    state = PRICING_TIME_EXCEEDED;
    break;
  case IloCplex::CplexStatus::MemLimFeas:
  case IloCplex::CplexStatus::MemLimInfeas:
    state = PRICING_MEM_EXCEEDED;
    break;
  default:
    state = PRICING_OTHER;
    break;
  }

  // Recover solution
  if (state == PRICING_OPTIMAL) {
    IloNumArray valY(cxenv, num_vertices(in.graph));
    cplex.getValues(y, valY);
    IloNumArray valWa(cxenv, in.nA);
    cplex.getValues(wa, valWa);
    IloNumArray valWb(cxenv, in.nB);
    cplex.getValues(wb, valWb);
    for (auto v : boost::make_iterator_range(vertices(in.graph)))
      if (valY[v] > 0.5)
        stab.stable.push_back(v);
    for (size_t iA = 0; iA < in.nA; ++iA)
      if (valWa[iA] > 0.5)
        stab.as.insert(in.idA2TyA[iA]);
    for (size_t iB = 0; iB < in.nB; ++iB)
      if (valWb[iB] > 0.5)
        stab.bs.insert(in.idB2TyB[iB]);
    stab.cost = cplex.getBestObjValue();
  } else if (state == PRICING_FEASIBLE) {
    // Maximalize stable
    for (Vertex v : boost::make_iterator_range(vertices(in.graph))) {
      auto [a, b] = in.graph[v];
      if (dualsA[in.tyA2idA[a]] < PRICING_EPSILON || stab.as.contains(a))
        continue;
      if (!stab.bs.contains(b))
        continue;
      auto it = std::find_if(
          stab.stable.begin(), stab.stable.end(),
          [v, this](auto w) { return edge(v, w, in.graph).second; });
      if (it != stab.stable.end())
        continue;
      stab.stable.push_back(v);
      stab.as.insert(a);
      stab.cost += dualsA[in.tyA2idA[a]];
    }
  }

  return std::make_pair(stab, state);
}

std::pair<StableEnv, PRICING_STATE> PricingEnv::heur_solve(IloNumArray &dualsA,
                                                           IloNumArray &dualsB,
                                                           Vertex start_v) {

  // Reset stable
  stab.stable.clear();
  stab.as.clear();
  stab.bs.clear();
  stab.cost = 0.0;

  // Candidates
  std::list<std::pair<double, Vertex>> candidates;
  TypeA start_a = in.graph[start_v].first;
  TypeA start_b = in.graph[start_v].second;
  for (auto v : boost::make_iterator_range(vertices(in.graph))) {
    auto [a, b] = in.graph[v];
    double cost_a = dualsA[in.tyA2idA[a]];
    double cost_b = dualsB[in.tyB2idB[b]];
    if (v == start_v) {
      stab.stable.push_back(start_v);
      stab.as.insert(start_a);
      stab.bs.insert(start_b);
      stab.cost += cost_a + cost_b;
      continue;
    }
    if (a == start_a || edge(start_v, v, in.graph).second)
      continue;
    if (b == start_v) {
      candidates.push_back(std::make_pair(cost_a, v));
      continue;
    }
    candidates.push_back(std::make_pair(cost_a + cost_b, v));
  }

  while (!candidates.empty()) {

    // Find the best candidate
    auto it_v = std::max_element(candidates.begin(), candidates.end());

    // Discard best candidate with some low probability
    if (rand_double(rng) < 0.05) {
      candidates.erase(it_v);
      continue;
    }

    Vertex v = it_v->second;
    auto [a, b] = in.graph[v];

    // Add the best candidate to the stable set
    stab.stable.push_back(v);
    stab.as.insert(a);
    stab.bs.insert(b);
    stab.cost += it_v->first;

    // Remove and update weights in the candidate list
    for (auto it = candidates.begin(); it != candidates.end();) {
      Vertex u = it->second;
      auto [au, bu] = in.graph[u];
      if (au == a || edge(u, v, in.graph).second) {
        it = candidates.erase(it);
        continue;
      }
      if (bu == b)
        it->first = dualsA[in.tyA2idA[au]];
      ++it;
    }
  }

  return std::make_pair(stab, PRICING_FEASIBLE);
}