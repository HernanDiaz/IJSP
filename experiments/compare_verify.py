#!/usr/bin/env python3
"""
compare_verify.py
Parse results from verify_refactor.sh and compare against the baseline
runs_data.csv (statistical_results_exp7/).

Comparison: for each (config, instance, run), checks that lower and upper
bounds match the baseline exactly (tolerance ±0.5 units, accommodating
floating-point rounding in CSV serialisation).

Exit code: 0 if all pass, 1 if any mismatch.
"""

import argparse
import csv
import os
import re
import sys
from collections import defaultdict

TIMESTAMP_RE = re.compile(r'^(.+)_(\d{14})\.csv$')
INTERVAL_RE  = re.compile(r'\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)')

TOLERANCE = 0.5   # units; rounding in CSV serialisation can shift by this much

CONFIG_LABEL = {
    "N1_hc_tuned":   "n1_HC",
    "N2_hc_tuned":   "n2_HC",
    "N3_hc_tuned":   "n3_HC",
    "Next_hc_tuned": "next_HC",
    "N8_hc_tuned":   "n8_HC",
}


def parse_results(results_dir: str, configs: list[str]) -> dict:
    """Return {(config_label, instance, run): (lower, upper)}."""
    data = {}
    for cfg in configs:
        label = CONFIG_LABEL[cfg]
        folder = os.path.join(results_dir, cfg)
        if not os.path.isdir(folder):
            print(f"  WARNING: results folder not found: {folder}")
            continue
        files = [f for f in os.listdir(folder)
                 if f.endswith(".csv") and "_Sols" not in f and "_Robustness" not in f]
        for fname in files:
            m = TIMESTAMP_RE.match(fname)
            if not m:
                continue
            instance = m.group(1)
            with open(os.path.join(folder, fname), encoding="utf-8", errors="replace") as fh:
                lines = fh.readlines()

            obj_idx = next((i for i, l in enumerate(lines)
                            if "Objective function values" in l), None)
            if obj_idx is None:
                print(f"  WARNING: no objective section in {fname}")
                continue

            data_line = None
            for j in range(obj_idx + 1, min(obj_idx + 6, len(lines))):
                if INTERVAL_RE.search(lines[j]):
                    data_line = lines[j].strip()
                    break
            if data_line is None:
                print(f"  WARNING: no interval data in {fname}")
                continue

            parts = data_line.split(";")
            run_vals = parts[4:]
            for run_idx, rv in enumerate(run_vals, start=1):
                mi = INTERVAL_RE.match(rv.strip())
                if mi:
                    data[(label, instance, run_idx)] = (float(mi.group(1)), float(mi.group(2)))
    return data


def load_baseline(baseline_path: str) -> dict:
    """Return {(config_label, instance, run): (lower, upper)}."""
    data = {}
    with open(baseline_path, newline="", encoding="utf-8") as fh:
        for row in csv.DictReader(fh):
            key = (row["config"], row["instance"], int(row["run"]))
            data[key] = (float(row["lower"]), float(row["upper"]))
    return data


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--results-dir", required=True)
    parser.add_argument("--baseline",    required=True)
    parser.add_argument("--configs",     nargs="+", required=True)
    args = parser.parse_args()

    print("Parsing new results...")
    new_data  = parse_results(args.results_dir, args.configs)
    print(f"  {len(new_data)} (config, instance, run) entries found.")

    print("Loading baseline...")
    baseline  = load_baseline(args.baseline)
    print(f"  {len(baseline)} entries in baseline.")

    if not new_data:
        print("\nFAIL: no result data found — did the runs complete?")
        sys.exit(1)

    # Only compare keys that exist in new_data (subset of baseline)
    failures   = []
    missing    = []
    matched    = 0

    for key, (new_lo, new_hi) in sorted(new_data.items()):
        if key not in baseline:
            missing.append(key)
            continue
        base_lo, base_hi = baseline[key]
        lo_ok = abs(new_lo - base_lo) <= TOLERANCE
        hi_ok = abs(new_hi - base_hi) <= TOLERANCE
        if lo_ok and hi_ok:
            matched += 1
        else:
            failures.append((key, (base_lo, base_hi), (new_lo, new_hi)))

    print(f"\n{'='*55}")
    print(f"  Checked  : {matched + len(failures)} entries")
    print(f"  Passed   : {matched}")
    print(f"  Failed   : {len(failures)}")
    if missing:
        print(f"  No baseline entry for: {len(missing)} keys (new instances?)")
    print(f"{'='*55}")

    if failures:
        print("\nMISMATCHES:")
        for (cfg, inst, run), (blo, bhi), (nlo, nhi) in failures[:20]:
            print(f"  {cfg} | {inst} | run {run:2d}"
                  f"  baseline=({blo},{bhi})  new=({nlo},{nhi})"
                  f"  delta=({nlo-blo:+.1f},{nhi-bhi:+.1f})")
        if len(failures) > 20:
            print(f"  ... and {len(failures)-20} more.")
        print("\nRESULT: FAIL")
        sys.exit(1)

    print("\nRESULT: PASS — all runs match the baseline within tolerance.")
    sys.exit(0)


if __name__ == "__main__":
    main()
