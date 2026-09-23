#!/usr/bin/env python3
"""Why do circle seeds decode to better schedules than random ones when their
coverage and diversity are the same?

Hypothesis: equally spaced ranks give sequences in which each job's operations
are spread more EVENLY along the sequence than chance would, i.e. closer to a
round-robin dispatch, which in JSP decodes to better schedules.

Measure, per sequence, the prefix imbalance: at every prefix length t, how far
the most over- or under-represented job is from its fair share t/n,
|c_j(t) - t/n|, averaged over t. A round-robin sequence scores under 1; a
random one drifts like a random walk.
"""
import random
import sys

import numpy as np

sys.path.insert(0, "/opt/scratch")
from circle_pool import N, SIZE                    # noqa: E402
from circle_probe import circle, random_pool       # noqa: E402  (runs its checks)


def imbalance(seq, n):
    L = len(seq)
    counts = np.zeros(n)
    total = 0.0
    for t, j in enumerate(seq, start=1):
        counts[j - 1] += 1
        total += np.abs(counts - t / n).max()
    return total / L


def first_positions(pool, n):
    """Mean position (as a share of the sequence) of each job's first op."""
    L = pool.shape[1]
    out = []
    for s in pool:
        firsts = [np.argmax(s == j) / L for j in range(1, n + 1)]
        out.append(np.mean(firsts))
    return float(np.mean(out))


rng = random.Random(7)
print("\n%-6s %-8s %16s %22s" % ("inst", "seeds", "prefix imbalance",
                                   "job's 1st op, mean pos."))
for instance in ("ta23", "ta45"):
    n, m = SIZE[instance]
    for label, pool in (("circle", circle(instance, 0)),
                        ("random", random_pool(instance, rng, N))):
        imb = [imbalance(s, n) for s in pool]
        print("%-6s %-8s %10.2f +- %4.2f %22.3f"
              % (instance, label, np.mean(imb), np.std(imb), first_positions(pool, n)))
    print()
