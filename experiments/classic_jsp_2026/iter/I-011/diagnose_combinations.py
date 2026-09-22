#!/usr/bin/env python3
"""Reported diagnostic, deciding nothing: which combination moved the filter?

Every portfolio chunk p<NN> carries combination NN-1, so the filter's chunks
cover combinations 0..14 with 2 runs per instance each. Recomputes every
makespan from its schedule, like every other analysis here.
"""
import glob
import os
import statistics
import sys

EXPERIMENT = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans                                  # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
INSTANCES = ["ta23", "ta29", "ta30", "ta45"]
NAMES = {1: "seeded", 2: "tails", 4: "kick", 8: "escape"}

control = {}
for instance in INSTANCES:
    d = os.path.join(EXPERIMENT, "results", "I-011_filter_p01_control")
    control[instance] = statistics.mean(makespans(d, ORLIB, instance))

print("%-4s %-28s %6s %6s %6s %6s %8s"
      % ("comb", "switches", "ta23", "ta29", "ta30", "ta45", "mean d"))
rows = []
for piece in range(1, 16):
    comb = piece - 1
    d = os.path.join(EXPERIMENT, "results",
                     "I-011_filter_p%02d_portfolio" % piece)
    if not os.path.isdir(d):
        continue
    label = ", ".join(NAMES[b] for b in (1, 2, 4, 8) if comb & b) or "(frozen)"
    diffs, cells = [], []
    for instance in INSTANCES:
        v = makespans(d, ORLIB, instance)
        m = statistics.mean(v)
        cells.append(m - control[instance])
        diffs.append(m - control[instance])
    rows.append((statistics.mean(diffs), comb, label))
    print("%-4d %-28s %+6.1f %+6.1f %+6.1f %+6.1f %+8.2f"
          % (comb, label, cells[0], cells[1], cells[2], cells[3],
             statistics.mean(diffs)))

print("\nsorted by mean difference to control (2 runs per cell, so this is noise-dominated):")
for mean, comb, label in sorted(rows):
    print("  %+7.2f  comb %-3d %s" % (mean, comb, label))
print("\nspread of the 15 combination means: %.2f" % statistics.stdev(r[0] for r in rows))
print("if one switch were carrying the result, its combinations would cluster low.")
for bit in (1, 2, 4, 8):
    on = [r[0] for r in rows if r[1] & bit]
    off = [r[0] for r in rows if not r[1] & bit]
    print("  %-7s on %+6.2f (n=%2d)   off %+6.2f (n=%2d)   difference %+6.2f"
          % (NAMES[bit], statistics.mean(on), len(on),
             statistics.mean(off), len(off),
             statistics.mean(on) - statistics.mean(off)))
