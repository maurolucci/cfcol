#include "stats.hpp"

std::string Stats::get_state_as_str() {
  switch (state) {
  case OPTIMAL:
    return "OPTIMAL";
  case FEASIBLE:
    return "FEASIBLE";
  case INFEASIBLE:
    return "INFEASIBLE";
  case TIME_EXCEEDED:
    return "TIME_EXCEEDED";
  case MEM_EXCEEDED:
    return "MEM_EXCEEDED";
  case NODE_TIME_EXCEEDED:
    return "NODE_TIME_EXCEEDED";
  case NODE_MEM_EXCEEDED:
    return "NODE_MEM_EXCEEDED";
  default:
    return "UNKNOWN";
  }
}

void Stats::write_stats(std::ostream &file) {
  file << nvars << "," << ncons << "," << get_state_as_str() << "," << time
       << "," << nodes << "," << initSol << "," << lb << "," << ub << "," << gap
       << "," << poolSize << "," << ncolsPool << "," << ncolsHeur << ","
       << ncolsExact << std::endl;
}

void Stats::print_stats(std::ostream &file) {
  file << std::endl << "*** Stats ***" << std::endl;
  if (nvars != -1)
    file << "Variables: " << nvars << std::endl;
  if (ncons != -1)
    file << "Constraints: " << ncons << std::endl;
  file << "State: " << get_state_as_str() << std::endl;
  file << "Time: " << time << std::endl;
  if (nodes != -1)
    file << "Nodes: " << nodes << std::endl;
  if (initSol != -1)
    file << "Initial solution: " << initSol << std::endl;
  if (lb != -1.0)
    file << "Lower bound: " << lb << std::endl;
  if (ub != -1)
    file << "Upper bound: " << ub << std::endl;
  if (gap != -1)
    file << "Gap: " << gap << std::endl;
  if (poolSize != -1)
    file << "Size of pool: " << poolSize << std::endl;
  if (ncolsPool != -1)
    file << "Pool columns: " << ncolsPool << std::endl;
  if (ncolsHeur != -1)
    file << "Heuristic columns: " << ncolsHeur << std::endl;
  if (ncolsExact != -1)
    file << "Exact columns: " << ncolsExact << std::endl;
}