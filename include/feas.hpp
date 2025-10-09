#include "graph.hpp"
#include "stats.hpp"

Stats dpcp_decide_feasibility_enumerative(const Graph &graph, Params &params,
                                          std::ostream &log);

Stats dpcp_decide_feasibility_ilp(const Graph &graph, Params &params,
                                  std::ostream &log);
