#!/usr/bin/env python3
"""Anytime analysis of IPRTS validation runs vs the TS-N2 baseline.

For every instance:
  - t_target : median time (s) at which IPRTS runs first reach the baseline's
               avg_E (runs that never reach it count as the 60 s cap).
  - reach    : how many runs reached the target within the budget.
  - E@N2rt   : mean IPRTS best-E at the moment the baseline self-converged
               (its med_rt), i.e. equal-time quality comparison.

Usage: python3 time_to_target.py [results_dir] [baseline_csv] [neigh]
"""
import csv
import glob
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    HERE, "..", "results", "validation_v1")
BASELINE = sys.argv[2] if len(sys.argv) > 2 else os.path.join(
    HERE, "..", "..", "cor_tabu_2026", "statistical_results",
    "per_instance_stats.csv")
NEIGH = sys.argv[3] if len(sys.argv) > 3 else "n2"
CAP = 60.0


def parse_evolution(path):
    """list of runs; each run = list of (runtime, best)."""
    runs = []
    with open(path) as fh:
        lines = fh.read().splitlines()
    start = None
    for i, line in enumerate(lines):
        if line.startswith("Step;Runtime;Best;Avg"):
            n_runs = line.count("Generation")
            start = i + 1
            break
    if start is None:
        return runs
    runs = [[] for _ in range(n_runs)]
    for line in lines[start:]:
        cells = line.split(";")
        if len(cells) < 4 + 4 * n_runs or not cells[0]:
            break
        try:
            for r in range(n_runs):
                rt = float(cells[4 + 4 * r + 1])
                best = float(cells[4 + 4 * r + 2])
                runs[r].append((rt, best))
        except ValueError:
            break
    return runs


def first_reach(run, target):
    for rt, best in run:
        if best <= target + 1e-9:
            return rt
    return None


def best_at(run, t):
    val = None
    for rt, best in run:
        if rt <= t + 1e-9:
            val = best
        else:
            break
    return val


def median(vals):
    vals = sorted(vals)
    mid = len(vals) // 2
    return vals[mid] if len(vals) % 2 else (vals[mid - 1] + vals[mid]) / 2.0


def main():
    baseline = {}
    with open(BASELINE) as fh:
        for row in csv.DictReader(fh):
            if row["neigh"].lower() == NEIGH.lower():
                baseline[row["instance"]] = (
                    float(row["avg_E"]), float(row["med_rt"]))

    print("%-22s %9s %6s %10s %9s" % (
        "instance", "t_target", "reach", "E@N2rt", "N2_avg"))
    for path in sorted(glob.glob(os.path.join(RESULTS, "*.csv"))):
        m = re.match(r"^(.+)_\d{14}\.csv$", os.path.basename(path))
        if not m or m.group(1) not in baseline:
            continue
        inst = m.group(1)
        target, n2_rt = baseline[inst]
        runs = parse_evolution(path)
        if not runs:
            print("%-22s  (no evolution data)" % inst)
            continue
        times = [first_reach(r, target) for r in runs]
        reached = sum(1 for t in times if t is not None)
        t_med = median([t if t is not None else CAP for t in times])
        at_rt = [b for b in (best_at(r, n2_rt) for r in runs) if b is not None]
        e_at = sum(at_rt) / len(at_rt) if at_rt else float("nan")
        mark = "" if reached == len(runs) else ">"
        print("%-22s %8s%.1f %4d/%d %10.1f %9.2f" % (
            inst, mark, t_med, reached, len(runs), e_at, target))


if __name__ == "__main__":
    main()
