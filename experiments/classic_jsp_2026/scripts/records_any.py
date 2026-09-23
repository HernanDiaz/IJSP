#!/usr/bin/env python3
"""Records and matches anywhere in one iteration, filter AND waves, every cell.

Usage: records_any.py <iteration id, e.g. I-016>

A record is strictly below the best known solution, a match equals it. Every
makespan is recomputed from its own schedule. The per-instance best is
printed for the record; every run at or below the best known is listed with
its results directory, so identical schedules found by the same seed in the
filter and in a wave (they share seeds 1 to 30) are visible as such.
"""
import csv
import glob
import os
import sys

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(E, "scripts"))
from verify_certificate import check_run, load_certificate, load_orlib  # noqa: E402

ORLIB = os.path.join(E, "reference", "taillard_orlib.txt")
it = sys.argv[1]
BKS = {r["instance"]: int(r["bks"])
       for r in csv.DictReader(open(os.path.join(E, "taillard_bounds.csv")))}

best, hits, total = {}, [], 0
for path in sorted(glob.glob(os.path.join(E, "results", it + "_*", "*_Certificate.csv"))):
    inst = os.path.basename(path).split("_")[0]
    routes, nj, nm = load_orlib(ORLIB, inst)
    for run, rows in sorted(load_certificate(path).items()):
        ms, errors = check_run(rows, routes, nj, nm)
        if errors:
            raise SystemExit("%s run %d is infeasible" % (path, run))
        total += 1
        ms = int(round(ms))
        if inst not in best or ms < best[inst]:
            best[inst] = ms
        if ms <= BKS[inst]:
            hits.append((inst, ms, os.path.relpath(path, E), run))

print("%-6s %6s %6s %7s" % ("inst", "best", "BKS", "gap"))
for inst in sorted(best):
    print("%-6s %6d %6d %+6.2f%%" % (inst, best[inst], BKS[inst],
                                     100.0 * (best[inst] - BKS[inst]) / BKS[inst]))
print("\n%d runs recomputed from their schedules" % total)
print("at or below the best known: %d" % len(hits))
for inst, ms, path, run in hits:
    print("  %s %d (%s) %s run %d" % (inst, ms, "RECORD" if ms < BKS[inst] else "match",
                                      path, run))
