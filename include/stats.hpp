enum LP_STATE {
  LP_UNSOLVED,
  LP_INFEASIBLE,
  LP_INTEGER,
  LP_FRACTIONAL,
  LP_TIMELIMIT,
  LP_MEMLIMIT,
};

enum STATE { OPTIMAL, FEASIBLE, INFEASIBLE, TIMELIMIT, MEM_LIMIT, UNKNOWN };

struct Stats {
  size_t nvars;
  size_t ncons;
  STATE state;
  double time;
  size_t nodes;
  double lb;
  size_t up;
  double gap;
};