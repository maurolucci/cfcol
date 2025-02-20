#include "graph.hpp"

void get_conflict_graph(const HGraph &hg, Graph &graph) {
  // Add vertices (pointed sets)
  for (const auto &e : hg.hyperedges())
    for (const auto &v : *hg.impliedVertices(e->id()))
      add_vertex(std::make_pair(e->id(), v->id()), graph);
  // Add edge ((e1,v1), (e2,v2)) such that v2 in e1 - v1
  auto [it1, end] = vertices(graph);
  for (; it1 != end; ++it1)
    for (auto it2 = std::next(it1); it2 != end; ++it2) {
      // Unpack PSets
      auto [e1, v1] = graph[*it1];
      auto [e2, v2] = graph[*it2];
      if (v1 != v2 &&
          (hg.isVertexOfHyperedge(v2, e1) || hg.isVertexOfHyperedge(v1, e2)))
        add_edge(*it1, *it2, graph);
    }
  return;
}

void get_gcp_graph(Graph &src, std::map<TypeB, std::unordered_set<TypeA>> &fst,
                   GCPGraph &dst) {
  // Add vertices
  std::map<TypeB, GCPGraph::vertex_descriptor> id;
  for (auto &p : fst)
    id[p.first] = add_vertex(p.first, dst);

  // Add edges
  for (auto e : boost::make_iterator_range(edges(src))) {
    auto p1 = src[source(e, src)];
    auto p2 = src[target(e, src)];
    if (!edge(id[p1.second], id[p2.second], dst).second)
      add_edge(id[p1.second], id[p2.second], dst);
  }
}