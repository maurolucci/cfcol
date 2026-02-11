#include "graph.hpp"
#include "heur.hpp"

#include <fstream>
#include <iostream>
#include <numeric>
#include <string>

int main() {

  std::vector<int> seeds(10);
  std::iota(seeds.begin(), seeds.end(), 0);

  for (auto i : seeds) {

    std::string file =
        "instances/cfc/geom/circle_n10_r40_i" + std::to_string(i) + ".dpcp";
    std::cout << "Processing file: " << file << std::endl;

    // Open input files
    std::ifstream inGraph(file + ".graph");
    std::ifstream inPartA(file + ".partA");
    std::ifstream inPartB(file + ".partB");
    if (!inGraph.is_open() || !inPartA.is_open() || !inPartB.is_open()) {
      std::cerr << "Error opening files" << std::endl;
      return 2;
    }

    // Read DPCP instance
    Graph graph;
    size_t nA, nB;
    std::tie(graph, nA, nB) = read_dpcp_instance(inGraph, inPartA, inPartB);

    GraphEnv genv(&graph, true, false, false, false);
    Col col;
    Stats stats;

    stats = dpcp_2_step_greedy_heur(genv, col, 0);
    std::cout << "2-STEP DEG-REAL: " << stats.get_state_as_str() << " "
              << stats.time << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();

    stats = dpcp_2_step_greedy_heur(genv, col, 2);
    std::cout << "2-STEP DEG-COLLAPSED: " << stats.get_state_as_str() << " "
              << stats.time << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();

    stats = dpcp_2_step_greedy_heur(genv, col, 3);
    std::cout << "2-STEP EDGE: " << stats.get_state_as_str() << " "
              << stats.time << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();

    stats = dpcp_1_step_greedy_heur(genv, col);
    std::cout << "1-STEP: " << stats.get_state_as_str() << " " << stats.time
              << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();

    stats = dpcp_2_step_semigreedy_heur(genv, col, 100, 0);
    std::cout << "1-STEP semigreedy DEG-REAL: " << stats.get_state_as_str()
              << " " << stats.time << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();

    stats = dpcp_2_step_semigreedy_heur(genv, col, 100, 2);
    std::cout << "1-STEP semigreedy DEG-COLLAPSED: " << stats.get_state_as_str()
              << " " << stats.time << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();

    stats = dpcp_2_step_semigreedy_heur(genv, col, 100, 3);
    std::cout << "1-STEP semigreedy EDGE: " << stats.get_state_as_str() << " "
              << stats.time << " " << stats.ub << std::endl;
    assert(col.check_coloring(graph));
    col.reset_coloring();
  }

  return 0;
}