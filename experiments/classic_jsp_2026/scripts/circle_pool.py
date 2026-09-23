#!/usr/bin/env python3
"""The PI's circle seeding (2026-09-23): represent every area of the search
space in the initial population.

The idea, in the PI's words: picture the whole space of solutions as a circle.
With 247 solutions, cut the circle into 247 equal arcs and let each cut be one
solution. To translate a point of the circle into a concrete solution, every
element of the search space must correspond to exactly one position on it.
And rotate the circle for each initial seed, so the selection is not repeated
across seeds.

How it is realised here.

* The search space is the set of operation sequences: each of the n jobs
  appears m times, so there are |S| = (nm)! / (m!)^n of them, about 10^500 for
  a 20x20 instance. They are put in LEXICOGRAPHIC order and numbered 0..|S|-1;
  unrank() turns a number into its sequence exactly, with integer arithmetic.
  That number is the position on the circle.
* Circle g (the one used by the run whose global index is g) places its 247
  points at o_g + floor(k |S| / 247), k = 0..246: 247 equal arcs.
* The rotation o_g is the golden-ratio sequence frac((g + 1) phi) times one arc,
  so successive seeds rotate by an irrational fraction and never repeat the
  same cut, and together they spread evenly round the arc.

The pool holds one circle per run, 247 lines each, in run order: the seeded
creation takes the block starting at (global run * seed.count) mod pool size,
so with creation.seed.count = 247 and creation.seed.offset = g, run g reads
exactly circle g.

Deterministic and reproducible; the pools themselves live under the
git-ignored seeds/ directory.

Usage: circle_pool.py <runs> <instance> [<instance> ...]
"""
import os
import sys
from decimal import Decimal, getcontext
from math import factorial
from multiprocessing import Pool

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
OUT = os.path.join(E, "seeds", "circle")
N = 247

SIZE = {}
for row in open(os.path.join(E, "taillard_bounds.csv")):
    f = row.strip().split(",")
    if f[0] != "instance":
        SIZE[f[0]] = (int(f[1]), int(f[2]))


def space_size(n, m):
    return factorial(n * m) // factorial(m) ** n


def unrank(r, n, m):
    """The r-th sequence, in lexicographic order, of n jobs each repeated m
    times. The block of sequences beginning with job j at a given point has
    exactly total * c_j / L members, so the job is found with one division."""
    counts = [m] * n
    L = n * m
    total = space_size(n, m)
    seq = []
    for _ in range(n * m):
        q = (r * L) // total              # which job's block holds r
        cum = 0
        for j in range(n):
            if counts[j] == 0:
                continue
            if q < cum + counts[j]:
                break
            cum += counts[j]
        r -= total * cum // L             # skip the blocks before job j
        total = total * counts[j] // L    # size of job j's block
        counts[j] -= 1
        L -= 1
        seq.append(j + 1)
    return seq


def golden(g):
    getcontext().prec = 60
    phi = (Decimal(5).sqrt() - 1) / 2
    x = (g + 1) * phi
    return x - int(x)


def build(args):
    instance, runs = args
    n, m = SIZE[instance]
    S = space_size(n, m)
    arc = S // N
    os.makedirs(OUT, exist_ok=True)
    path = os.path.join(OUT, instance + ".csv")
    with open(path, "w", newline="\n") as out:
        for g in range(runs):
            # the rotation: a golden-ratio fraction of one arc, exact enough
            u = golden(g)
            num, den = u.as_integer_ratio()
            o = arc * num // den
            for k in range(N):
                r = o + (k * S) // N
                seq = unrank(r, n, m)
                out.write(" ".join(map(str, seq)) + ";circle g=%d k=%d\n" % (g, k))
    return instance, runs * N, os.path.getsize(path)


def main():
    if len(sys.argv) < 3:
        raise SystemExit(__doc__)
    runs = int(sys.argv[1])
    jobs = [(i, runs) for i in sys.argv[2:]]
    with Pool(min(14, len(jobs))) as pool:
        for instance, lines, size in pool.imap_unordered(build, jobs):
            print("%-6s %6d lines  %6.1f MB" % (instance, lines, size / 1e6))


if __name__ == "__main__":
    main()
