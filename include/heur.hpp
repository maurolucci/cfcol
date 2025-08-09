#include "col.hpp"
#include "graph.hpp"
#include "stats.hpp"

Stats dpcp_heur_1_step(const GraphEnv &genv, Col &col);

Stats heur_solve(const GraphEnv &genv, const std::vector<TypeA> &as, Col &col,
                 size_t repetitions, Pool &pool);