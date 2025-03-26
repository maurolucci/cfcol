"""Random generation of hypergraphs associated with axis-parallel rectangle regions."""

import random
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle


def gen_rectangles(
    sside: int, rnum: int, rmin: int, rmax: int
) -> list[tuple[int, int, int, int]]:
    """
    Generate a list of random rectangles.

    Args:
        sside: Length of the side of the scene.
        rnum: Number of rectangles.
        rmin: Minimum length of the side of the rectangles (rmin > 0).
        rmax: Maximum length of the side of the rectangles (rmax >= rmin and rmax < sside).

    Returns:
        A list of rectangles. Each one of the form (x1,y1,x2,y2) where (x1,y1) y (x2,y2) are
        the coordinates of the opposite vertices.
    """
    nrect = 0
    rect = []
    while nrect < rnum:
        x1 = random.randint(0, sside)
        y1 = random.randint(0, sside)
        x2 = x1 + random.randint(rmin, rmax)
        y2 = y1 + random.randint(rmin, rmax)
        if x2 > sside or y2 > sside:
            continue
        rect.append((x1, y1, x2, y2))
        nrect += 1
    return rect


def plot(sside: int, rect: list[(int, int, int, int)]) -> None:
    """Plot a list of rectangles inside a scene.

    Args:
        sside: Length of the side of the scene.
        rect: List of rectangles.
    """
    plt.figure()
    plt.xlim(0, sside)
    plt.ylim(0, sside)
    ax = plt.gca()
    for x1, y1, x2, y2 in rect:
        ax.add_patch(Rectangle((x1, y1), x2 - x1, y2 - y1, fill=False))
    plt.show()


def get_inner_points(
    rect: list[(int, int, int, int)],
) -> tuple[list[float], list[float]]:
    xs = set()
    ys = set()
    for x1, y1, x2, y2 in rect:
        xs.add(x1)
        xs.add(x2)
        ys.add(y1)
        ys.add(y2)
    xs = sorted(xs)
    ys = sorted(ys)
    mxs = list(map(lambda x, y: (x + y) / 2, xs[:-1], xs[1:]))
    mys = list(map(lambda x, y: (x + y) / 2, ys[:-1], ys[1:]))
    return mxs, mys


def get_hypergraph(
    rect: list[(int, int, int, int)],
) -> tuple[int, int, list[tuple[int, list[int]]]]:
    n = len(rect)
    m = 0
    hedges = []
    mxs, mys = get_inner_points(rect)
    for x in mxs:
        for y in mys:
            nregs = 0
            regs = []
            for i, (x1, y1, x2, y2) in enumerate(rect):
                if x < x1 or x > x2 or y < y1 or y > y2:
                    continue
                nregs += 1
                regs.append(i)
            if nregs > 0 and (nregs, regs) not in hedges:
                m += 1
                hedges.append((nregs, regs))
    return n, m, hedges


def write_hypergraph(path: str, graph: tuple[int, int, list[tuple[int, list[int]]]]):
    with open(path, "w", encoding="utf8") as f:
        f.write(str(graph[0]) + " " + str(graph[1]) + "\n")
        for e in graph[2]:
            f.write(str(e[0]))
            for v in e[1]:
                f.write(" " + str(v))
            f.write("\n")


# Example
SSIDE = 20
rectangles = gen_rectangles(SSIDE, 50, 2, 5)
num_vertices, num_edges, edges = get_hypergraph(rectangles)
write_hypergraph("out.txt", (num_vertices, num_edges, edges))
plot(SSIDE, rectangles)
