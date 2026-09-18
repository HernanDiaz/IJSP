#!/usr/bin/env python3
"""Collect the best verified schedule found for each instance.

Scans every results directory, re-verifies each certificate against the
published instance data, and writes the best feasible schedule per instance to
solutions/<instance>.csv with a header recording the makespan, the published
bounds and where the schedule came from.

Only schedules that pass verification are written, so every file under
solutions/ is a claim that can be rechecked with verify_certificate.py.
"""
import argparse
import csv
import glob
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_certificate import check_run, load_certificate, load_orlib

FIELDS = ["task", "job", "operation", "machine", "start", "duration",
          "completion"]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results-root", required=True)
    parser.add_argument("--orlib", required=True)
    parser.add_argument("--bounds", required=True)
    parser.add_argument("--out-dir", required=True)
    args = parser.parse_args()

    bounds = {}
    for row in csv.DictReader(open(args.bounds)):
        bounds[row["instance"]] = (int(row["lb"]), int(row["bks"]))

    best = {}
    pattern = os.path.join(args.results_root, "*", "*_Certificate.csv")
    for path in sorted(glob.glob(pattern)):
        instance = os.path.basename(path).split("_")[0]
        if instance not in bounds:
            continue
        routes, n_jobs, n_machines = load_orlib(args.orlib, instance)
        for run, rows in sorted(load_certificate(path).items()):
            makespan, errors = check_run(rows, routes, n_jobs, n_machines)
            if errors:
                continue
            if instance not in best or makespan < best[instance][0]:
                source = os.path.basename(os.path.dirname(path))
                best[instance] = (makespan, rows, source, run)

    os.makedirs(args.out_dir, exist_ok=True)
    index = []
    for instance in sorted(best):
        makespan, rows, source, run = best[instance]
        lb, bks = bounds[instance]
        out_path = os.path.join(args.out_dir, instance + ".csv")
        with open(out_path, "w", newline="") as handle:
            handle.write("# instance %s, makespan %g, lower bound %d, "
                         "best known %d\n" % (instance, makespan, lb, bks))
            handle.write("# produced by %s run %d; recheck with "
                         "verify_certificate.py\n" % (source, run))
            writer = csv.DictWriter(handle, fieldnames=FIELDS, delimiter=";",
                                    lineterminator="\n")
            writer.writeheader()
            for row in sorted(rows, key=lambda r: r["task"]):
                writer.writerow({k: ("%g" % row[k] if isinstance(row[k], float)
                                     else row[k]) for k in FIELDS})
        status = ("optimal" if makespan == lb else
                  "improves best known" if makespan < bks else
                  "gap %.2f%%" % (100.0 * (makespan - lb) / lb))
        index.append((instance, lb, bks, makespan, status, source))
        print("%-6s makespan %-7g LB %-6d BKS %-6d %-20s (%s)"
              % (instance, makespan, lb, bks, status, source))

    with open(os.path.join(args.out_dir, "index.csv"), "w", newline="") as h:
        writer = csv.writer(h, lineterminator="\n")
        writer.writerow(["instance", "lb", "bks", "makespan", "status",
                         "source"])
        writer.writerows(index)


if __name__ == "__main__":
    main()
