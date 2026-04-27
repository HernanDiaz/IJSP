#!/usr/bin/env python3
"""
build_stats_data_exp7.py
Extract per-run makespan interval data from Exp7 HC result CSVs.
Output: statistical_results_exp7/runs_data.csv

Format: config,neigh,comp,instance,run,lower,upper,midpoint
"""

import os
import re
import csv

RESULTS_BASE = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp7"
OUT_DIR      = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/statistical_results_exp7"

CONFIGS = {
    "N1_hc_tuned":   ("n1",   "HC"),
    "N2_hc_tuned":   ("n2",   "HC"),
    "N3_hc_tuned":   ("n3",   "HC"),
    "Next_hc_tuned": ("next", "HC"),
    "N8_hc_tuned":   ("n8",   "HC"),
}

TIMESTAMP_RE = re.compile(r'^(.+)_(\d{14})\.csv$')
INTERVAL_RE  = re.compile(r'\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)')

os.makedirs(OUT_DIR, exist_ok=True)

rows = []
warn = 0

for folder, (neigh, comp) in CONFIGS.items():
    config      = f"{neigh}_HC"
    folder_path = os.path.join(RESULTS_BASE, folder)
    if not os.path.isdir(folder_path):
        print(f"WARNING: folder not found: {folder_path}")
        continue

    files = sorted(f for f in os.listdir(folder_path)
                   if f.endswith(".csv")
                   and "_Sols" not in f
                   and "_Robustness" not in f)

    for fname in files:
        m = TIMESTAMP_RE.match(fname)
        if not m:
            continue
        instance = m.group(1)
        fpath    = os.path.join(folder_path, fname)

        with open(fpath, encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()

        # Find "Objective function values" section
        obj_idx = next((i for i, l in enumerate(lines)
                        if "Objective function values" in l), None)
        if obj_idx is None:
            print(f"WARNING: no objective section in {fname}"); warn += 1; continue

        # Data line: first line after the section header that contains interval values
        data_line = None
        for j in range(obj_idx + 1, min(obj_idx + 6, len(lines))):
            if INTERVAL_RE.search(lines[j]):
                data_line = lines[j].strip()
                break

        if data_line is None:
            print(f"WARNING: no interval data in {fname}"); warn += 1; continue

        # Fields: Average;Best;Worst;Std.Dev.;Run1;Run2;...;Run30
        parts     = data_line.split(";")
        run_vals  = parts[4:34]   # exactly 30 runs

        if len(run_vals) < 30:
            print(f"WARNING: only {len(run_vals)} runs in {fname}"); warn += 1; continue

        for run_idx, rv in enumerate(run_vals, start=1):
            mi = INTERVAL_RE.match(rv.strip())
            if mi:
                lower = float(mi.group(1))
                upper = float(mi.group(2))
            else:
                try:
                    v = float(rv.strip()); lower = upper = v
                except ValueError:
                    print(f"WARNING: cannot parse '{rv}' in {fname}"); warn += 1; continue
            rows.append({
                "config":   config,
                "neigh":    neigh,
                "comp":     comp,
                "instance": instance,
                "run":      run_idx,
                "lower":    lower,
                "upper":    upper,
                "midpoint": (lower + upper) / 2.0,
            })

out_file = os.path.join(OUT_DIR, "runs_data.csv")
with open(out_file, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["config","neigh","comp","instance","run",
                                        "lower","upper","midpoint"])
    w.writeheader()
    w.writerows(rows)

n_inst    = len({(r["config"], r["instance"]) for r in rows})
n_configs = len({r["config"] for r in rows})
print(f"Done. {len(rows)} rows | {n_configs} configs | {n_inst // n_configs} instances | {warn} warnings")
print(f"Output: {out_file}")
