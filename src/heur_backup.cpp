// Select a set of candidate vertices to color, with the following criterion:
// (i) Among the uncolored a, choose one with the lowest |V_a|
// (ii) Build a set of candidates, containing every vertex (a,b) of V_a
// such that: (1) some vertex in V^b is already colored or (2) (a,b) has
// the lowest degree of saturation in V_a
VertexVector vertices_selector(const GraphEnv &genv, Col &col,
                               const InfoMap &info, VaSizeMap &nVa, ASet &A,
                               bool greedy) {

  // Comparison function for elements of A
  std::function<bool(const TypeA, const TypeA)> compareFunc;
  if (greedy) {
    // For greedy strategy, choose a such that Va has currently the minimum
    // size, break ties by the index of a
    compareFunc = [&nVa](const TypeA a1, const TypeA a2) {
      return (nVa.at(a1) < nVa.at(a2) || (nVa.at(a1) == nVa.at(a2) && a1 < a2));
    };
  } else {
    // Otherwise, choose a such that Va has currently the minimum size,
    // but break ties randomly
    compareFunc = [&nVa](const TypeA a1, const TypeA a2) {
      return (nVa.at(a1) < nVa.at(a2) ||
              (nVa.at(a1) == nVa.at(a2) && (rand_int(rng) % 2) == 0));
    };
  }

  // Choose the next a
  TypeA a = *std::min_element(A.begin(), A.end(), compareFunc);
  A.erase(a);

  // Set of candidates
  VertexVector candidates1, candidates2;
  size_t minDSat = std::numeric_limits<size_t>::max();
  for (Vertex v : genv.Va.at(a)) {
    if (info.at(v).removed)
      continue;
    TypeB b = genv.graph[v].second;
    if (col.is_colored_B(b)) {
      candidates1.push_back(v);
      continue;
    }
    size_t dsat = info.at(v).adjColors.size();
    if (dsat < minDSat) {
      minDSat = dsat;
      candidates2.clear();
    }
    if (dsat == minDSat)
      candidates2.push_back(v);
  }
  for (Vertex v : candidates1)
    candidates2.push_back(v);

  return candidates2;
}

// Select a vertex from the candidates and a used free color, with the
// following criterion: the one that invalidates the lowest number of
// vertices. If there is none, do the same with a new color. Return the
// vertex, the color, and the set of invalidated vertices
std::tuple<Vertex, Color, VertexSet>
greedy_vertex_color_selector(const GraphEnv &genv, Col &col,
                             const InfoMap &info, const VaSizeMap &nVa,
                             const VertexVector &vCandidates) {

  VertexSet minInv;
  size_t n_minInv = std::numeric_limits<size_t>::max();
  Vertex v = NULL;
  Color c = -1;
  bool hasNewColor = true;

  // Iterate over the candidate vertices
  for (Vertex u : vCandidates) {

    // Find candidate colors for u
    std::vector<Color> colors;
    if (col.is_colored_B(genv.graph[u].second))
      colors.push_back(col.get_color_B(genv.graph[u].second));
    else {
      for (size_t i = 0; i < col.get_n_colors(); ++i)
        if (!info.at(u).adjColors.contains(i))
          colors.push_back(i);              // Free used color
      colors.push_back(col.get_n_colors()); // New color
    }

    // Among the "safe" vertex-color pairs, i.e. that do not invalidate all
    // the remaining vertices of any V_a', select the one that minimice the
    // number of invalidated vertices Use a new color only if there is no safe
    // pair with a used color
    for (Color k : colors) {
      bool newColor = (k == static_cast<int>(col.get_n_colors()));
      // Ignore new colors if there is already a safe pair with a used color
      if (!hasNewColor && newColor)
        continue;
      VertexSet inv;
      try {
        inv = get_invalidated_vertices(genv, col, info, nVa, u, k);
      } catch (...) {
        continue;
      }
      if ((hasNewColor && !newColor) || inv.size() < n_minInv ||
          (inv.size() == n_minInv && genv.getId.at(u) < genv.getId.at(v)) ||
          (inv.size() == n_minInv && genv.getId.at(u) == genv.getId.at(v) &&
           k < c)) {
        n_minInv = inv.size();
        minInv = inv;
        v = u;
        c = k;
        if (!newColor)
          hasNewColor = false;
      }
    }
  }

  // If any pair is candidate yet, throw exception
  if (v == NULL)
    throw std::exception();

  return std::make_tuple(v, c, minInv);
}

