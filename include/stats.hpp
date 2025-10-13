#ifndef __STATS_HPP__
#define __STATS_HPP__

#include <iostream>
#include <limits>
#include <string>

enum PRICING_STATE {
  PRICING_STABLE_FOUND,
  PRICING_STABLE_NOT_FOUND,
  PRICING_STABLE_NOT_EXIST,
  PRICING_READY,
  PRICING_TIME_EXCEEDED,
  PRICING_MEM_EXCEEDED,
  PRICING_OTHER,
};

enum LP_STATE {
  LP_UNSOLVED,
  LP_INFEASIBLE,
  LP_INTEGER,
  LP_FRACTIONAL,
  LP_TIME_EXCEEDED,
  LP_TIME_EXCEEDED_PR,
  LP_MEM_EXCEEDED,
  LP_MEM_EXCEEDED_PR,
  LP_INIT_FAIL,
};

enum STATE {
  OPTIMAL,
  FEASIBLE,
  INFEASIBLE,
  TIME_EXCEEDED,
  TIME_EXCEEDED_LP,
  TIME_EXCEEDED_PR,
  MEM_EXCEEDED,
  MEM_EXCEEDED_LP,
  MEM_EXCEEDED_PR,
  INIT_FAIL,
  UNKNOWN,
};

class Stats {

public:
  // Instance name
  std::string instance;
  // Solver name
  std::string solver;
  // Run number
  int run;
  // Number of vertices
  int nvertices;
  // Number of edges
  int nedges;
  // Cardinality of A
  int nA;
  // Cardinality of B
  int nB;
  // Number of variables
  int nvars;
  // Number of constraints
  int ncons;
  // Final state
  STATE state;
  // Total time
  double time;
  // Number of processed nodes
  int nodes;
  // Number of nodes left in the queue
  int nodesLeft;
  // Value of the initial solution
  int initSol;
  // Time required to find the initial solution
  double initSolTime;
  // Value of the root node relaxation
  double rootval;
  // Final lower bound
  double lb;
  // Final upper bound
  int ub;
  // Final optimality gap (in percentage)
  double gap;
  // Number of infeasible instances detected in the byp tree
  int ninfeas;
  // Number of GCP instances detected in the byp tree
  int ngcp;
  // Time spent on solving GCP instances
  double gcpTime;
  // Size of the pool of columns
  int poolSize;
  // Time required for finding heuristic solutions in the whole byp tree
  double heurTime;
  // Number of calls to the feasibility check in the whole byp tree
  int nCallsFeas;
  // Time required for feasibility checks in the whole byp tree
  double feasTime;
  // For each pricing method, number of columns added, number of calls, and
  // total time required in the whole byp tree
  int nColsPool;
  int nColsHeur;
  int nColsMwis1;
  int nColsMwis2;
  int nColsExact;
  int nCallsPool;
  int nCallsHeur;
  int nCallsMWis1;
  int nCallsMWis2;
  int nCallsExact;
  double nTimePool;
  double nTimeHeur;
  double nTimeMwis1;
  double nTimeMwis2;
  double nTimeExact;

  Stats()
      : instance(""), solver(""), run(-1), nvertices(-1), nedges(-1), nA(-1),
        nB(-1), nvars(-1), ncons(-1), state(UNKNOWN), time(0.0), nodes(0),
        nodesLeft(0), initSol(-1), initSolTime(0.0), rootval(-1.0), lb(-1.0),
        ub(-1), gap(-1.0), ninfeas(0), ngcp(0), gcpTime(0.0), poolSize(-1),
        heurTime(0.0), nCallsFeas(0), feasTime(0.0), nColsPool(0), nColsHeur(0),
        nColsMwis1(0), nColsMwis2(0), nColsExact(0), nCallsPool(0),
        nCallsHeur(0), nCallsMWis1(0), nCallsMWis2(0), nCallsExact(0),
        nTimePool(0.0), nTimeHeur(0.0), nTimeMwis1(0.0), nTimeMwis2(0.0),
        nTimeExact(0.0) {}

  std::string get_state_as_str();
  void write_stats(std::ostream &file);
  void print_stats(std::ostream &file);
};

#endif // __STATS_HPP__