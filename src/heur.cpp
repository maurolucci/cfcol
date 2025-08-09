#include "heur.hpp"
#include "graph.hpp"
#include "random.hpp"
extern "C" {
#include "color.h"
}

#include <chrono>
#include <limits>
#include <queue>
#include <random>

using ClockType = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::_V2::system_clock::time_point;

// Vertex information
struct Heur1SVertexInfo {
  Color color;               // color of the vertex
  std::set<Color> adjColors; // adjacent colors
  int nreprs;                // -|V_a|
  int nincomp;               // -|N(v) \cap V^b|
  bool removed;              // whether the vertex has been removed
  size_t id;

  // Constructor
  Heur1SVertexInfo(const GraphEnv &genv, const Vertex &u)
      : color(-1), adjColors(), nincomp(0), removed(false) {
    TypeA a = genv.graph[u].first;
    TypeB b = genv.graph[u].second;
    nreprs = -genv.Va.at(a).size();
    for (Vertex v :
         boost::make_iterator_range(adjacent_vertices(u, genv.graph)))
      if (b == genv.graph[v].second)
        nincomp--;
    id = genv.getId.at(u);
  }

  void print_info() {
    std::cout << "id: " << id << ", color: " << color << ", adjColors: [";
    for (Color i : adjColors)
      std::cout << i << ",";
    std::cout << "], nreprs: " << -nreprs << ", nincomp: " << -nincomp
              << ", removed: " << removed << std::endl;
  }
};

// Function to find a free color in the the neighborhood of a vertex
Color get_free_color(const Graph &graph, const Vertex u,
                     const std::map<Vertex, Heur1SVertexInfo> &info,
                     std::vector<bool> &used) {

  // Mark the colors used in the neighborhood of u
  for (Vertex v : boost::make_iterator_range(adjacent_vertices(u, graph)))
    if (info.at(v).color != -1)
      used[info.at(v).color] = true;

  // Find the first available color for u
  Color i;
  for (i = 0; i < static_cast<int>(used.size()); i++)
    if (used[i] == false)
      break;

  // Unmark the colors used in the neighborhood of u
  for (Vertex v : boost::make_iterator_range(adjacent_vertices(u, graph)))
    if (info.at(v).color != -1)
      used[info.at(v).color] = false;

  return i;
}

