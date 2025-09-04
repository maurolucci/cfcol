#include "col.hpp"
#include "graph.hpp"
#include "stats.hpp"

Stats dpcp_2_step_greedy_heur_min_degree_in_Vb(const GraphEnv &genv, Col &col);

Stats dpcp_2_step_greedy_heur_min_degree_in_B(const GraphEnv &genv, Col &col);

Stats dpcp_2_step_greedy_heur_min_degree_in_selected_B(const GraphEnv &genv,
                                                       Col &col);

Stats dpcp_2_step_greedy_heur_min_n_new_edges(const GraphEnv &genv, Col &col);

Stats dpcp_2_step_semigreedy_heur_min_degree_in_Vb(const GraphEnv &genv,
                                                   Col &col, size_t nIters);

Stats dpcp_2_step_semigreedy_heur_min_degree_in_B(const GraphEnv &genv,
                                                  Col &col, size_t nIters);

Stats dpcp_2_step_semigreedy_heur_min_degree_in_selected_B(const GraphEnv &genv,
                                                           Col &col,
                                                           size_t nIters);

Stats dpcp_2_step_semigreedy_heur_min_n_new_edges(const GraphEnv &genv,
                                                  Col &col, size_t nIters);

Stats heur_solve(const GraphEnv &genv, const std::vector<TypeA> &as, Col &col,
                 size_t repetitions, Pool &pool);