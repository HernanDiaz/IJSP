#!/usr/bin/env python3
"""I-008: the pre-declared analysis of the ta29 attempt at 300 s a run.

Usage: analyze.py attempt

Every makespan is recomputed from its schedule, never read from the solver.
Nothing is accepted or rejected.

A RECORD is a verified makespan **strictly below** 1625. Equalling 1625 is a
match, which this line has already achieved twice; the distinction cost a
mislabelled headline in I-007 and is spelled out here so it cannot happen
again.

Endpoints, in order:

1. Anything at or below 1627, labelled record, match or near, with its chunk
   so the schedule can be pulled and re-verified.
2. The minimum over the batch and its gap.
3. The whole lower tail up to 1630, and the mean of the three lowest.
4. Counts at or below each threshold from 1625 to 1630, laid beside I-007's
   300 s cell so the two batches of the same configuration can be pooled by
   eye.

Reported and deciding nothing: mean, spread, worst.
"""
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans                                # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
INSTANCE = "ta29"
BKS = 1625
PRINT_AT_OR_BELOW = 1627

# I-007's 300 s cell, for the side-by-side: 27 runs, minimum 1625.
I007_PATTERN = "I-007_b300_c*"


def collect(pattern):
    values = []
    for directory in sorted(glob.glob(os.path.join(EXPERIMENT, "results", pattern))):
        for v in makespans(directory, ORLIB, INSTANCE):
            values.append((v, os.path.basename(directory)))
    return sorted(values)


def main():
    if (sys.argv[1:2] or [""])[0] != "attempt":
        raise SystemExit(__doc__)
    data = collect("I-008_c*")
    if not data:
        raise SystemExit("no results yet")
    values = [v for v, _ in data]

    print("=== 1. anything at or below %d (BKS %d) ===" % (PRINT_AT_OR_BELOW, BKS))
    record = False
    shown = 0
    for value, tag in data:
        if value <= PRINT_AT_OR_BELOW:
            if value < BKS:
                mark = "*** RECORD: strictly below the best known ***"
                record = True
            elif value == BKS:
                mark = "MATCH (equals 1625, does not beat it)"
            else:
                mark = "near"
            print("  %s  makespan %d, gap %+d   (chunk %s)"
                  % (mark, value, value - BKS, tag))
            shown += 1
    if not shown:
        print("  nothing at or below %d" % PRINT_AT_OR_BELOW)
    print("  RECORD FOUND" if record else "  no record: nothing strictly below 1625")

    print("\n=== 2. the minimum ===")
    print("  %d runs of 300 s, minimum %d, gap %+d, %.1f CPU-hours"
          % (len(values), min(values), min(values) - BKS, 300 * len(values) / 3600.0))

    print("\n=== 3. the lower tail ===")
    tail = [v for v in values if v <= 1630]
    print("  values <= 1630: %s" % (tail if tail else "none"))
    if len(values) >= 3:
        print("  mean of the three lowest: %.1f" % statistics.mean(sorted(values)[:3]))

    print("\n=== 4. counts by threshold, beside I-007's 300 s cell ===")
    other = [v for v, _ in collect(I007_PATTERN)]
    print("%-14s %8s %s" % ("batch", "runs", "".join("%9s" % ("<=%d" % t)
                                                     for t in range(BKS, BKS + 6))))
    for name, v in (("I-008 (this)", values), ("I-007 b300", other)):
        if v:
            print("%-14s %8d %s" % (name, len(v),
                                    "".join("%9d" % sum(1 for x in v if x <= t)
                                            for t in range(BKS, BKS + 6))))
    pooled = values + other
    if other:
        print("%-14s %8d %s" % ("pooled", len(pooled),
                                "".join("%9d" % sum(1 for x in pooled if x <= t)
                                        for t in range(BKS, BKS + 6))))

    print("\n=== reported, deciding nothing ===")
    print("  mean %.1f, sd %.1f, worst %d"
          % (statistics.mean(values),
             statistics.stdev(values) if len(values) > 1 else 0.0, max(values)))


if __name__ == "__main__":
    main()
