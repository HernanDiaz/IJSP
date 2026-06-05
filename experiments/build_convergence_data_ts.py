#!/usr/bin/env python3
"""
build_convergence_data_ts.py
Extract per-generation convergence profiles from Phase B TS exp4 CSV files.

Output: statistical_results_exp4/convergence_data_ts.csv
Format: neigh, step, mean_best_norm, sd_best_norm, n_obs
  - step: rounded generation number (integer)
  - best_norm: best midpoint at step / best midpoint at step 0, per run
  - aggregated across all instances and 30 runs
"""

import os
import re
import csv
import statistics
from collections import defaultdict

RESULTS_BASE = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp4"
OUT_DIR      = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/statistical_results_exp4"

CONFIGS = {
    "N1_tuned":   "n1",
    "N2_tuned":   "n2",
    "N3_tuned":   "n3",
    "N8_tuned":   "n8",
    "Next_tuned": "next",
}

TIMESTAMP_RE = re.compile(r'^(.+)_(\d{14})\.csv$')
N_RUNS = 30   # runs per file

os.makedirs(OUT_DIR, exist_ok=True)

# 82-instance allow-list = intersection of stems across all 5 NB folders
def _stems_in(folder):
    if not os.path.isdir(folder): return set()
    out = set()
    for fname in os.listdir(folder):
        m = TIMESTAMP_RE.match(fname)
        if m and "Sols" not in fname and "Robustness" not in fname and "Scenarios" not in fname:
            out.add(m.group(1))
    return out
ALLOW_82 = None
for folder in CONFIGS:
    s = _stems_in(os.path.join(RESULTS_BASE, folder))
    ALLOW_82 = s if ALLOW_82 is None else (ALLOW_82 & s)
print(f"Allow-list (intersection across 5 NBs): {len(ALLOW_82)} stems")

# key: (neigh, step_int) → list of best_norm values (one per run×instance)
step_data = defaultdict(list)

warn = 0
for folder, neigh in CONFIGS.items():
    folder_path = os.path.join(RESULTS_BASE, folder)
    if not os.path.isdir(folder_path):
        print(f"WARNING: folder not found: {folder_path}")
        continue

    files = [f for f in os.listdir(folder_path)
             if f.endswith(".csv")
             and "_Sols" not in f and "_Robustness" not in f
             and "_Scenarios" not in f]

    # Keep only latest timestamp per instance stem; restrict to 82 common
    latest = {}
    for fname in files:
        m = TIMESTAMP_RE.match(fname)
        if not m:
            continue
        stem, ts = m.group(1), m.group(2)
        if stem not in ALLOW_82:
            continue
        if stem not in latest or ts > latest[stem][0]:
            latest[stem] = (ts, fname)

    n_files = 0
    for stem, (ts, fname) in sorted(latest.items()):
        fpath = os.path.join(folder_path, fname)
        with open(fpath, encoding="utf-8", errors="replace") as fh:
            raw_lines = fh.readlines()

        # Find the convergence section header: line starting with "Step;"
        header_idx = next((i for i, l in enumerate(raw_lines)
                           if l.strip().startswith("Step;")), None)
        if header_idx is None:
            print(f"WARNING: no Step; header in {fname}"); warn += 1; continue

        # Data rows follow the header (lines starting with a digit)
        data_rows = []
        for line in raw_lines[header_idx + 1:]:
            line = line.strip()
            if not line or not line[0].isdigit():
                break
            data_rows.append(line.split(";"))

        if not data_rows:
            print(f"WARNING: no convergence rows in {fname}"); warn += 1; continue

        # Parse: for run k (0-indexed), best midpoint is at column 6*k + 2
        # (run 0: col 2; run 1: col 8; run 2: col 14; ...)
        # Column 6*k is the step/generation for that run; col 6*k+2 is best midpoint.

        # Collect (step_int, best_norm) for each run
        run_series = defaultdict(list)   # run_idx → [(step_int, best)]

        for row in data_rows:
            for k in range(N_RUNS):
                step_col = 6 * k
                best_col = 6 * k + 2
                if step_col >= len(row) or best_col >= len(row):
                    break
                try:
                    step_val = float(row[step_col])
                    best_val = float(row[best_col])
                except ValueError:
                    continue
                step_int = int(round(step_val))
                run_series[k].append((step_int, best_val))

        # Normalize each run by its step-0 value
        for k, series in run_series.items():
            if not series:
                continue
            # Sort by step
            series.sort(key=lambda x: x[0])
            # Get initial best (step 0)
            init_vals = [v for s, v in series if s == 0]
            if not init_vals:
                init = series[0][1]
            else:
                init = init_vals[0]
            if init <= 0:
                continue
            # Emit normalized values; where step repeats, use min best_norm
            by_step = defaultdict(list)
            for s, v in series:
                by_step[s].append(v / init)
            for step_int, norms in by_step.items():
                step_data[(neigh, step_int)].extend(norms)

        n_files += 1

    print(f"  {folder}: {n_files} instances processed")

# ── Write output ──────────────────────────────────────────────────────────────
out_rows = []
for (neigh, step), vals in sorted(step_data.items()):
    if len(vals) < 2:
        continue
    out_rows.append({
        "neigh":          neigh,
        "step":           step,
        "mean_best_norm": round(statistics.mean(vals),   6),
        "sd_best_norm":   round(statistics.stdev(vals),  6),
        "n_obs":          len(vals),
    })

out_file = os.path.join(OUT_DIR, "convergence_data_ts.csv")
with open(out_file, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["neigh","step","mean_best_norm","sd_best_norm","n_obs"])
    w.writeheader()
    w.writerows(out_rows)

print(f"\nDone. {len(out_rows)} rows → {out_file}  |  {warn} warnings")

# ── Quick sanity check ────────────────────────────────────────────────────────
print("\nSanity check — step 0 mean (should be ~1.0 for all neighs):")
for (neigh, step), vals in sorted(step_data.items()):
    if step == 0:
        print(f"  {neigh:6s} step 0: mean={statistics.mean(vals):.4f}  n={len(vals)}")

print("\nFinal step per neigh (last step with ≥ 50% of max observations):")
max_obs = max(len(v) for v in step_data.values())
for neigh in ["n1","n2","n3","n8","next"]:
    steps = [(s, step_data[(neigh,s)]) for (n,s) in step_data if n==neigh]
    if steps:
        last = max(s for s,_ in steps if len(step_data[(neigh,s)]) >= max_obs*0.1)
        print(f"  {neigh:6s}: last step = {last}")
