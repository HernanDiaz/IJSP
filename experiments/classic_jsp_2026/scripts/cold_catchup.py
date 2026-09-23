#!/usr/bin/env python3
"""How far does a COLD start get in the time a stall restart would leave it?

I-014 rebuilds the population from the creation operator when the run has
stalled. That only pays if a fresh population can beat the incumbent in the
budget that remains. This reads the 120 control traces already on disk and
answers it directly: the median best a run has reached after t seconds from a
cold start, against the median final best.

Nothing is run.
"""
import glob
import os
import statistics
import sys

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
INSTANCES = [("ta23", 40), ("ta29", 40), ("ta30", 40), ("ta45", 150)]


def traces(instance):
    out = []
    for directory in sorted(glob.glob(os.path.join(
            E, "results", "I-013_filter_p*_control"))):
        for path in sorted(glob.glob(os.path.join(directory, instance + "_*.csv"))):
            if path.endswith("_Certificate.csv") or path.endswith("_Sols.csv"):
                continue
            rows, seen = [], False
            for line in open(path):
                if line.startswith("Step;Runtime;"):
                    seen = True
                    continue
                if not seen:
                    continue
                f = line.strip().split(";")
                if len(f) < 5:
                    continue
                try:
                    rows.append((float(f[1]), float(f[4])))
                except ValueError:
                    continue
            if rows:
                out.append(rows)
    return out


def best_at(rows, t):
    v = None
    for runtime, cmax in rows:
        if runtime > t:
            break
        v = cmax if v is None else min(v, cmax)
    return v


print("median best reached from a COLD start after t seconds")
print("(a stall restart at 0.2 of the budget leaves at most 0.8 of it, and in")
print(" practice far less, because it fires late)\n")
for instance, budget in INSTANCES:
    runs = traces(instance)
    if not runs:
        continue
    finals = [min(c for _, c in rows) for rows in runs]
    print("%s, budget %d s, %d runs, median final best %.1f"
          % (instance, budget, len(runs), statistics.median(finals)))
    print("  %-10s %12s %14s" % ("t (s)", "median best", "vs final"))
    for frac in (0.1, 0.2, 0.4, 0.6, 0.8, 1.0):
        t = budget * frac
        vals = [best_at(rows, t) for rows in runs]
        vals = [v for v in vals if v is not None]
        if not vals:
            continue
        m = statistics.median(vals)
        print("  %-10.1f %12.1f %+13.1f" % (t, m, m - statistics.median(finals)))
    print()
