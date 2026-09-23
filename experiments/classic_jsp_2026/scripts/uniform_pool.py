#!/usr/bin/env python3
"""The control for the circle: uniformly random job sequences (every multiset
permutation equally likely), written in the same pool format, one block of 247
per run, so they enter through exactly the same seeded creation path.

If circle seeds and uniform seeds start equally well, the generation-0 gain is
the creation path and its distribution, not the circle.

Usage: uniform_pool.py <runs> <instance> [<instance> ...]
"""
import os
import random
import sys

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
OUT = os.path.join(E, "seeds", "uniform")
N = 247
SIZE = {}
for row in open(os.path.join(E, "taillard_bounds.csv")):
    f = row.strip().split(",")
    if f[0] != "instance":
        SIZE[f[0]] = (int(f[1]), int(f[2]))

runs = int(sys.argv[1])
os.makedirs(OUT, exist_ok=True)
for instance in sys.argv[2:]:
    n, m = SIZE[instance]
    base = [j for j in range(1, n + 1) for _ in range(m)]
    rng = random.Random("uniform-%s" % instance)
    with open(os.path.join(OUT, instance + ".csv"), "w", newline="\n") as out:
        for g in range(runs):
            for k in range(N):
                s = base[:]
                rng.shuffle(s)
                out.write(" ".join(map(str, s)) + ";uniform g=%d k=%d\n" % (g, k))
    print("%-6s %d lines" % (instance, runs * N))
