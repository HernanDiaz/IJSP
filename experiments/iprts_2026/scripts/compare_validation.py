#!/usr/bin/env python3
"""Compare IPRTS validation results against the COR 2026 TS-N2 baseline.

Usage:
    python3 compare_validation.py [results_dir] [baseline_csv] [neigh]

Parses the "Best solution;" row (E[makespan]: avg/best/worst/sd over runs)
of every FuzzyFW CSV in results_dir and joins it with the paper's
per_instance_stats.csv (default baseline: tuned TS-N2, 30 runs, 900 s budget).
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


def parse_results(results_dir):
    """instance -> dict(avg, best, worst, sd, n_runs) from FuzzyFW CSVs."""
    out = {}
    for path in sorted(glob.glob(os.path.join(results_dir, "*.csv"))):
        name = os.path.basename(path)
        match = re.match(r"(F[\d.]+\.\w+)_\d{14}\.csv$", name)
        if not match:  # skip Robustness/Sols/Scenarios companions
            continue
        instance = match.group(1)
        entry = {}
        with open(path) as fh:
            for line in fh:
                if line.startswith("Best solution;"):
                    fields = line.strip().split(";")
                    entry.update({
                        "avg": float(fields[1]),
                        "best": float(fields[2]),
                        "worst": float(fields[3]),
                        "sd": float(fields[4]),
                        "n_runs": len(fields) - 5,
                    })
                elif line.startswith("Total runtime;"):
                    runs = sorted(float(v) for v in line.strip().split(";")[5:])
                    mid = len(runs) // 2
                    median = runs[mid] if len(runs) % 2 \
                        else (runs[mid - 1] + runs[mid]) / 2.0
                    entry["rt_med"] = median
                if "avg" in entry and "rt_med" in entry:
                    break
        if entry:
            out[instance] = entry
    return out


def parse_baseline(baseline_csv, neigh):
    """instance -> dict(best_E, avg_E, sd_E, med_rt) for one neighbourhood."""
    out = {}
    with open(baseline_csv) as fh:
        for row in csv.DictReader(fh):
            if row["neigh"].lower() == neigh.lower():
                out[row["instance"]] = {
                    "best_E": float(row["best_E"]),
                    "avg_E": float(row["avg_E"]),
                    "sd_E": float(row["sd_E"]),
                    "med_rt": float(row["med_rt"]),
                }
    return out


def main():
    results = parse_results(RESULTS)
    baseline = parse_baseline(BASELINE, NEIGH)
    if not results:
        sys.exit("No result CSVs found in %s" % RESULTS)

    header = ("instance", "runs", "IPRTS_avg", "IPRTS_best",
              "N2_avg", "N2_best", "d_avg%", "d_best%", "rt_med", "N2_rt")
    print("%-22s %4s %10s %10s %9s %8s %7s %7s %7s %7s" % header)
    deltas_avg, deltas_best = [], []
    for instance in sorted(results):
        res = results[instance]
        base = baseline.get(instance)
        if base is None:
            print("%-22s  (no baseline row)" % instance)
            continue
        d_avg = 100.0 * (res["avg"] - base["avg_E"]) / base["avg_E"]
        d_best = 100.0 * (res["best"] - base["best_E"]) / base["best_E"]
        deltas_avg.append(d_avg)
        deltas_best.append(d_best)
        print("%-22s %4d %10.2f %10.1f %9.2f %8.1f %+7.2f %+7.2f %7.1f %7.1f"
              % (instance, res["n_runs"], res["avg"], res["best"],
                 base["avg_E"], base["best_E"], d_avg, d_best,
                 res.get("rt_med", -1), base["med_rt"]))
    if deltas_avg:
        print("-" * 102)
        print("%-22s %4s %10s %10s %9s %8s %+7.2f %+7.2f"
              % ("MEAN DELTA", "", "", "", "", "",
                 sum(deltas_avg) / len(deltas_avg),
                 sum(deltas_best) / len(deltas_best)))


if __name__ == "__main__":
    main()
