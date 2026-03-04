#include "col.hpp"
#include "graph.hpp"
#include "stats.hpp"

void dpcp_dsatur_heur(const GraphEnv &genv, VertexVector &selected,
                      std::map<TypeB, std::set<TypeB>> &adj, Col &col);

HeurStats dpcp_2_step_greedy_heur(const GraphEnv &genv, Col &col,
                                  size_t variant = 3);

HeurStats dpcp_2_step_semigreedy_heur(const GraphEnv &genv, Col &col,
                                      size_t nIters, size_t variant = 3);

HeurStats dpcp_1_step_greedy_heur(const GraphEnv &genv, Col &col);