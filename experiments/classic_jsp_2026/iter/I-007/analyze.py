#!/usr/bin/env python3
"""I-007: the pre-declared analysis of the concentrated attempt on ta29.

Usage: analyze.py attempt

Every makespan is recomputed from its schedule, never read from the solver.
Nothing is accepted or rejected. Endpoints, in order:

1. RECORD. Any verified makespan at or below the BKS of 1625. The schedule is
   the proof, so the certificate and the chunk are printed for anything at
   1627 or below.
2. The minimum per cell, which is what the attempt is for, with the gap to the
   BKS and the CPU spent to get it.
3. The mean of the three lowest per cell, the tail endpoint the protocol
   requires, and the whole lower tail up to BKS + 5 so the shape is visible.
4. How many runs of each cell landed at or below each threshold from BKS to
   BKS + 5. This is the run-length question stated the way a record hunt cares
   about: at equal CPU, which budget puts more mass in the bottom.

Reported and deciding nothing: the mean and the spread per cell.
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
SECONDS = {"b040": 40, "b100": 100, "b300": 300}
PRINT_AT_OR_BELOW = 1627


def cells():
    found = {}
    for line in open(os.path.join(HERE, "jobs_attempt.tsv")):
        if line.strip() and not line.startswith("#"):
            tag = line.split("\t")[1].strip()[len("I-007_"):]
            found.setdefault(tag.split("_")[0], []).append(tag)
    return found


def main():
    if (sys.argv[1:2] or [""])[0] != "attempt":
        raise SystemExit(__doc__)
    groups = cells()
    data = {}
    for cell, tags in sorted(groups.items()):
        values = []
        for tag in tags:
            directory = os.path.join(EXPERIMENT, "results", "I-007_%s" % tag)
            if os.path.isdir(directory):
                for v in makespans(directory, ORLIB, INSTANCE):
                    values.append((v, tag))
        data[cell] = sorted(values)

    print("=== 1. RECORDS and everything at or below %d (BKS %d) ===" % (PRINT_AT_OR_BELOW, BKS))
    any_record = False
    shown = 0
    for cell in sorted(data):
        for value, tag in data[cell]:
            if value <= PRINT_AT_OR_BELOW:
                mark = "*** RECORD ***" if value <= BKS else ("MATCH" if value == BKS else "near")
                print("  %-5s %s  makespan %d, gap %+d   (chunk %s)"
                      % (cell, mark, value, value - BKS, tag))
                shown += 1
                if value <= BKS:
                    any_record = True
    if not shown:
        print("  nothing at or below %d" % PRINT_AT_OR_BELOW)
    print("  RECORD FOUND" if any_record else "  no record")

    print("\n=== 2. the minimum per cell ===")
    print("%-6s %6s %8s %8s %9s %12s"
          % ("cell", "s/run", "runs", "best", "gap", "CPU-hours"))
    for cell in sorted(data):
        v = [x for x, _ in data[cell]]
        if not v:
            continue
        print("%-6s %6d %8d %8d %+9d %12.2f"
              % (cell, SECONDS[cell], len(v), min(v), min(v) - BKS,
                 SECONDS[cell] * len(v) / 3600.0))

    print("\n=== 3. the lower tail per cell ===")
    for cell in sorted(data):
        v = [x for x, _ in data[cell]]
        if len(v) < 3:
            continue
        tail = [x for x in v if x <= BKS + 5]
        print("%-6s three lowest mean %7.1f   values <= BKS+5: %s"
              % (cell, statistics.mean(v[:3]), tail if tail else "none"))

    print("\n=== 4. runs at or below each threshold, at equal CPU ===")
    print("%-6s %s" % ("cell", "".join("%9s" % ("<=%d" % (BKS + d)) for d in range(0, 6))))
    for cell in sorted(data):
        v = [x for x, _ in data[cell]]
        print("%-6s %s" % (cell, "".join("%9d" % sum(1 for x in v if x <= BKS + d)
                                         for d in range(0, 6))))

    print("\n=== reported, deciding nothing ===")
    print("%-6s %10s %10s %8s" % ("cell", "mean", "sd", "worst"))
    for cell in sorted(data):
        v = [x for x, _ in data[cell]]
        if len(v) < 2:
            continue
        print("%-6s %10.1f %10.1f %8d"
              % (cell, statistics.mean(v), statistics.stdev(v), max(v)))


if __name__ == "__main__":
    main()
