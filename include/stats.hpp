enum LP_STATE {
  LP_UNSOLVED,
  LP_INFEASIBLE,
  LP_INTEGER,
  LP_FRACTIONAL,
  LP_TIME_OR_MEM_LIMIT
};

enum STATE { OPTIMAL, FEASIBLE, INFEASIBLE, TIME_OR_MEM_LIMIT, UNKNOWN };

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