#ifndef _LP_HPP_
#define _LP_HPP_

#include "col.hpp"
#include "cplex_env.hpp"
#include "graph.hpp"
extern "C" {
#include "color.h"
#include "color_private.h"
#include "mwis.h"
#include "stats.hpp"
}

#include <chrono>

#define EPSILON 0.00001 // 10e-5
#define TIMELIMIT 300.0 // 5 minutes
#define THRESHOLD 0.1
#ifndef N_BRANCHES
#define N_BRANCHES 2
#endif

// typedef struct Column {
//   std::set<PSet> elements;
//   Column(int n_best, const nodepnt *best_sol);
// } Column;

class LP {

public:
  LP(const Graph &graph);
  LP(const Graph &&graph);
  ~LP();

  // Optimize the linear relaxation by column generation
  [[nodiscard]] LP_STATE optimize();

  // Get objective value (after calling optimize)
  [[nodiscard]] double get_obj_value() const { return objVal; };

  // Get number of columns (after calling optimize)
  [[nodiscard]] size_t get_n_columns() const { return stables.size(); };

  // Save the optimal solution (after calling optimize)
  void save_solution(Col &col);

  // Branch (after calling optimize)
  void branch(std::vector<LP *> &branches);

private:
  Graph graph;                     // Input graph with vertices in TypeA x TypeB
  size_t nA, nB;                   // |A| and |B|
  std::map<TypeA, size_t> tyA2idA; // Map from TypeA to idA
  std::map<TypeB, size_t> tyB2idB; // Map from TypeB to idB
  std::vector<TypeA> idA2TyA;      // Map from idA to TypeA
  std::vector<TypeB> idB2TyB;      // Map from idB to TypeB
  std::vector<std::vector<int>> snd; // Map from idA to subset of Vertex:
                                     // snd[i_a] = {(a,b) \in V}
  std::vector<std::vector<int>> fst; // Map from idB to subset of Vertex:
                                     // fst[i_b] = {(a,b) \in V}
  std::vector<COLORset *> stables;   // Vector of columns (stable sets)
  std::vector<int> posVars;          // Vector of positive variables
  Vertex branchVar;                  // Branching variable
  double objVal;                     // Objective value

  bool isGCP; // Whether the instance is a graph coloring instance, i.e.
              // |snd[a]| = 1 forall a

  // Initialize the linear relaxation with an initial set of columns
  void initialize(CplexEnv &cenv);

  // Add a new column to the linear relaxation
  void add_column(CplexEnv &cenv, COLORset *newset);

  // Set CPLEX's parameters
  void set_parameters(CplexEnv &cenv, IloCplex &cplex);

  // Compute vertex weights from dual values
  // Return a boolean to indicate whether the weight of any vertex changed sign
  // from the previous iteration and the number of positive weights
  std::pair<bool, size_t> get_weights(std::vector<double> &weights,
                                      IloNumArray &duals);

  // Covert weights from double to int
  int double2COLORNWT(COLORNWT nweights[], COLORNWT *scalef, size_t nPosWeights,
                      const std::vector<double> &dbl_nweights);

  // Exact solve of a GCP instance
  LP_STATE solve_GCP();

  // Get branching variable
  size_t get_branching_variable(const IloNumArray &values);
};

#endif // _LP_HPP_
