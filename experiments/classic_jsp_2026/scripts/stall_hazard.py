#!/usr/bin/env python3
"""Does the chance of a further improvement decay with the age of the stall?

B-7 wants to end a run after S seconds without improving the global best and
let the harness start the next one. The external review of 2026-09-21 warned
that the criterion assigns backwards -- a run that finds a good incumbent early
is handed MORE sterile time than a bad one that gets cut sooner -- and that
before implementing anything one has to check that the improvement hazard
really decays with the age of the stall. If it does not, the cut has no basis.

This reads the per-generation traces already on disk: I-013's filter control
cell, 30 runs on each of four instances, at the frozen configuration and the
per-class budgets. Nothing is run.

The hazard at stall age a is the share of generations observed at age a that
produced a new global best. It is computed on the generation grid and reported
against the stall age in seconds, because that is the unit the cut would use.
"""
import glob
import os
import statistics
import sys

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
INSTANCES = ["ta23", "ta29", "ta30", "ta45"]
BUDGET = {"ta23": 40, "ta29": 40, "ta30": 40, "ta45": 150}


def traces(instance):
    """Yield [(runtime, best_cmax), ...] per run."""
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


print("stall-age hazard, I-013 filter control cell, frozen configuration")
print("(a generation 'at age a' is one whose last global-best improvement was")
print(" a seconds earlier; the hazard is the share of those that improved)\n")

EDGES = [0, 1, 2, 4, 8, 16, 32, 64, 128]
for instance in INSTANCES:
    runs = traces(instance)
    if not runs:
        print("%s: no traces" % instance)
        continue
    hit = [0] * len(EDGES)
    seen = [0] * len(EDGES)
    last_fraction = []
    for rows in runs:
        best = None
        last_improve_t = 0.0
        last_improve_at = 0.0
        for t, cmax in rows:
            age = t - last_improve_t
            b = 0
            while b + 1 < len(EDGES) and age >= EDGES[b + 1]:
                b += 1
            seen[b] += 1
            if best is None or cmax < best:
                best = cmax
                if t > 0:
                    hit[b] += 1
                last_improve_t = t
                last_improve_at = t
        if rows:
            last_fraction.append(last_improve_at / BUDGET[instance])

    print("%s (%d runs, budget %d s)" % (instance, len(runs), BUDGET[instance]))
    print("  %-14s %10s %10s %9s" % ("stall age (s)", "gens", "improved", "hazard"))
    for b, lo in enumerate(EDGES):
        hi = EDGES[b + 1] if b + 1 < len(EDGES) else None
        label = "%g-%g" % (lo, hi) if hi else ">=%g" % lo
        if seen[b]:
            print("  %-14s %10d %10d %8.2f%%"
                  % (label, seen[b], hit[b], 100.0 * hit[b] / seen[b]))
    print("  last improvement at a median of %.2f of the budget; %.1f%% of the run"
          % (statistics.median(last_fraction),
             100.0 * (1.0 - statistics.median(last_fraction))))
    print()
