#!/usr/bin/env python3
"""
build_phaseB_stats.py
Extract per-instance per-run final best E[Cmax] and runtime from Phase B TS result CSVs.

Reads the "Best solution" row from the "Basic statistics" section and the
"Total runtime" row from the "Runtimes" section — no need to parse the evolution table.

Output: cor_tabu_2026/statistical_results/per_instance_stats.csv
Format: neigh, instance, group, n_runs, best_E, avg_E, sd_E, med_rt
"""

import os
import re
import csv
import statistics
from collections import defaultdict

RESULTS_BASE = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp4"
OUT_DIR      = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/cor_tabu_2026/statistical_results"

CONFIGS = {
    "N1_tuned":   "n1",
    "N2_tuned":   "n2",
    "N3_tuned":   "n3",
    "N8_tuned":   "n8",
    "Next_tuned": "next",
}

TIMESTAMP_RE = re.compile(r'^(.+)_(\d{14})\.csv$')
N_RUNS = 30

# Two naming conventions:
#   Classical:  F0.15.0.abz7_06      -> group = abz7
#   Taillard:   tai15_15_01.F.15_01  -> group = tai15_15
TAI_RE     = re.compile(r'^(tai\d+_\d+)_\d+')
CLASSIC_RE = re.compile(r'\.(abz\d+|ft\d+|la\d+)_')

def extract_group(stem):
    m = TAI_RE.match(stem)
    if m:
        return m.group(1)
    m = CLASSIC_RE.search(stem)
    if m:
        return m.group(1)
    return None

os.makedirs(OUT_DIR, exist_ok=True)

# Build the 82-instance allow-list: the intersection of stems present in all 5
# tuned neighbourhood folders. This matches the paper's Phase B scope of 82
# common benchmark instances (tai100_20 excludes itself because N_3 and N_8
# were not run on it). Drop this filter to include all instances.
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

# Regex to parse interval objective written as "(low, high)" in Sols.csv
INTERVAL_RE = re.compile(r'\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)')

records = []
runs_records = []      # one row per (neigh, instance, run) for runs_data.csv
warn = 0

