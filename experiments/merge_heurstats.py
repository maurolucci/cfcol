import os
import sys
import pandas as pd


def get_solver(name, subdir):
    parts = subdir.split(os.path.sep)[2:-1]
    return name + "_" + "_".join(parts)


def main() -> int:
    statsPath = sys.argv[1]
    statsName = sys.argv[2]

    columns = [
        "instance",
        "solver",
        "run",
        "nvertices",
        "nedges",
        "n",
        "m",
        "state",
        "value",
        "totalTime",
        "totalIters",
        "bestTime",
        "bestIter",
    ]

    data = []
    for subdir, dirs, files in os.walk(statsPath):
        for file in files:
            if os.path.splitext(file)[-1] != ".stat":
                continue
            with open(os.path.join(subdir, file)) as f:

                # first line
                l0 = f.readline().strip().split(",")
                entry = [
                    l0[0],  # instance
                    get_solver(l0[1], subdir),  # solver
                    int(l0[2]),  # run
                    int(l0[3]),  # nvertices
                    int(l0[4]),  # nedges
                    int(l0[5]),  # n
                    int(l0[6]),  # m
                    l0[7],  # state
                    l0[8],  # value
                    l0[9],  # totalTime
                    l0[10],  # totalIters
                    l0[11],  # bestTime
                    l0[12],  # bestIter
                ]

                data.append(entry)

    df = pd.DataFrame(data, columns=columns)
    df.to_csv(statsName, index=False)

    return 0


if __name__ == "__main__":
    sys.exit(main())
