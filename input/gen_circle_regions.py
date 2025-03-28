"""Random generation of hypergraphs associated with circle regions."""

import math
import random
import networkx as nx
import matplotlib.pyplot as plt
from matplotlib.patches import Circle

# Type alias
Number = float | int
Point = tuple[Number, Number]
Circ = tuple[Point, Number]  # x-coordinate, y-coordinate, radious
Edge = list[int]
HyperGraph = tuple[int, int, tuple[int, list[int]]]
Vertex = int

def gen_circles(
    sside: int,
    cnum: int,
    cmin: int,
    cmax: int,
    real: bool = False,
    seed: int | None = None,
) -> list[Circ]:
    """
    Generate a list of random circles.

    Args:
        sside: Length of the side of the scene.
        cnum: Number of circles.
        cmin: Minimum length of the radious (cmin > 0).
        cmax: Maximum length of the readious (cmax >= cmin and cmax < sside).
        real: Whether to generate random real numbers, otherwise integers.
        seed: Seed for random generation.

    Returns:
        A list of circles. Each one of the form (x,y,r) where (x,y) are
        the coordinates of the center and r is the radious.
    """
    ncirc = 0
    circ = []
    generator = random.uniform if real else random.randint
    random.seed(seed)
    while ncirc < cnum:
        x = generator(0, sside)
        y = generator(0, sside)
        r = generator(cmin, cmax)
        if x - r < 0 or x + r > sside or y - r < 0 or y + r > sside:
            continue
        circ.append(((x, y), r))
        ncirc += 1
    return circ


def plot(sside: int, circ: list[Circ]) -> None:
    """Plot a list of circles inside a scene.

    Args:
        sside: Length of the side of the scene.
        circ: List of rectangles.
    """
    plt.figure()
    plt.xlim(0, sside)
    plt.ylim(0, sside)
    ax = plt.gca()
    for c, r in circ:
        ax.add_patch(Circle(c, r, fill=False))
    plt.show()


def gen_mesh_points(sside: int, nsteps: int) -> list[Point]:
    """Generate the points of a mesh.

    Args:
        sside: Length of the side of the scene.
        nsteps: Size of the mesh, that is,
            with (nsteps + 1)*(nsteps + 1) points.
    """
    step = sside/nsteps
    return [
        (x * step, y * step) for x in range(nsteps + 1) for y in range(nsteps + 1)
    ]


def build_hypergraph(circ: list[Circ], points: list[Point]) -> HyperGraph:
    """Build the hypergraph associated with the circle regions.

    Args:
        circ: A list of circles.
        points: A list of points in R^2.

    Returns:
        A hypergraph. That is, a tuple with: number of vertices, number of
        hyperedges, and a list of hyperedges. Each hyperedge is a tuple with
        the number of implied vertices and a list of them.
    """
    n = len(circ)
    m = 0
    edges = []
    for p in points:
        nedge = 0
        edge = []
        for i, (c, r) in enumerate(circ):
            if (euclidean_distance(p, c) > r):
                continue
            nedge += 1
            edge.append(i)
        if nedge > 0 and (nedge, edge) not in edges:
            m += 1
            edges.append((nedge, edge))
    return n, m, edges


def write_hypergraph(path: str, graph: HyperGraph) -> None:
    """Write a hypergraph in a file.

    Args:
        path: Path of file.
        graph: Hypergraph.
    """
    with open(path, "w", encoding="utf8") as f:
        f.write(str(graph[0]) + " " + str(graph[1]) + "\n")
        for e in graph[2]:
            f.write(str(e[0]))
            for v in e[1]:
                f.write(" " + str(v))
            f.write("\n")


def euclidean_distance(point1: Point, point2: Point) -> float:
    """Euclidean distance between two points in R2.
    
    Args:
        point1: Point in R2.
        point2: Point in R2.

    Returns:
        Euclidean distance.
    """
    return math.sqrt((point1[0] - point2[0])**2 + (point1[1] - point2[1])**2)


# Example
SSIDE = 20
circles = gen_circles(SSIDE, 50, 1, 2, real=True)
points = gen_mesh_points(SSIDE, 1000)  # 1M points
num_vertices, num_edges, edges = build_hypergraph(circles, points)
write_hypergraph("out1.txt", (num_vertices, num_edges, edges))
plot(SSIDE, circles)


# def circle_includes(circ1: Circ, circ2: Circ) -> bool:
#     return euclidean_distance(circ1[0], circ2[0]) + circ2[1] <= circ1[1]


# def circle_intersect(circ1: Circ, circ2: Circ) -> bool:
#     return euclidean_distance(circ1[0], circ2[0]) <= circ1[1] + circ2[1]


# def build_inclusion_digraph(circ: list[Circ]) -> nx.DiGraph:
#     digraph = nx.DiGraph()
#     for i in range(len(circ)):
#         digraph.add_node(i)
#     for i1, c1 in enumerate(circ):
#         for i2, c2 in enumerate(circ):
#             if i1 != i2 and circle_includes(c1, c2):
#                 digraph.add_edge(i1, i2)
#     return digraph

# def build_intersection_graph(circ: list[Circ]) -> nx.Graph:
#     graph = nx.Graph()
#     for i in range(len(circ)):
#         graph.add_node(i)
#     for i1, c1 in enumerate(circ):
#         for i2, c2 in enumerate(circ):
#             if i2 > i1 and circle_intersect(c1, c2):
#                 graph.add_edge(i1, i2)
#     return graph


# def get_ancestors(v: Vertex, digraph: nx.DiGraph) -> list[int]:
#     return sorted(nx.ancestors(digraph, v) | {v})


# def build_hypergraph2(circ: list[Circ]) -> HyperGraph:
#     """Build the hypergraph associated with the circle regions.

#     Args:
#         circ: A list of circles.

#     Returns:
#         A hypergraph. That is, a tuple with: number of vertices, number of
#         hyperedges, and a list of hyperedges. Each hyperedge is a tuple with
#         the number of implied vertices and a list of them.
#     """
#     n = len(circ)
#     m = 0
#     edges = []
#     digraph = build_inclusion_digraph(circ)
#     graph = build_intersection_graph(circ)
#     ancestors = {v: get_ancestors(v, digraph) for v in graph}
#     # Remove edges
#     for u, anc in ancestors.items():
#         for v in anc:
#             if graph.has_edge(u, v):
#                 graph.remove_edge(u, v)
#     # Generate cliques
#     cliques = list(nx.enumerate_all_cliques(graph))
#     # Translate cliques
#     for clique in cliques:
#         edge = set()
#         for v in clique:
#             edge.update(ancestors[v])
#         m += 1
#         edges.append((len(edge), list(edge)))
#     return n, m, edges