for folder, neigh in CONFIGS.items():
    folder_path = os.path.join(RESULTS_BASE, folder)
    if not os.path.isdir(folder_path):
        print(f"WARNING: folder not found: {folder_path}"); warn += 1; continue

    files = [f for f in os.listdir(folder_path)
             if f.endswith(".csv")
             and "_Sols" not in f and "_Robustness" not in f
             and "_Scenarios" not in f]

    # Keep only latest timestamp per instance stem
    latest = {}
    for fname in files:
        m = TIMESTAMP_RE.match(fname)
        if not m:
            continue
        stem, ts = m.group(1), m.group(2)
        if stem not in latest or ts > latest[stem][0]:
            latest[stem] = (ts, fname)

    n_files = 0
    for stem, (ts, fname) in sorted(latest.items()):
        if stem not in ALLOW_82:
            continue
        group = extract_group(stem)
        if group is None:
            print(f"WARNING: cannot extract group from '{stem}'"); warn += 1; continue

        fpath = os.path.join(folder_path, fname)
        with open(fpath, encoding="utf-8", errors="replace") as fh:
            raw_lines = fh.readlines()

        # ── Find "Total runtime" row (Runtimes section) ────────────────────────
        rt_row = None
        for line in raw_lines:
            s = line.strip()
            if s.startswith("Total runtime;"):
                rt_row = s.split(";")
                break
        if rt_row is None:
            print(f"WARNING: no 'Total runtime' row in {fname}"); warn += 1; continue

        # ── Find "Best solution" row (Basic statistics section) ────────────────
        best_row = None
        for line in raw_lines:
            s = line.strip()
            if s.startswith("Best solution;"):
                best_row = s.split(";")
                break
        if best_row is None:
            print(f"WARNING: no 'Best solution' row in {fname}"); warn += 1; continue

        # Both rows: Field ; Avg ; Best ; Worst ; SD ; Run1 ; Run2 ; ... ; Run30
        # Per-run values start at index 5 (0-based)
        try:
            run_E  = [float(v) for v in best_row[5:5 + N_RUNS]]
            run_rt = [float(v) for v in rt_row[5:5 + N_RUNS]]
        except ValueError as e:
            print(f"WARNING: parse error in {fname}: {e}"); warn += 1; continue

        if len(run_E) < 2:
            print(f"WARNING: only {len(run_E)} runs in {fname}"); warn += 1; continue

        records.append({
            "neigh":    neigh,
            "instance": stem,
            "group":    group,
            "n_runs":   len(run_E),
            "best_E":   round(min(run_E),   4),
            "avg_E":    round(statistics.mean(run_E),   4),
            "sd_E":     round(statistics.stdev(run_E),  4),
            "med_rt":   round(statistics.median(run_rt), 3),
        })

        # ── Also collect granular per-run intervals from Sols.csv ─────────────
        sols_path = os.path.join(folder_path, fname.replace(".csv", "_Sols.csv"))
        if os.path.isfile(sols_path):
            with open(sols_path, encoding="utf-8", errors="replace") as fh:
                next(fh, None)
                for line in fh:
                    parts = line.rstrip("\n").split(";")
                    if len(parts) < 3 or not parts[0].isdigit():
                        continue
                    try:
                        run_idx = int(parts[0])
                        m = INTERVAL_RE.match(parts[2].strip())
                        if m:
                            lo, hi = float(m.group(1)), float(m.group(2))
                        else:
                            lo = hi = float(parts[2].strip())
                    except ValueError:
                        continue
                    runs_records.append({
                        "config":   f"{neigh}_TS",
                        "neigh":    neigh,
                        "comp":     "TS",
                        "instance": stem,
                        "run":      run_idx,
                        "lower":    lo,
                        "upper":    hi,
                        "midpoint": (lo + hi) / 2.0,
                    })
        n_files += 1

    print(f"  {folder}: {n_files} instances processed")

# ── Write output ──────────────────────────────────────────────────────────────
out_file = os.path.join(OUT_DIR, "per_instance_stats.csv")
with open(out_file, "w", newline="") as fh:
    w = csv.DictWriter(fh,
        fieldnames=["neigh","instance","group","n_runs","best_E","avg_E","sd_E","med_rt"])
    w.writeheader()
    w.writerows(records)

print(f"\nDone. {len(records)} rows → {out_file}  |  {warn} warnings")

# ── Also write the granular runs_data.csv (one row per (NB, instance, run)) ──
runs_file = os.path.join(OUT_DIR, "runs_data.csv")
with open(runs_file, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["config","neigh","comp","instance","run",
                                       "lower","upper","midpoint"])
    w.writeheader()
    w.writerows(runs_records)
print(f"  + {len(runs_records)} runs → {runs_file}")

# ── Sanity check ──────────────────────────────────────────────────────────────
print("\nSanity check — abz7 per neigh (LB=656, expect avg_E > 656):")
by_neigh = defaultdict(list)
for r in records:
    by_neigh[r['neigh']].append(r)

for neigh in ['n1','n2','n3','n8','next']:
    abz7 = [r for r in by_neigh[neigh] if r['group'] == 'abz7']
    if abz7:
        r = abz7[0]
        re_pct = (r['avg_E'] - 656) / 656 * 100
        print(f"  {neigh}: best_E={r['best_E']:.2f}  avg_E={r['avg_E']:.2f}  RE%={re_pct:.2f}  t={r['med_rt']:.1f}s")

print("\nGrand mean avg_E by neigh (across all instances):")
for neigh in ['n1','n2','n3','n8','next']:
    vals = [r['avg_E'] for r in by_neigh[neigh]]
    if vals:
        print(f"  {neigh}: {len(vals)} instances, grand_mean={statistics.mean(vals):.2f}")