// One-step heuristic for DPCP
// Based on a DSATUR implementation for GCP that runs in O((n + m) log n)
Stats dpcp_heur_1_step(const GraphEnv &genv, Col &col) {

  TimePoint start = ClockType::now();

  // Map with the necessary information of each vertex
  std::map<Vertex, Heur1SVertexInfo> info;

  // Comparison function between two vertices
  // First: the greatest number of adjacent colors
  // Second: the lowest size of |V_a|
  // Third: the lowest size of |N(v) \cap V^b|
  // Fourth: the greatest index
  auto cmp = [&info](const Vertex &u, const Vertex &v) {
    return std::tie(info.at(u).adjColors, info.at(u).nreprs, info.at(u).nincomp,
                    info.at(u).id) >
           std::tie(info.at(v).adjColors, info.at(v).nreprs, info.at(v).nincomp,
                    info.at(v).id);
  };

  // Priority queue with the candidate vertices
  std::set<Vertex, decltype(cmp)> pqueue(cmp);

  // Fill the information of the vertices and add them to the queue
  for (Vertex u : boost::make_iterator_range(vertices(genv.graph))) {
    info.emplace(u, Heur1SVertexInfo(genv, u));
    pqueue.insert(u);
  }

  // Auxiliary vector that helps to decide the color of a vertex. In fact,
  // used[k] tells whether the color k is used in the neighborhood of the
  // current vertex
  std::vector<bool> used(std::min(genv.nA, genv.nB), false);

  // Local variables
  size_t ret;

  while (!pqueue.empty()) {
    // Choose the next vertex
    Vertex u = *pqueue.begin();
    pqueue.erase(pqueue.begin());
    // info.at(u).print_info();

    // Get components (a,b) of u
    TypeA a = genv.graph[u].first;
    TypeB b = genv.graph[u].second;

    // Decide the color of u
    Color i;
    if (col.is_colored_B(b)) {
      // All colored vertices in V^b must have the same color
      i = col.get_color_B(b);
    } else {
      // Get a non-adjacent color
      i = get_free_color(genv.graph, u, info, used);
    }

    // Color u with i
    // std::cout << "Pintando: (" << genv.graph[u].first << ","
    //           << genv.graph[u].second << ") con " << i << std::endl;
    info.at(u).color = i;
    col.set_color(genv.graph, u, i);

    // Set of vertices that must be removed after coloring u
    std::set<Vertex> toRemove;

    for (Vertex v :
         boost::make_iterator_range(adjacent_vertices(u, genv.graph))) {

      // Ignore removed or colored neighbors
      if (info.at(v).removed || info.at(v).color != -1)
        continue;

      // Get components (a,b) of v
      TypeA av = genv.graph[v].first;
      TypeB bv = genv.graph[v].second;

      // If v in V_a or v in V^b, remove v
      if (a == av || b == bv)
        toRemove.insert(v);

      // Otherwise, add the new adjacent color
      // This forces a reallocation in the priority queue
      else {
        if (col.is_colored_B(bv) && i == col.get_color_B(bv))
          toRemove.insert(v);
        else {
          // std::cout << "Realocando: (" << genv.graph[v].first << ","
          //           << genv.graph[v].second << ")" << std::endl;
          pqueue.erase(v);
          info.at(v).adjColors.insert(i);
          pqueue.insert(v);
        }
      }
    }

    // Also, the vertices in V^b that has i as an adjacent color should be
    // removed
    for (auto v : genv.Vb.at(b))
      if (!info.at(v).removed && info.at(v).color == -1 &&
          info.at(v).adjColors.contains(i))
        toRemove.insert(v);

    // Remove vertices and update their neighbors
    for (auto v : toRemove) {

      info.at(v).removed = true;
      ret = pqueue.erase(v);
      assert(ret > 0);
      // std::cout << "Borrando: (" << genv.graph[v].first << ","
      //           << genv.graph[v].second << ")" << std::endl;

      // Get components (a,b) of v
      TypeA av = genv.graph[v].first;
      TypeB bv = genv.graph[v].second;

      // If v not in V_a, update every vertex in V_av,
      // as the size of V_av has changed
      // This forces a reallocation in the priority queue
      if (av != a) {
        size_t count = 0;
        for (auto w : genv.Va.at(av)) {
          if (info.at(w).removed)
            continue;
          // std::cout << "Realocando: (" << genv.graph[w].first << ","
          //           << genv.graph[w].second << ")" << std::endl;
          count++;
          pqueue.erase(w);
          info.at(w).nreprs++;
          pqueue.insert(w);
        }
        if (count == 0) {
          TimePoint end = ClockType::now();
          Stats stats;
          stats.state = INFEASIBLE;
          stats.time = std::chrono::duration<double>(end - start).count();
          return stats;
        }
      }

      // Update every neighbor w of v such that w in V^bv,
      // as v has been removed
      // This forces a reallocation in the priority queue
      for (auto w :
           boost::make_iterator_range(adjacent_vertices(v, genv.graph)))
        if (genv.graph[w].second == bv && !info.at(w).removed &&
            info.at(w).color == -1) {
          // std::cout << "Realocando: (" << genv.graph[w].first << ","
          //           << genv.graph[w].second << ")" << std::endl;
          pqueue.erase(w);
          info.at(w).nincomp++;
          pqueue.insert(w);
        }
    }
  }

  assert(col.check_coloring(genv.graph));

  TimePoint end = ClockType::now();

  Stats stats;
  stats.state = FEASIBLE;
  stats.time = std::chrono::duration<double>(end - start).count();
  stats.ub = static_cast<double>(col.get_n_colors());

  return stats;
}

bool is_conflict(const GraphEnv &genv, Vertex v, Vertex u) {
  return edge(v, u, genv.graph).second;
}

