#ifndef _LP_HPP_
#define _LP_HPP_

#include "col.hpp"
#include "cplex_env.hpp"
#include "graph.hpp"
extern "C" {
#include "color.h"
#include "color_private.h"
#include "mwis.h"
}

#include <chrono>

#define EPSILON 0.00001 // 10e-5
#define TIMELIMIT 300.0 // 5 minutes
#define THRESHOLD 0.1
#ifndef N_BRANCHES
#define N_BRANCHES 2
#endif

enum LP_STATE { INFEASIBLE, INTEGER, FRACTIONAL, TIME_OR_MEM_LIMIT };

// typedef struct Column {
//   std::set<PSet> elements;
//   Column(int n_best, const nodepnt *best_sol);
// } Column;

class LP {

public:
  LP(const Graph &graph);
  LP(const Graph &&graph);

  // Optimize the linear relaxation by column generation
  [[nodiscard]] LP_STATE optimize();

  // Save the optimal solution
  void save_solution(Coloring &coloring);

  // Branch
  void branch(std::vector<LP *> &branches, Vertex v);

private:
  Graph graph;                     // Input graph with vertices in TypeA x TypeB
  std::map<TypeA, size_t> tyA2Col; // Map from TypeA to id of Constraint (>= 1)
  std::map<TypeB, size_t> tyB2Col; // Map from TypeB to id of Constraint (<= 1)
  std::vector<TypeB> col2TyB;      // Map from id of Constraint (<= 1) to TypeB
  std::map<TypeA, std::unordered_set<TypeB>> snd; // snd[a] = {b: (a,b) \in V}
  std::map<TypeB, std::unordered_set<TypeA>> fst; // fst[b] = {a: (a,b) \in V}
  Vertex branchVar;                               // Branching variable
  bool isGCP; // Whether the instance is a graph coloring instance, i.e.
              // |snd[a]| = 1 forall a
  Col col;    // Coloring enviroment

  // Initialize the linear relaxation with an initial set of columns
  void initialize(CplexEnv &cenv);

  // Add a new column to the linear relaxation
  void add_column(CplexEnv &cenv, const std::unordered_set<size_t> &cA,
                  const std::unordered_set<size_t> &cB);
  void add_column(CplexEnv &cenv, int count, const int *members);

  // Set CPLEX's parameters
  void set_parameters(CplexEnv &cenv, IloCplex &cplex);

  // Covert weights from double to int
  int double2COLORNWT(COLORNWT nweights[], COLORNWT *scalef,
                      const IloNumArray dbl_nweights);

  // Exact solve of a GCP instance
  LP_STATE solve_GCP();
};

#endif // _LP_HPP_
