#!/usr/bin/env python3
"""I-030: the pre-declared analysis, over whichever waves have run.

Usage: analyze.py

Every makespan recomputed from its schedule. Per instance: the mean of each
cell and the difference current - original; the paired Wilcoxon over the 21
instances and how many instances each side wins; the best of each cell. Nothing
is accepted or rejected: both mechanisms are already accepted, and this says
whether their gain holds at 300 s.
"""
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon                  # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["original", "current"]


def main():
    data = {}
    instances = set()
    for cell in CELLS:
        for d in sorted(glob.glob(os.path.join(EXPERIMENT, "results", "I-030_w*_s*_%s" % cell))):
            for path in glob.glob(os.path.join(d, "*_Certificate.csv")):
                inst = os.path.basename(path).split("_")[0]
                instances.add(inst)
                data.setdefault((cell, inst), []).extend(makespans(d, ORLIB, inst))
    insts = sorted(instances)
    print("%-6s %12s %12s %9s   %8s %8s   runs" % ("inst", "mean orig", "mean curr", "d",
                                                    "best o", "best c"))
    diffs = []
    for i in insts:
        a, b = data.get(("original", i), []), data.get(("current", i), [])
        if not a or not b:
            continue
        d = statistics.mean(b) - statistics.mean(a)
        diffs.append(d)
        print("%-6s %12.1f %12.1f %+9.2f   %8d %8d   %d/%d"
              % (i, statistics.mean(a), statistics.mean(b), d, min(a), min(b), len(a), len(b)))
    W, p = wilcoxon(diffs)
    print("\n%d instances: current better on %d, worse on %d; mean difference %+.2f; "
          "Wilcoxon W = %s, p = %.4f"
          % (len(diffs), sum(1 for x in diffs if x < 0), sum(1 for x in diffs if x > 0),
             statistics.mean(diffs), W, p))


if __name__ == "__main__":
    main()