void heur_solve_aux(const GraphEnv &genv, const std::vector<TypeA> &as,
                    Col &col) {

  VertexVector vertices;      // Vector of chosen vertices to be colored
  std::map<TypeB, size_t> bs; // Map from b to its new index (in the subgraph)
  std::vector<TypeB> invbs;   // Map from new index (in the subgraph) to b
  std::map<TypeB, VertexVector>
      repr; // Map from b to the vector of vertices that b represents
  std::vector<std::vector<TypeB>> adj;  // Adjacent lists of the subgraph
                                        // : new index -> std::vector<TypeB>
  int ecount = 0;                       // Number of edges in the subgraph
  int elist[2 * num_edges(genv.graph)]; // Edgle list of the subgraph

  col.reset_coloring();

  for (TypeA a : as) {
    // For each a in A, choose v in snd[i_a] such that:
    // v minimizes the conflicts with the already chosen representatives
    size_t minNNewConflicts = std::numeric_limits<size_t>::max();
    // size_t minNAllConflicts = std::numeric_limits<size_t>::max();
    std::set<TypeB> minNewConflicts;
    // std::set<TypeB> minAllConflicts;
    Vertex bestVertex;
    for (Vertex v : genv.snd[genv.tyA2idA.at(a)]) {
      std::set<TypeB> newConflicts;
      std::set<TypeB> allConflicts;
      TypeB bv = genv.graph[v].second;
      for (Vertex u : vertices) {
        TypeB bu = genv.graph[u].second;
        if (is_conflict(genv, v, u)) {
          if (!bs.contains(bv) ||
              find(adj[bs.at(bv)].begin(), adj[bs.at(bv)].end(), bu) ==
                  adj[bs[bv]].end())
            newConflicts.insert(bu);
          // allConflicts.insert(bu);
        }
      }
      if ((newConflicts.size() < minNNewConflicts) ||
          (newConflicts.size() == minNNewConflicts && rand_double(rng) < 0.5)) {
        minNNewConflicts = newConflicts.size();
        // minNAllConflicts = allConflicts.size();
        // minAllConflicts = allConflicts;
        minNewConflicts = newConflicts;
        bestVertex = v;
      }
    }

    vertices.push_back(bestVertex);
    TypeB b1 = genv.graph[bestVertex].second;
    if (!bs.contains(b1)) {
      bs[b1] = bs.size();
      invbs.push_back(b1);
      adj.push_back(std::vector<TypeB>());
      repr[b1] = std::vector<Vertex>();
    }
    repr[b1].push_back(bestVertex);
    for (auto b2 : minNewConflicts) {
      elist[2 * ecount] = bs[b1];
      elist[2 * ecount++ + 1] = bs[b2];
      adj[bs[b1]].push_back(b2);
      adj[bs[b2]].push_back(b1);
    }
  }

  int ncolors = 0;
  COLORset *colorclasses = NULL;
  COLORdsatur(bs.size(), ecount, elist, &ncolors, &colorclasses);

  // Recover coloring
  for (int k = 0; k < ncolors; ++k)
    for (int j = 0; j < colorclasses[k].count; ++j) {
      TypeB b = invbs[colorclasses[k].members[j]];
      for (Vertex v : repr[b])
        col.set_color(genv.graph, v, k);
    }
  // assert(col.check_coloring());
  return;
}

Stats heur_solve(const GraphEnv &genv, const std::vector<TypeA> &as, Col &col,
                 size_t repetitions, Pool &pool) {
  TimePoint start = ClockType::now();
  Col currentCol;
  std::vector<TypeA> ascopy(as);
  size_t minNCol = std::numeric_limits<size_t>::max();
  for (size_t iter = 0; iter < repetitions; iter++) {
    std::shuffle(ascopy.begin(), ascopy.end(), rng);
    heur_solve_aux(genv, ascopy, currentCol);
    if (currentCol.get_n_colors() < minNCol) {
      minNCol = currentCol.get_n_colors();
      col = currentCol;
    }
    // Add the stable to the pool
    for (size_t k = 0; k < currentCol.get_n_colors(); ++k)
      pool.push_back(currentCol.get_stable(genv.graph, k));
  }
  assert(col.check_coloring(genv.graph));
  TimePoint end = ClockType::now();

  Stats stats;
  stats.state = FEASIBLE;
  stats.time = std::chrono::duration<double>(end - start).count();
  stats.ub = static_cast<double>(col.get_n_colors());

  return stats;
}
