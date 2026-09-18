#!/usr/bin/env python3
"""Compare two configurations over the same instances, with a paired test.

Makespans are recomputed from the certificates, so a configuration cannot win
by reporting a number its schedule does not support.

The comparison is paired by instance, because instances differ from each other
far more than configurations do: an unpaired test over pooled makespans would
be dominated by which instances happen to be hard. Reports the per-instance
difference, a sign test on best-of-runs, and a Wilcoxon signed-rank test on the
per-instance mean, so a result that rests on one or two instances is visible as
such rather than hidden in an average.
"""
import argparse
import csv
import glob
import itertools
import math
import os
import statistics
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from verify_certificate import check_run, load_certificate, load_orlib


def makespans(directory, orlib, instance):
    routes, n_jobs, n_machines = load_orlib(orlib, instance)
    values = []
    for path in sorted(glob.glob(os.path.join(
            directory, instance + "_*_Certificate.csv"))):
        for _, rows in sorted(load_certificate(path).items()):
            makespan, errors = check_run(rows, routes, n_jobs, n_machines)
            if errors:
                raise SystemExit("%s in %s is infeasible" % (instance, directory))
            values.append(makespan)
    return values


def sign_test(wins, losses):
    """Two-sided probability of a split this lopsided under a fair coin."""
    n = wins + losses
    if n == 0:
        return 1.0
    extreme = max(wins, losses)
    tail = sum(math.comb(n, k) for k in range(extreme, n + 1)) / 2.0 ** n
    return min(1.0, 2.0 * tail)


def wilcoxon(differences):
    """Signed-rank statistic with an exact two-sided p (small samples)."""
    values = [d for d in differences if d != 0]
    n = len(values)
    if n == 0:
        return None, 1.0

    order = sorted(range(n), key=lambda i: abs(values[i]))
    ranks = [0.0] * n
    position = 0
    while position < n:
        stop = position
        while (stop + 1 < n
               and abs(values[order[stop + 1]]) == abs(values[order[position]])):
            stop += 1
        shared = (position + stop) / 2.0 + 1
        for k in range(position, stop + 1):
            ranks[order[k]] = shared
        position = stop + 1

    positive = sum(r for r, v in zip(ranks, values) if v > 0)
    negative = sum(r for r, v in zip(ranks, values) if v < 0)
    statistic = min(positive, negative)

    # Exact null: every sign assignment is equally likely.
    if n > 20:
        mean = n * (n + 1) / 4.0
        sd = math.sqrt(n * (n + 1) * (2 * n + 1) / 24.0)
        z = (statistic - mean) / sd
        return statistic, min(1.0, 2.0 * 0.5 * math.erfc(abs(z) / math.sqrt(2)))

    count = 0
    for signs in itertools.product((1, -1), repeat=n):
        pos = sum(r for r, s in zip(ranks, signs) if s > 0)
        if min(pos, sum(ranks) - pos) <= statistic:
            count += 1
    return statistic, min(1.0, count / 2.0 ** n)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--a", required=True, help="results directory A")
    parser.add_argument("--b", required=True, help="results directory B")
    parser.add_argument("--orlib", required=True)
    parser.add_argument("--bounds", required=True)
    parser.add_argument("instances", nargs="+")
    args = parser.parse_args()

    lb = {}
    for row in csv.DictReader(open(args.bounds)):
        lb[row["instance"]] = int(row["lb"])

    name_a, name_b = os.path.basename(args.a), os.path.basename(args.b)
    print("%-7s %8s %8s %8s   %8s %8s %8s"
          % ("inst", "A best", "B best", "diff", "A mean", "B mean", "diff"))
    print("-" * 62)

    best_wins = best_losses = 0
    mean_differences, best_gaps_a, best_gaps_b = [], [], []
    for instance in args.instances:
        a, b = (makespans(args.a, args.orlib, instance),
                makespans(args.b, args.orlib, instance))
        if not a or not b:
            print("%-7s (missing runs)" % instance)
            continue
        best_a, best_b = min(a), min(b)
        mean_a, mean_b = statistics.mean(a), statistics.mean(b)
        best_wins += best_b < best_a
        best_losses += best_b > best_a
        mean_differences.append(mean_a - mean_b)
        best_gaps_a.append(100.0 * (best_a - lb[instance]) / lb[instance])
        best_gaps_b.append(100.0 * (best_b - lb[instance]) / lb[instance])
        print("%-7s %8g %8g %+8g   %8.1f %8.1f %+8.1f"
              % (instance, best_a, best_b, best_b - best_a, mean_a, mean_b,
                 mean_b - mean_a))

    print("-" * 62)
    print("A = %s" % name_a)
    print("B = %s" % name_b)
    print("\nmean best gap to LB:  A %.3f%%   B %.3f%%"
          % (statistics.mean(best_gaps_a), statistics.mean(best_gaps_b)))

    ties = len(args.instances) - best_wins - best_losses
    print("\nbest-of-runs: B wins %d, loses %d, ties %d  -- sign test p = %.3f"
          % (best_wins, best_losses, ties, sign_test(best_wins, best_losses)))

    statistic, p = wilcoxon(mean_differences)
    positive = sum(1 for d in mean_differences if d > 0)
    print("per-instance mean: B better on %d of %d  -- Wilcoxon W = %s, "
          "p = %.3f" % (positive, len(mean_differences), statistic, p))

    if p < 0.05:
        print("\nB is better at the 5%% level on the per-instance mean.")
    else:
        print("\nNot separable at the 5%% level. The direction may still be "
              "real; this many\ninstances and runs cannot establish it.")


if __name__ == "__main__":
    main()
