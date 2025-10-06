#include "feas.hpp"

extern "C" {
#include "color.h"
#include "color_private.h"
#include "mwis.h"
}
#include <cfloat>
#include <chrono>

Stats dpcp_decide_feasibility(const Graph &graph_) {

  // Initial time instant
  auto start = std::chrono::high_resolution_clock::now();

  // First make a copy of the graph
  Graph g;
  graph_copy(graph_, g);
  GraphEnv genv(&g, true, false, false, true);

  // Remove edges whose endpoints do not belongs to the same Va and Vb
  Graph::edge_iterator ei, ei_end, next;
  boost::tie(ei, ei_end) = edges(g);
  for (next = ei; ei != ei_end; ei = next) {
    ++next;
    Vertex u = source(*ei, g);
    Vertex v = target(*ei, g);
    if (genv.graph[u].first != genv.graph[v].first &&
        genv.graph[u].second != genv.graph[v].second) {
      remove_edge(*ei, g);
    }
  }

  // Initalize stable environment
  MWISenv *mwis_env = NULL;
  COLORstable_initenv(&mwis_env, NULL, 0);

  // Intialize vectors of weights
  COLORNWT *mwis_pi = NULL;
  mwis_pi = (COLORNWT *)COLOR_SAFE_MALLOC(num_vertices(g), COLORNWT);
  for (size_t i = 0; i < num_vertices(g); ++i)
    mwis_pi[i] = 1;
  COLORNWT mwis_pi_scalef = INT_MAX; // Force optimality

  // Initialize edge array
  int ecount = 0;
  int *elist = (int *)malloc(sizeof(int) * 2 * num_edges(g));
  for (auto e : boost::make_iterator_range(edges(g))) {
    elist[2 * ecount] = genv.getId[source(e, g)];
    elist[2 * ecount++ + 1] = genv.getId[target(e, g)];
  }

  // Solve the MWIS problem up to optimality
  COLORset *newsets = NULL;
  int nnewsets = 0;
  COLORstable_wrapper(&mwis_env, &newsets, &nnewsets, num_vertices(genv.graph),
                      ecount, elist, mwis_pi, mwis_pi_scalef, 0, 0, 2);

  assert(nnewsets > 0);

  // Save stats
  Stats stats;
  stats.state =
      newsets[0].count == static_cast<int>(genv.nA) ? FEASIBLE : INFEASIBLE;
  stats.time = std::chrono::duration<double>(
                   std::chrono::high_resolution_clock::now() - start)
                   .count();
  stats.lb = newsets[0].count;

  // Free memory
  for (int i = 0; i < nnewsets; ++i)
    free(newsets[i].members);
  free(newsets);
  free(elist);
  free(mwis_pi);
  COLORstable_freeenv(&mwis_env);

  return stats;
}