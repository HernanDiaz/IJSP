#!/usr/bin/env python3
"""
build_stats_data_exp8.py
Extract per-run makespan data from Exp8 HC result CSVs (CRISP instances).
Output: statistical_results_exp8/runs_data.csv

Format: config,neigh,instance,run,makespan

Notes:
  - Crisp instances have degenerate intervals (a,a); makespan = integer value.
  - tai100 instances are excluded (not part of the 82-instance benchmark set).
  - When an instance has multiple result files (duplicates from pre-kill run),
    only the file with the LATEST timestamp is used.
"""

import os
import re
import csv

RESULTS_BASE = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp8"
OUT_DIR      = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/statistical_results_exp8"

CONFIGS = {
    "N1_hc_crisp":   "n1",
    "N2_hc_crisp":   "n2",
    "N3_hc_crisp":   "n3",
    "Next_hc_crisp": "next",
    "N8_hc_crisp":   "n8",
}

TIMESTAMP_RE = re.compile(r'^(.+)_(\d{14})\.csv$')
INTERVAL_RE  = re.compile(r'\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)')

os.makedirs(OUT_DIR, exist_ok=True)

rows = []
warn = 0
skipped_tai100 = 0

for folder, neigh in CONFIGS.items():
    config      = f"{neigh}_HC"
    folder_path = os.path.join(RESULTS_BASE, folder)
    if not os.path.isdir(folder_path):
        print(f"WARNING: folder not found: {folder_path}")
        continue

    # Collect all main CSV files (skip Sols, Robustness, Scenarios, tai100)
    all_files = [
        f for f in os.listdir(folder_path)
        if f.endswith(".csv")
        and "_Sols"       not in f
        and "_Robustness" not in f
        and "_Scenarios"  not in f
        and "tai100"      not in f
    ]

    # Group by instance stem → keep file with highest timestamp (latest)
    latest = {}  # stem → (timestamp_str, filename)
    for fname in all_files:
        m = TIMESTAMP_RE.match(fname)
        if not m:
            continue
        stem, ts = m.group(1), m.group(2)
        if stem not in latest or ts > latest[stem][0]:
            latest[stem] = (ts, fname)

    if not latest:
        print(f"WARNING: no files found in {folder_path}")
        continue

    n_dup = len(all_files) - len(latest)
    if n_dup > 0:
        print(f"  {folder}: {len(latest)} unique instances, {n_dup} duplicate file(s) skipped")

    for stem, (ts, fname) in sorted(latest.items()):
        fpath = os.path.join(folder_path, fname)

        with open(fpath, encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()

        # Find "Objective function values" section
        obj_idx = next((i for i, l in enumerate(lines)
                        if "Objective function values" in l), None)
        if obj_idx is None:
            print(f"WARNING: no objective section in {fname}"); warn += 1; continue

        # Data line: first line after section header containing interval values
        data_line = None
        for j in range(obj_idx + 1, min(obj_idx + 6, len(lines))):
            if INTERVAL_RE.search(lines[j]):
                data_line = lines[j].strip()
                break

        if data_line is None:
            print(f"WARNING: no interval data in {fname}"); warn += 1; continue

        # Fields: Average;Best;Worst;Std.Dev.;Run1;...;Run30
        parts    = data_line.split(";")
        run_vals = parts[4:34]   # exactly 30 runs

        if len(run_vals) < 30:
            print(f"WARNING: only {len(run_vals)} runs in {fname}"); warn += 1; continue

        for run_idx, rv in enumerate(run_vals, start=1):
            rv = rv.strip()
            mi = INTERVAL_RE.match(rv)
            if mi:
                lower = float(mi.group(1))
                upper = float(mi.group(2))
                if abs(lower - upper) > 0.5:
                    print(f"WARNING: non-crisp interval ({lower},{upper}) in {fname} run {run_idx}")
                makespan = int(round(lower))   # for crisp: lower == upper
            else:
                try:
                    makespan = int(round(float(rv)))
                except ValueError:
                    print(f"WARNING: cannot parse '{rv}' in {fname}"); warn += 1; continue

            rows.append({
                "config":   config,
                "neigh":    neigh,
                "instance": stem,
                "run":      run_idx,
                "makespan": makespan,
            })

out_file = os.path.join(OUT_DIR, "runs_data.csv")
with open(out_file, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["config", "neigh", "instance", "run", "makespan"])
    w.writeheader()
    w.writerows(rows)

n_inst    = len({(r["config"], r["instance"]) for r in rows})
n_configs = len({r["config"] for r in rows})
n_per_cfg = n_inst // n_configs if n_configs else 0
print(f"\nDone. {len(rows)} rows | {n_configs} configs | {n_per_cfg} instances each | {warn} warnings")
print(f"Output: {out_file}")
