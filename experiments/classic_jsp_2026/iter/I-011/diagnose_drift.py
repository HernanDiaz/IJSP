#!/usr/bin/env python3
"""Does a run get worse the later it sits inside its process?

The budget is wall-clock, so a run is worth whatever it manages to compute in
its seconds. The control cell is one process of 30 runs; the portfolio is
fifteen processes of 2. If quality decays with the run index inside a process,
the control is handicapped by its own chunking and I-011's filter compares
chunking, not switches.

The certificate keys runs by an integer, so the makespans come back in
execution order. Reads only what is already on disk.
"""
import os
import statistics
import sys

EXPERIMENT = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans                                  # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
INSTANCES = ["ta23", "ta29", "ta30", "ta45"]
CONTROL = os.path.join(EXPERIMENT, "results", "I-011_filter_p01_control")


def slope(xs, ys):
    mx, my = statistics.mean(xs), statistics.mean(ys)
    den = sum((x - mx) ** 2 for x in xs)
    return sum((x - mx) * (y - my) for x, y in zip(xs, ys)) / den if den else 0.0


print("control cell, one process of 30 runs per instance, in execution order")
print("%-6s %9s %9s %9s %9s %12s"
      % ("inst", "runs 1-2", "runs 1-10", "runs 11-20", "runs 21-30", "slope/run"))
slopes, firsts = [], []
for instance in INSTANCES:
    v = makespans(CONTROL, ORLIB, instance)
    if len(v) < 30:
        print("%-6s only %d runs" % (instance, len(v)))
        continue
    s = slope(list(range(1, 31)), v)
    slopes.append(s)
    firsts.append(statistics.mean(v[:2]) - statistics.mean(v))
    print("%-6s %9.1f %9.1f %9.1f %9.1f %+12.3f"
          % (instance, statistics.mean(v[:2]), statistics.mean(v[:10]),
             statistics.mean(v[10:20]), statistics.mean(v[20:]), s))

print("\nmean slope %+.3f makespan units per run index" % statistics.mean(slopes))
print("over 30 runs that is %+.2f units from first to last" % (statistics.mean(slopes) * 29))
print("\nthe comparison the portfolio's combination 0 actually made:")
print("  mean(runs 1-2) - mean(runs 1-30) = %+.2f" % statistics.mean(firsts))
print("  combination 0 is the frozen configuration, i.e. the control itself,")
print("  and it measured -0.97 against the control.")
