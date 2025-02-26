#ifndef _BP_HPP_
#define _BP_HPP_

#include "bp.hpp"
#include "lp.hpp"
#include <cfloat>
#include <chrono>
#include <cmath>
#include <iterator>

#define EPSILON_BP 0.001 // For doing ceil(x - EPSILON_BP) during prunning

// #define ONLY_RELAXATION
#ifndef MAXTIME
#define MAXTIME 3600
#endif

using ClockType = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::_V2::system_clock::time_point;

class Node {

public:
  Node(LP *lp) : lp(lp) {}

  ~Node() { delete lp; }

  double get_obj_value() const { return lp->get_obj_value(); }

  bool operator<(const Node &n) const {
    return (get_obj_value() < n.get_obj_value());
  }

  LP_STATE solve() { return lp->optimize(); }

  void branch(std::vector<Node *> &sons) {

    std::vector<LP *> lps;
    lp->branch(lps);

    sons.reserve(lps.size());
    for (auto x : lps)
      sons.push_back(new Node(x));

    return;
  }

  template <class Solution> void save(Solution &sol) { lp->save_solution(sol); }

  int get_n_columns() { return lp->get_n_columns(); }

private:
  LP *lp;
};

template <class Solution> class BP {

public:
  BP(Solution &sol, bool DFS = false, bool EARLY_BRANCHING = false)
      : best_integer_solution(sol), DFS(DFS), EARLY_BRANCHING(EARLY_BRANCHING) {

    primal_bound = DBL_MAX;
    nodes = 0;
    root_lower_bound = -1;
  }

  void solve(Node *root) {

    start_t = ClockType::now();
    first_t = start_t;
    first_call = true;

    try {
      push(root);
    } catch (...) { // Time or mem expired
      opt_flag = 0;
      primal_bound = 99999999;
      dual_bound = -99999999;
      nodes = -1;
      time = std::chrono::duration_cast<std::chrono::seconds>(ClockType::now() -
                                                              start_t)
                 .count();
      if (time >= MAXTIME) {
        std::cout << "Time limit reached" << std::endl;
        time = MAXTIME;
      } else {
        std::cout << "Mem limit reached" << std::endl;
      }
      return;
    }

#ifdef ONLY_RELAXATION
    if (!L.empty()) {
      // The initial relaxation is fractional
      opt_flag = 3;
      primal_bound = 99999999;
      double db = calculate_dual_bound();
      dual_bound = db == -DBL_MAX ? -99999999 : db;
      time = std::chrono::duration_cast<std::chrono::seconds>(ClockType::now() -
                                                              start_t)
                 .count();
      std::cout << "The initial relaxation is fractional" << std::endl;
      std::cout << "Objective value = " << dual_bound << std::endl;
      return;
    } else {
      if (primal_bound == DBL_MAX) {
        // The initial relaxation is infeasible
        opt_flag = 2;
        primal_bound = 99999999;
        dual_bound = -99999999;
        time = std::chrono::duration_cast<std::chrono::seconds>(
                   ClockType::now() - start_t)
                   .count();
        std::cout << "The initial relaxation is infeasible" << std::endl;
        return;
      } else {
        // The initial relaxation is integer
        opt_flag = 1;
        dual_bound = primal_bound;
        time = std::chrono::duration_cast<std::chrono::seconds>(
                   ClockType::now() - start_t)
                   .count();
        std::cout << "The initial relaxation is integer" << std::endl;
        std::cout << "Objective value = " << dual_bound << std::endl;
        return;
      }
    }
#endif

    if (!L.empty())
      root_lower_bound = ceil(root->get_obj_value() - EPSILON_BP);

    while (!L.empty()) {

      // Pop
      Node *node = top();
      show_stats(*node); // First show_stats, then pop
      pop();

      // Re-try to prune by bound, since primal_bound could have been improved
      if (ceil(node->get_obj_value() - EPSILON_BP) >= primal_bound) {
        delete node;
        continue;
      }

      // Add sons
      std::vector<Node *> sons;
      node->branch(sons);
      for (auto n : sons) {
        try {
          push(n);
        } catch (...) {
          // Time or mem expired
          opt_flag = 0;
          primal_bound = primal_bound == DBL_MAX ? 99999999 : primal_bound;
          dual_bound = calculate_dual_bound();
          dual_bound = dual_bound == -DBL_MAX ? -99999999 : dual_bound;
          nodes = -1;
          time = std::chrono::duration_cast<std::chrono::seconds>(
                     ClockType::now() - start_t)
                     .count();
          if (time >= MAXTIME) {
            std::cout << "Time limit reached" << std::endl;
            time = MAXTIME;
          } else {
            std::cout << "Mem limit reached" << std::endl;
          }
          L.clear();
          delete node;
          return;
        }
      }

      delete node;
    }

    if (primal_bound == DBL_MAX) { // Infeasibility case:
      opt_flag = 2;
      primal_bound = 99999999;
      double db = calculate_dual_bound();
      dual_bound = db == -DBL_MAX ? -99999999 : db;
      time = std::chrono::duration_cast<std::chrono::seconds>(ClockType::now() -
                                                              start_t)
                 .count();
      std::cout << "Infeasibility proved" << std::endl;
    } else { // Optimality case:
      opt_flag = 1;
      dual_bound = primal_bound;
      time = std::chrono::duration_cast<std::chrono::seconds>(ClockType::now() -
                                                              start_t)
                 .count();
      std::cout << "Optimality reached" << std::endl;
      std::cout << "Optimal value = " << primal_bound << std::endl;
    }

    return;
  }

  // Methods for getting variables' value
  int get_nodes() { return nodes; }

  double get_gap() {
    double _dual_bound = calculate_dual_bound();
    double gap = abs(_dual_bound - primal_bound) /
                 (0.0000000001 + abs(primal_bound)) * 100;
    return gap;
  }

  double get_primal_bound() { return primal_bound; }

  double get_dual_bound() { return dual_bound; };

  double get_time() { return time; }

  int get_opt_flag() { return opt_flag; }

private:
  std::list<Node *> L; // Priority queue

  Solution &best_integer_solution; // Current best integer solution
  double primal_bound; // Primal bound (given by the best integer solution)
  double dual_bound;   // Dual bound (given by the worst open relaxation)
  int nodes; // Number of processed nodes so far. A node is considered processed
  // if its relaxation has been solved
  TimePoint start_t;       // B&P initial execution time
  TimePoint first_t;       // used by log
  bool first_call;         // used by log
  int opt_flag;            // Optimality flag
  double time;             // Total execution time
  double root_lower_bound; // Round-up of the LP objective value at the root of
  // the B&P. Used for early branching

  void push(Node *node) {

    // Solve the linear relaxation of the node and prune if possible
    LP_STATE state = node->solve();

#ifdef ONLY_RELAXATION
    std::cout << "Number of columns = " << node->get_n_columns() << std::endl;
#endif

    nodes++;
    double obj_value;

    switch (state) {

    case INFEASIBLE:
      // Prune by infeasibility
      delete node;
      return;

    case INTEGER:
      // Prune by optimality
      obj_value = node->get_obj_value();
      if (obj_value < primal_bound)
        update_primal_bound(*node);
      delete node;
      return;

    case FRACTIONAL:
      obj_value = node->get_obj_value();
      if (ceil(obj_value - EPSILON_BP) >= primal_bound) {
        // Prune by bound
        delete node;
        return;
      }
      break;

    case TIME_OR_MEM_LIMIT:
      delete node;
      throw std::exception{};
      return;
    }

    // Place the node in the list according to its priority

    if (DFS) { // DFS strategy
      L.push_back(node);
      return;
    }

    // Otherwise, best-bound strategy

    // list is empty
    if (L.empty()) {
      L.push_back(node);
      return;
    }

    // list is not empty
    for (auto it = L.begin(); it != L.end(); ++it)
      if (*node < **it) {
        L.insert(it, node);
        return;
      }

    // Otherwise, push back
    L.push_back(node);
    return;
  }

  Node *top() { return L.back(); }

  void pop() {
    L.pop_back();
    return;
  }

  void update_primal_bound(Node &node) {

    // Update best integer solution and value
    node.save(best_integer_solution);
    primal_bound = node.get_obj_value();

    // Prune if possible
    for (auto it = L.begin(); it != L.end();) {
      if ((*it)->get_obj_value() >= primal_bound) {
        delete *it;
        it = L.erase(it);
      } else
        ++it;
    }
  }

  double calculate_dual_bound() {

    double _dual_bound = DBL_MAX; // minimum objective value of unpruned nodes
    if (DFS) {
      // Traverse the list and search for the minimum objective value
      for (auto it = L.begin(); it != L.end(); ++it)
        if ((*it)->get_obj_value() < _dual_bound)
          _dual_bound = (*it)->get_obj_value();
    } else {
      _dual_bound = L.front()->get_obj_value();
    }

    if (_dual_bound == DBL_MAX) {
      return -DBL_MAX;
    } else {
      return _dual_bound;
    }
  }

  void show_stats(Node &node) {

    auto now_t = ClockType::now();

    if (first_call)
      first_call = false;
    else {
      if (std::chrono::duration_cast<std::chrono::seconds>(now_t - start_t)
              .count() < 10.0)
        return;
      first_t = now_t;
    }

    double _dual_bound =
        calculate_dual_bound(); // (note: it is time consuming when DFS is used)

    std::cout << std::fixed << std::setprecision(3);

    // std::cout << "  Obj value = " << node.get_obj_value();
    std::cout << "  Best obj value  = " << _dual_bound << "\t Best int = ";
    if (primal_bound == DBL_MAX)
      std::cout << "inf\t Gap = ---";
    else
      std::cout << (int)(EPSILON + primal_bound) << "\t Gap = "
                << (primal_bound - _dual_bound) / (EPSILON + primal_bound) * 100
                << "%";
    std::cout << "\t Nodes: processed = " << nodes << ", left = " << L.size()
              << "\t time = "
              << std::chrono::duration_cast<std::chrono::seconds>(now_t -
                                                                  start_t)
                     .count()
              << std::endl;
  }

  // Flags
  bool DFS;
  // **** Early Branching ****
  //  At any node, if the column generation arrives to a solution with lower
  //  value than root_lower_bound, then is not necessary to keep optimizing the
  //  node and it could be inmediatly branched. CAUTION: Early branching may
  //  alter the way nodes are branched (beacause braching depends on the LP
  //  solution)
  bool EARLY_BRANCHING;
};

#endif // _BP_HPP_
