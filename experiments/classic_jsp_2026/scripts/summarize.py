#!/usr/bin/env python3
"""Summarise classic-JSP results from verified certificates.

Every makespan reported here is recomputed from the certificate rather than
read from the solver's own output, and any certificate that fails feasibility
is reported instead of being summarised.  That keeps the table honest even if
the solver's internal bookkeeping is wrong.
"""
import argparse
import csv
import glob
import os
import statistics
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_certificate import check_run, load_certificate, load_orlib


def summarise(directory, orlib, bounds, instances):
    table = {}
    for row in csv.DictReader(open(bounds)):
        table[row["instance"]] = (int(row["lb"]), int(row["bks"]))

    results = []
    for instance in instances:
        pattern = os.path.join(directory, instance + "_*_Certificate.csv")
        files = sorted(glob.glob(pattern))
        if not files:
            results.append((instance, None, None, None, None, "no certificate"))
            continue

        routes, n_jobs, n_machines = load_orlib(orlib, instance)
        makespans, broken = [], 0
        for path in files:
            for run, rows in sorted(load_certificate(path).items()):
                makespan, errors = check_run(rows, routes, n_jobs, n_machines)
                if errors:
                    broken += 1
                else:
                    makespans.append(makespan)

        if not makespans:
            results.append((instance, None, None, None, None, "all runs infeasible"))
            continue

        lb, bks = table.get(instance, (None, None))
        best, mean = min(makespans), statistics.mean(makespans)
        note = "%d runs" % len(makespans)
        if broken:
            note += ", %d INFEASIBLE" % broken
        results.append((instance, lb, bks, best, mean, note))
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True, help="a results directory")
    parser.add_argument("--orlib", required=True)
    parser.add_argument("--bounds", required=True)
    parser.add_argument("instances", nargs="+")
    args = parser.parse_args()

    rows = summarise(args.results, args.orlib, args.bounds, args.instances)

    print("%-8s %7s %7s %7s %9s %9s %9s  %s"
          % ("inst", "LB", "BKS", "best", "gap%", "avg", "avggap%", "note"))
    best_gaps, mean_gaps = [], []
    for instance, lb, bks, best, mean, note in rows:
        if best is None:
            print("%-8s %7s %7s %7s %9s %9s %9s  %s"
                  % (instance, "-", "-", "-", "-", "-", "-", note))
            continue
        best_gap = 100.0 * (best - lb) / lb
        mean_gap = 100.0 * (mean - lb) / lb
        best_gaps.append(best_gap)
        mean_gaps.append(mean_gap)
        flag = ""
        if best < lb:
            flag = "  <-- BELOW LOWER BOUND, investigate"
        elif best < bks:
            flag = "  <-- improves best known"
        elif best == lb:
            flag = "  <-- optimal"
        print("%-8s %7d %7d %7g %9.2f %9.1f %9.2f  %s%s"
              % (instance, lb, bks, best, best_gap, mean, mean_gap, note, flag))

    if best_gaps:
        print("\nmean gap over %d instances: best %.3f%%, average run %.3f%%"
              % (len(best_gaps), statistics.mean(best_gaps),
                 statistics.mean(mean_gaps)))


if __name__ == "__main__":
    main()
