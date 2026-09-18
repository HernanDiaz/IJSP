#!/usr/bin/env python3
"""Show where the Taillard benchmark is actually hard, and why.

Size is a poor predictor of difficulty in the job shop. What predicts it is the
jobs-to-machines ratio, through the strength of the trivial lower bound:

    trivial LB = max( busiest machine's total workload , longest job )

When jobs greatly outnumber machines, the bottleneck machine is so loaded that
almost any schedule keeping it busy is optimal, and the published lower bound
sits within a fraction of a percent of this trivial one. When the instance is
square, the trivial bound is weak, establishing the real bound took serious
branch-and-bound work, and the gap to the best known solution stays open.

This script prints, per size group, how often the trivial bound already equals
the published lower bound, how far the published bound rises above it, and how
many instances remain open -- so the three quantities can be read together.
"""
import argparse
import csv
import os
import statistics
import sys
from collections import OrderedDict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_certificate import load_orlib


def trivial_bound(routes):
    """max(busiest machine workload, longest job) -- both are valid bounds."""
    machine_load = {}
    for job in routes:
        for machine, duration in job:
            machine_load[machine] = machine_load.get(machine, 0) + duration
    longest_job = max(sum(d for _, d in job) for job in routes)
    return max(max(machine_load.values()), longest_job)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--orlib", required=True)
    parser.add_argument("--bounds", required=True)
    args = parser.parse_args()

    groups = OrderedDict()
    for row in csv.DictReader(open(args.bounds)):
        routes, n_jobs, n_machines = load_orlib(args.orlib, row["instance"])
        groups.setdefault((n_jobs, n_machines), []).append(
            (trivial_bound(routes), int(row["lb"]), int(row["bks"]),
             row["status"]))

    print("%-9s %5s %8s %10s %9s %9s" % ("group", "n/m", "LB==triv",
                                         "LB>triv", "open", "openness"))
    print("-" * 56)
    for (n_jobs, n_machines), rows in groups.items():
        equal = sum(1 for t, lb, _, _ in rows if t == lb)
        lift = statistics.mean(100.0 * (lb - t) / t for t, lb, _, _ in rows)
        openi = [r for r in rows if r[3] == "open"]
        gap = (statistics.mean(100.0 * (bks - lb) / lb
                               for _, lb, bks, _ in openi) if openi else 0.0)
        print("%-9s %5.1f %8s %9.2f%% %9s %8.2f%%"
              % ("%dx%d" % (n_jobs, n_machines), n_jobs / float(n_machines),
                 "%d/%d" % (equal, len(rows)), lift,
                 "%d/%d" % (len(openi), len(rows)), gap))

    print("\nLB==triv  the trivial bound already equals the published one")
    print("LB>triv   how far the published bound rises above the trivial one")
    print("openness  mean (BKS-LB)/LB over the group's open instances")


if __name__ == "__main__":
    main()