// Incorporate some randomness in the greedy selection
std::tuple<Vertex, Color, VertexSet>
semigreedy_vertex_color_selector(const GraphEnv &genv, Col &col,
                                 const InfoMap &info, const VaSizeMap &nVa,
                                 const VertexVector &vCandidates) {

  size_t n_minInv = std::numeric_limits<size_t>::max();
  size_t n_maxInv = 0;
  std::map<Vertex, std::map<Color, VertexSet>> invMap;
  std::tuple<Vertex, Color, VertexSet> bestNewColor;
  size_t nInvNewColor = std::numeric_limits<size_t>::max();

  // Iterate over the candidate vertices
  for (Vertex u : vCandidates) {

    // Find candidate colors for u
    std::vector<Color> colors;
    if (col.is_colored_B(genv.graph[u].second))
      colors.push_back(col.get_color_B(genv.graph[u].second));
    else {
      for (size_t i = 0; i < col.get_n_colors(); ++i)
        if (!info.at(u).adjColors.contains(i))
          colors.push_back(i);              // Free used color
      colors.push_back(col.get_n_colors()); // New color
    }

    // Find the minimum and maximum number of invalidated vertices
    for (Color k : colors) {
      bool newColor = (k == static_cast<int>(col.get_n_colors()));
      VertexSet inv;
      try {
        inv = get_invalidated_vertices(genv, col, info, nVa, u, k);
      } catch (...) {
        continue;
      }
      invMap[u][k] = inv;
      if (newColor && inv.size() < nInvNewColor) {
        bestNewColor = std::make_tuple(u, k, inv);
        nInvNewColor = inv.size();
      }
      if (!newColor && inv.size() < n_minInv)
        n_minInv = inv.size();
      if (!newColor && inv.size() > n_maxInv)
        n_maxInv = inv.size();
    }
  }

  // If there is no candidate with a used color, return the best new color
  if (n_minInv == std::numeric_limits<size_t>::max())
    return bestNewColor;

  // Build the RCL
  std::vector<std::tuple<Vertex, Color, VertexSet>> rcl;
  double cutoff = n_minInv + ALPHA_B * (n_maxInv - n_minInv);
  for (auto &[u, cmap] : invMap)
    for (auto &[k, inv] : cmap) {
      if (k == static_cast<int>(col.get_n_colors()))
        continue;
      if (inv.size() <= cutoff)
        rcl.emplace_back(u, k, inv);
    }

  if (rcl.empty())
    throw std::exception();

  return rcl[rand_int(rng) % rcl.size()];
}

// One-step heuristic for DPCP
// Return false if it is not possible to color the graph
bool single_step(const GraphEnv &genv, Col &col, bool greedy) {

  // Map with the necessary information of each vertex
  std::map<Vertex, Heur1SVertexInfo> info;

  // Map with the size of each Va
  std::map<TypeA, size_t> nVa;
  for (auto &[a, Va] : genv.Va)
    nVa.emplace(a, Va.size());

  // Fill the information of the vertices and add them to the candidate list
  for (Vertex u : boost::make_iterator_range(vertices(genv.graph)))
    info.emplace(u, Heur1SVertexInfo(genv.graph, u));

  // Subset of A that has not been processed yet
  ASet A(genv.idA2TyA.begin(), genv.idA2TyA.end());

  while (!A.empty()) {

    auto vCandidates = vertices_selector(genv, col, info, nVa, A, greedy);
    std::tuple<Vertex, Color, VertexSet> tuple;
    try {
      if (greedy)
        tuple = greedy_vertex_color_selector(genv, col, info, nVa, vCandidates);
      else
        tuple =
            semigreedy_vertex_color_selector(genv, col, info, nVa, vCandidates);
    } catch (...) {
      return false;
    }
    Vertex u = std::get<0>(tuple);
    Color i = std::get<1>(tuple);
    VertexSet &inv = std::get<2>(tuple);

    // Color u with i
    info.at(u).color = i;
    col.set_color(genv.graph, u, i);
    // info.at(u).print_info();

    // Invalidate the remaining vertices of Va
    for (Vertex v : genv.Va.at(genv.graph[u].first))
      if (v != u && !info.at(v).removed)
        inv.insert(v);

    // Add i as an adjacent color
    for (Vertex v : info.at(u).uncolNeighbors)
      info.at(v).adjColors.insert(i);

    // Remove vertices and update information
    for (Vertex v : inv) {
      info.at(v).removed = true;
      nVa.at(info.at(v).a)--;
      for (Vertex w : info.at(v).uncolNeighbors) {
        info.at(w).uncolNeighbors.erase(v);
        TypeB jv = info.at(v).b;
        info.at(w).degree[jv]--;
        if (info.at(w).degree[jv] == 0)
          info.at(w).degree.erase(jv);
      }
    }
  }
  return true;
}

// One-step semigreedy heuristic for DPCP
// Semigreedy version of the one-step heuristic
Stats dpcp_1_step_semigreedy_heur(const GraphEnv &genv, Col &col,
                                  size_t nIters) {
  TimePoint start = ClockType::now();
  Stats stats;

  while (nIters-- > 0) {
    Col newCol;
    bool success = single_step(genv, newCol, false);
    if (!success)
      continue;
    if (col.get_n_colors() == 0 || newCol.get_n_colors() < col.get_n_colors())
      col = newCol;
  }

  if (col.get_n_colors() == 0) {
    stats.state = INFEASIBLE;
  } else {
    assert(col.check_coloring(genv.graph));
    stats.state = FEASIBLE;
    stats.ub = static_cast<double>(col.get_n_colors());
  }

  TimePoint end = ClockType::now();
  stats.time = std::chrono::duration<double>(end - start).count();
  return stats;
}