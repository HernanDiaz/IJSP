#!/usr/bin/env python3
"""I-032: the pre-declared analysis of the hunt.

Usage: analyze.py hunt

Every makespan is recomputed from its schedule by the independent checker's
functions. Pre-declared, in order:

1. RECORD: any run strictly below the best known. The proof is the schedule.
2. MATCH: any run equal to the best known, and how many DISTINCT schedules
   (by start times) sit there, which measures the plateau at the best known.
3. Best of the 800 and the lower quantiles of the run distribution, so the
   distance to the record is on the table.
"""
import csv
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from verify_certificate import check_run, load_certificate, load_orlib  # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
BKS = {r["instance"]: int(r["bks"]) for r in csv.DictReader(
    open(os.path.join(EXPERIMENT, "taillard_bounds.csv")))}
LB = {r["instance"]: int(r["lb"]) for r in csv.DictReader(
    open(os.path.join(EXPERIMENT, "taillard_bounds.csv")))}
INSTANCES = ["ta23"]


def main():
    if (sys.argv[1:2] or [""])[0] != "hunt":
        raise SystemExit(__doc__)
    for inst in INSTANCES:
        routes, nj, nm = load_orlib(ORLIB, inst)
        values, at_bks, below = [], {}, []
        for path in sorted(glob.glob(os.path.join(
                EXPERIMENT, "results", "I-032_c*", inst + "_*_Certificate.csv"))):
            for run, rows in sorted(load_certificate(path).items()):
                ms, errors = check_run(rows, routes, nj, nm)
                if errors:
                    raise SystemExit("%s run %d is infeasible" % (path, run))
                ms = int(round(ms))
                values.append(ms)
                key = tuple(sorted((r["task"], r["start"]) for r in rows))
                if ms < BKS[inst]:
                    below.append((ms, os.path.relpath(path, EXPERIMENT), run))
                elif ms == BKS[inst]:
                    at_bks.setdefault(key, []).append((os.path.relpath(path, EXPERIMENT), run))
        values.sort()
        n = len(values)
        print("=== %s: BKS %d, lower bound %d, %d runs ===" % (inst, BKS[inst], LB[inst], n))
        print("1. RECORDS (strictly below the BKS): %d" % len(below))
        for ms, path, run in below:
            print("   *** RECORD *** %d  %s run %d" % (ms, path, run))
        matches = sum(len(v) for v in at_bks.values())
        print("2. MATCHES (at the BKS): %d runs, %d distinct schedules" % (matches, len(at_bks)))
        for k, runs in enumerate(at_bks.values(), 1):
            print("   schedule %d: %s" % (k, ", ".join("%s run %d" % r for r in runs)))
        q = lambda f: values[min(n - 1, int(f * n))]
        print("3. best %d; quantiles 1%% %d, 5%% %d, 10%% %d, 50%% %d; mean %.1f"
              % (values[0], q(0.01), q(0.05), q(0.10), q(0.50), statistics.mean(values)))
        print()


if __name__ == "__main__":
    main()
