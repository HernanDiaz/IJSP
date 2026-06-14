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

# Lower bounds, copied verbatim from the paper pipeline
# (cor_tabu_2026/scripts/gen_phaseB_tables.py) so RE % matches the paper.
CLASSICAL_LB = {"abz7": 656, "abz8": 645, "abz9": 661, "ft10": 930,
                "ft20": 1165, "la21": 1046, "la24": 935, "la25": 977,
                "la27": 1235, "la29": 1152, "la38": 1196, "la40": 1222}
TAILLARD_LB = {
    "tai15_15": [1231, 1244, 1218, 1175, 1224, 1238, 1227, 1217, 1274, 1241],
    "tai20_15": [1357, 1367, 1342, 1345, 1339, 1360, 1462, 1377, 1332, 1348],
    "tai20_20": [1642, 1561, 1518, 1644, 1558, 1591, 1652, 1603, 1583, 1528],
    "tai30_15": [1764, 1774, 1788, 1828, 2007, 1819, 1771, 1673, 1795, 1651],
    "tai30_20": [1906, 1884, 1809, 1948, 1997, 1957, 1807, 1912, 1931, 1833],
    "tai50_15": [2760, 2756, 2717, 2839, 2679, 2781, 2943, 2885, 2655, 2723],
    "tai50_20": [2868, 2869, 2755, 2702, 2725, 2845, 2825, 2784, 3071, 2995],
}


def lb_of(stem):
    if stem.startswith("F0.15.0."):
        m = re.match(r"F0\.15\.0\.([a-z]+[0-9]+)_", stem)
        return CLASSICAL_LB.get(m.group(1)) if m else None
    m = re.match(r"^(tai\d+_\d+)_(\d+)", stem)
    if m and m.group(1) in TAILLARD_LB:
        return TAILLARD_LB[m.group(1)][int(m.group(2)) - 1]
    return None
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
        # Stem before the run timestamp: matches both selecta
        # (F0.15.0.abz7_06_<ts>.csv) and Taillard
        # (tai50_20_01.F.15_01_<ts>.csv) main outputs, while excluding the
        # _Robustness/_Sols/_Scenarios companions (which do not end _<14d>.csv)
        match = re.match(r"^(.+)_\d{14}\.csv$", name)
        if not match:
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

    header = ("instance", "LB", "IPRTS_avg", "RE_a%", "IPRTS_bst", "RE_b%",
              "N2_avg", "N2REa%", "N2_bst", "N2REb%", "rt", "N2rt")
    print("%-22s %5s %9s %6s %9s %6s %8s %6s %7s %6s %5s %5s" % header)
    re_a, re_b, n2re_a, n2re_b = [], [], [], []
    for instance in sorted(results):
        res = results[instance]
        base = baseline.get(instance)
        lb = lb_of(instance)
        if base is None or lb is None:
            print("%-22s  (no baseline row or LB)" % instance)
            continue
        rea = 100.0 * (res["avg"] - lb) / lb
        reb = 100.0 * (res["best"] - lb) / lb
        n2a = 100.0 * (base["avg_E"] - lb) / lb
        n2b = 100.0 * (base["best_E"] - lb) / lb
        re_a.append(rea)
        re_b.append(reb)
        n2re_a.append(n2a)
        n2re_b.append(n2b)
        print("%-22s %5d %9.2f %6.2f %9.1f %6.2f %8.2f %6.2f %7.1f %6.2f %5.0f %5.0f"
              % (instance, lb, res["avg"], rea, res["best"], reb,
                 base["avg_E"], n2a, base["best_E"], n2b,
                 res.get("rt_med", -1), base["med_rt"]))
    if re_a:
        n = len(re_a)
        print("-" * 110)
        print("%-22s %5s %9s %6.2f %9s %6.2f %8s %6.2f %7s %6.2f"
              % ("MEAN RE", "", "", sum(re_a) / n, "", sum(re_b) / n,
                 "", sum(n2re_a) / n, "", sum(n2re_b) / n))


if __name__ == "__main__":
    main()
