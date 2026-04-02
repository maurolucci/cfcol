#ifndef _COL_HPP_
#define _COL_HPP_

#include <list>
#include <map>
#include <set>
#include <vector>

#include "graph.hpp"

using Color = int;
using VertexId = size_t;
using Coloring = std::map<VertexId, Color>;
using ColorClass = std::map<Color, std::set<VertexId>>;

class Col {
 public:
  Col();

  [[nodiscard]] inline const Coloring& get_coloring() const {
    return coloring;
  };

  [[nodiscard]] inline const ColorClass& get_color_classes() const {
    return classes;
  };

  [[nodiscard]] inline size_t get_n_colors() const { return classes.size(); };

  [[nodiscard]] inline bool is_colored(VertexId v) const {
    return coloring.contains(v);
  };

  [[nodiscard]] inline Color get_color(VertexId v) const {
    return coloring.at(v);
  };

  [[nodiscard]] inline bool is_colored_Q(size_t qj) const {
    return colorQ.contains(qj);
  };

  [[nodiscard]] inline Color get_color_Q(size_t qj) const {
    return colorQ.at(qj);
  };

  void reset_coloring();

  // Set the color of vertex v (explicit pi/qj)
  void set_color(VertexId v, size_t pi, size_t qj, Color k);
  // Convenience: look up pi/qj from dpcp
  void set_color(const DPCPInst& dpcp, VertexId v, Color k);

  [[nodiscard]] bool check_coloring(const DPCPInst& dpcp) const;

  [[nodiscard]] StableEnv get_stable(const DPCPInst& dpcp, Color k) const;

  Col translate_coloring(const DPCPInst& currentDpcp,
                         const DPCPInst& originalDPCP) const;

  void color_isolated_vertices(
      const DPCPInst& dpcp,
      const std::list<IsolatedVertex>& isolatedVertices);

  void write_coloring(std::ostream& out) const;

 private:
  Coloring coloring;
  ColorClass classes;
  std::map<size_t, Color> colorP;
  std::map<size_t, Color> colorQ;
};

#endif  // _COL_HPP_