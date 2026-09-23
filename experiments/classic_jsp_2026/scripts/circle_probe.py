#!/usr/bin/env python3
"""Does the circle cover the search space better than random sampling?

1. Self-test of unrank on a case small enough to enumerate.
2. Coverage in the space of operation sequences, BEFORE the solver decodes
   them: for thousands of random probe sequences, the distance to the NEAREST
   seed. If the circle represents every area, no probe should be left far from
   every seed, so both the mean and the worst nearest distance should drop
   against 247 random seeds. Distance is the solver's own: the share of
   positions holding a different job.
3. Which positions the circle actually stratifies: distinct jobs seen at the
   first positions of the 247 sequences.
"""
import itertools
import random
import sys

import numpy as np

sys.path.insert(0, "/opt/scratch")
from circle_pool import N, SIZE, golden, space_size, unrank   # noqa: E402

# ---- 1. self-test ---------------------------------------------------------
n, m = 3, 2
S = space_size(n, m)
seqs = [tuple(unrank(r, n, m)) for r in range(S)]
brute = sorted(set(itertools.permutations([j for j in range(1, n + 1) for _ in range(m)])))
assert S == 90 and seqs == brute, "unrank is wrong"
print("self-test: unrank enumerates all %d sequences of 3 jobs x 2, in "
      "lexicographic order, exactly" % S)


def circle(instance, g):
    n, m = SIZE[instance]
    S = space_size(n, m)
    arc = S // N
    num, den = golden(g).as_integer_ratio()
    o = arc * num // den
    return np.array([unrank(o + (k * S) // N, n, m) for k in range(N)], dtype=np.int16)


def random_pool(instance, rng, count):
    n, m = SIZE[instance]
    base = [j for j in range(1, n + 1) for _ in range(m)]
    out = []
    for _ in range(count):
        s = base[:]
        rng.shuffle(s)
        out.append(s)
    return np.array(out, dtype=np.int16)


def pairwise(pool):
    L = pool.shape[1]
    d = []
    for i in range(len(pool)):
        d.append((pool[i + 1:] != pool[i]).sum(axis=1) / L)
    return float(np.concatenate(d).mean())


def nearest(pool, probes):
    L = pool.shape[1]
    best = np.full(len(probes), 1.0)
    for s in pool:
        best = np.minimum(best, (probes != s).sum(axis=1) / L)
    return best


rng = random.Random(20260923)
print()
print("%-6s %-8s %10s %14s %14s %8s" % ("inst", "seeds", "pairwise", "nearest mean",
                                          "nearest worst", "|S|"))
for instance in ("ta23", "ta45"):
    n, m = SIZE[instance]
    probes = random_pool(instance, rng, 4000)
    c = circle(instance, 0)
    r = random_pool(instance, rng, N)
    for label, pool in (("circle", c), ("random", r)):
        near = nearest(pool, probes)
        print("%-6s %-8s %10.4f %14.4f %14.4f %8s"
              % (instance, label, pairwise(pool), near.mean(), near.min() if False else near.max(),
                 "10^%d" % (len(str(space_size(n, m))) - 1)))
    firsts = ["%d" % len(set(c[:, p])) for p in range(6)]
    print("%-6s circle: distinct jobs at positions 1-6 among the 247: %s (of %d)"
          % (instance, " ".join(firsts), n))
    print()
