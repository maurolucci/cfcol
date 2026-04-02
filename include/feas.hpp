#include "col.hpp"
#include "graph.hpp"
#include "stats.hpp"

Stats dpcp_decide_feasibility_enumerative(const DPCPInst &dpcp, Col &col,
                                          std::ostream &log);

Stats dpcp_decide_feasibility_ilp(const DPCPInst &dpcp, Col &col, int timeLimit,
                                  std::ostream &log);
