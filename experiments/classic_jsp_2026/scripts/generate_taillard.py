#!/usr/bin/env python3
"""Generate random job shop instances from Taillard's distribution.

Taillard (1993) builds an n x m instance by drawing every processing time
uniformly from 1..99 and giving every job a uniformly random permutation of the
machines. These instances follow that recipe with Python's generator rather
than Taillard's own linear congruential one, so they are new draws from the
same distribution, not reproductions of his; that is the point -- they are for
tuning, and they must not be the instances the tuned algorithm is then judged
on. The 30x20 class in particular has no closed Taillard instance to train on:
all ten are open.

Files are written in the format convert_orlib.py produces for the Taillard
instances, so the solver reads them the same way.

Usage:
    generate_taillard.py --out DIR --seed S CLASS[:COUNT] ...
    e.g. generate_taillard.py --out tuning/instances --seed 20260919 20x15:5 20x20:5 30x15:5 30x20:5
"""
import argparse
import os
import random


def write_instance(path, n_jobs, n_machines, routes, durations):
    with open(path, "w") as out:
        out.write("NUMERO DE TRABAJOS\n%d\n" % n_jobs)
        out.write("NUMERO DE RECURSOS\n%d\n" % n_machines)
        out.write("SECUENCIA DE MAQUINAS\n")
        for route in routes:
            out.write(" ".join(str(m) for m in route) + " \n")
        out.write("DURACIONES\n")
        for row in durations:
            out.write(" ".join("(%d, %d)" % (p, p) for p in row) + " \n")


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--out", required=True)
    parser.add_argument("--seed", type=int, required=True)
    parser.add_argument("classes", nargs="+", help="JOBSxMACHINES[:COUNT]")
    args = parser.parse_args()

    os.makedirs(args.out, exist_ok=True)
    rng = random.Random(args.seed)
    written = []
    for spec in args.classes:
        size, _, count = spec.partition(":")
        n_jobs, n_machines = (int(x) for x in size.lower().split("x"))
        for k in range(1, int(count or 1) + 1):
            routes = []
            durations = []
            for _ in range(n_jobs):
                route = list(range(n_machines))
                rng.shuffle(route)
                routes.append(route)
                durations.append([rng.randint(1, 99) for _ in range(n_machines)])
            name = "gen%dx%d_%02d" % (n_jobs, n_machines, k)
            write_instance(os.path.join(args.out, name + ".txt"),
                           n_jobs, n_machines, routes, durations)
            written.append(name)
    print("wrote %d instances to %s (seed %d): %s"
          % (len(written), args.out, args.seed, " ".join(written)))


if __name__ == "__main__":
    main()
