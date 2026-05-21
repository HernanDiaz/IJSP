#!/usr/bin/env python3
"""
build_stats_data_exp9.py
Extract per-run makespan and runtime from Exp9 TS-CRISP result CSVs.

Outputs (in statistical_results_exp9/):
  runs_data.csv      -- per-run: neigh, instance, run, makespan, runtime_s
  rpd_per_instance.csv  -- per-instance: best_of_30 RPD vs BKS for each neigh
  rpd_by_group.csv   -- aggregated by instance group x neigh (matches fig7 format)
  rpd_by_config.csv  -- global summary per neigh (for Table tab5)

TS ordering: N2 ≈ N8 >> N_ext >> N1 >> N3
"""

import os
import re
import csv
import statistics

# ── Paths ─────────────────────────────────────────────────────────────────────
RESULTS_BASE = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp9_ts_crisp"
OUT_DIR      = "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/statistical_results_exp9"

CONFIGS = {
    "N1_ts_crisp":   "n1",
    "N2_ts_crisp":   "n2",
    "N3_ts_crisp":   "n3",
    "N8_ts_crisp":   "n8",
    "Next_ts_crisp": "next",
}

# ── BKS table (82 instances) ──────────────────────────────────────────────────
BKS = {
    "F0.15.0.abz7_06": 656,
    "F0.15.0.abz8_05": 665,
    "F0.15.0.abz9_10": 679,
    "F0.15.0.ft10_10": 930,
    "F0.15.0.ft20_05": 1165,
    "F0.15.0.la21_04": 1046,
    "F0.15.0.la24_03": 935,
    "F0.15.0.la25_04": 977,
    "F0.15.0.la27_09": 1235,
    "F0.15.0.la29_03": 1152,
    "F0.15.0.la38_06": 1196,
    "F0.15.0.la40_05": 1222,
    # tai15x15
    "tai15_15_01.F.15_01": 1231, "tai15_15_02.F.15_01": 1244,
    "tai15_15_03.F.15_01": 1218, "tai15_15_04.F.15_01": 1175,
    "tai15_15_05.F.15_01": 1224, "tai15_15_06.F.15_01": 1238,
    "tai15_15_07.F.15_01": 1227, "tai15_15_08.F.15_01": 1217,
    "tai15_15_09.F.15_01": 1274, "tai15_15_10.F.15_01": 1241,
    # tai20x15
    "tai20_15_01.F.15_01": 1357, "tai20_15_02.F.15_01": 1367,
    "tai20_15_03.F.15_01": 1342, "tai20_15_04.F.15_01": 1345,
    "tai20_15_05.F.15_01": 1339, "tai20_15_06.F.15_01": 1360,
    "tai20_15_07.F.15_01": 1462, "tai20_15_08.F.15_01": 1396,
    "tai20_15_09.F.15_01": 1332, "tai20_15_10.F.15_01": 1348,
    # tai20x20
    "tai20_20_01.F.15_01": 1642, "tai20_20_02.F.15_01": 1600,
    "tai20_20_03.F.15_01": 1557, "tai20_20_04.F.15_01": 1644,
    "tai20_20_05.F.15_01": 1595, "tai20_20_06.F.15_01": 1645,
    "tai20_20_07.F.15_01": 1680, "tai20_20_08.F.15_01": 1603,
    "tai20_20_09.F.15_01": 1625, "tai20_20_10.F.15_01": 1584,
    # tai30x15
    "tai30_15_01.F.15_01": 1764, "tai30_15_02.F.15_01": 1784,
    "tai30_15_03.F.15_01": 1791, "tai30_15_04.F.15_01": 1828,
    "tai30_15_05.F.15_01": 2007, "tai30_15_06.F.15_01": 1819,
    "tai30_15_07.F.15_01": 1771, "tai30_15_08.F.15_01": 1673,
    "tai30_15_09.F.15_01": 1795, "tai30_15_10.F.15_01": 1670,
    # tai30x20
    "tai30_20_01.F.15_01": 2006, "tai30_20_02.F.15_01": 1939,
    "tai30_20_03.F.15_01": 1846, "tai30_20_04.F.15_01": 1979,
    "tai30_20_05.F.15_01": 2000, "tai30_20_06.F.15_01": 2006,
    "tai30_20_07.F.15_01": 1889, "tai30_20_08.F.15_01": 1937,
    "tai30_20_09.F.15_01": 1960, "tai30_20_10.F.15_01": 1923,
    # tai50x15
    "tai50_15_01.F.15_01": 2760, "tai50_15_02.F.15_01": 2756,
    "tai50_15_03.F.15_01": 2717, "tai50_15_04.F.15_01": 2839,
    "tai50_15_05.F.15_01": 2679, "tai50_15_06.F.15_01": 2781,
    "tai50_15_07.F.15_01": 2943, "tai50_15_08.F.15_01": 2885,
    "tai50_15_09.F.15_01": 2655, "tai50_15_10.F.15_01": 2723,
    # tai50x20
    "tai50_20_01.F.15_01": 2868, "tai50_20_02.F.15_01": 2869,
    "tai50_20_03.F.15_01": 2755, "tai50_20_04.F.15_01": 2702,
    "tai50_20_05.F.15_01": 2725, "tai50_20_06.F.15_01": 2845,
    "tai50_20_07.F.15_01": 2825, "tai50_20_08.F.15_01": 2784,
    "tai50_20_09.F.15_01": 3071, "tai50_20_10.F.15_01": 2995,
}

# ── Regex patterns ─────────────────────────────────────────────────────────────
TIMESTAMP_RE = re.compile(r'^(.+)_(\d{14})\.csv$')
INTERVAL_RE  = re.compile(r'\(\s*(-?\d+(?:\.\d+)?)\s*,\s*(-?\d+(?:\.\d+)?)\s*\)')
FLOAT_RE     = re.compile(r'-?\d+(?:\.\d+)?')

def instance_to_group(inst):
    """Map instance stem to group name."""
    m = re.match(r'^F0\.15\.0\.(abz7|abz8|abz9|ft10|ft20|la\d+)_\d+$', inst)
    if m:
        return m.group(1)
    m = re.match(r'^(tai\d+_\d+)_\d+\.F\.15_\d+$', inst)
    if m:
        return m.group(1)
    return inst

os.makedirs(OUT_DIR, exist_ok=True)

rows_runs = []
warn = 0

for folder, neigh in CONFIGS.items():
    folder_path = os.path.join(RESULTS_BASE, folder)
    if not os.path.isdir(folder_path):
        print(f"WARNING: folder not found: {folder_path}")
        continue

    all_files = [
        f for f in os.listdir(folder_path)
        if f.endswith(".csv")
        and "_Sols"       not in f
        and "_Robustness" not in f
        and "_Scenarios"  not in f
    ]

    # Keep latest file per instance stem (handles duplicates)
    latest = {}
    for fname in all_files:
        m = TIMESTAMP_RE.match(fname)
        if not m:
            continue
        stem, ts = m.group(1), m.group(2)
        if stem not in latest or ts > latest[stem][0]:
            latest[stem] = (ts, fname)

    n_dup = len(all_files) - len(latest)
    if n_dup > 0:
        print(f"  {folder}: {len(latest)} instances, {n_dup} duplicate(s) skipped")

    for stem, (ts, fname) in sorted(latest.items()):
        if stem not in BKS:
            print(f"WARNING: no BKS for '{stem}' ({folder})")
            warn += 1
            continue

        fpath = os.path.join(folder_path, fname)
        with open(fpath, encoding="utf-8", errors="replace") as fh:
            lines = fh.readlines()

        # ── Objective values ──────────────────────────────────────────────────
        obj_idx = next((i for i, l in enumerate(lines)
                        if "Objective function values" in l), None)
        if obj_idx is None:
            print(f"WARNING: no objective section in {fname}"); warn += 1; continue

        data_line = None
        for j in range(obj_idx + 1, min(obj_idx + 6, len(lines))):
            if INTERVAL_RE.search(lines[j]):
                data_line = lines[j].strip()
                break

        if data_line is None:
            print(f"WARNING: no interval data in {fname}"); warn += 1; continue

        parts    = data_line.split(";")
        run_vals = parts[4:34]
        if len(run_vals) < 30:
            print(f"WARNING: only {len(run_vals)} runs in {fname}"); warn += 1; continue

        makespans = []
        for rv in run_vals:
            rv = rv.strip()
            mi = INTERVAL_RE.match(rv)
            if mi:
                makespan = int(round(float(mi.group(1))))
            else:
                try:
                    makespan = int(round(float(rv)))
                except ValueError:
                    print(f"WARNING: cannot parse makespan '{rv}' in {fname}")
                    makespan = None
            makespans.append(makespan)

        # ── Runtime values ────────────────────────────────────────────────────
        rt_idx = next((i for i, l in enumerate(lines) if "Total runtime" in l), None)
        runtimes = [None] * 30
        if rt_idx is not None:
            rt_line = lines[rt_idx].strip()
            rt_parts = rt_line.split(";")
            # Format: "Total runtime;avg;best;worst;sd;run1;...;run30"
            rt_vals = rt_parts[5:35]
            if len(rt_vals) >= 30:
                runtimes = []
                for rv in rt_vals[:30]:
                    try:
                        runtimes.append(float(rv.strip()))
                    except ValueError:
                        runtimes.append(None)
            else:
                print(f"WARNING: only {len(rt_vals)} runtime values in {fname}")

        # ── Emit rows ─────────────────────────────────────────────────────────
        for run_idx, (ms, rt) in enumerate(zip(makespans, runtimes), start=1):
            if ms is not None:
                rows_runs.append({
                    "neigh":      neigh,
                    "instance":   stem,
                    "group":      instance_to_group(stem),
                    "run":        run_idx,
                    "makespan":   ms,
                    "bks":        BKS[stem],
                    "rpd":        round((ms - BKS[stem]) / BKS[stem] * 100, 6),
                    "runtime_s":  round(rt, 4) if rt is not None else "",
                })

# ── Write runs_data.csv ───────────────────────────────────────────────────────
out_runs = os.path.join(OUT_DIR, "runs_data.csv")
fields = ["neigh", "instance", "group", "run", "makespan", "bks", "rpd", "runtime_s"]
with open(out_runs, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=fields)
    w.writeheader()
    w.writerows(rows_runs)
print(f"Wrote {len(rows_runs)} rows → {out_runs}")

# ── rpd_per_instance.csv  (best_of_30 per neigh×instance) ────────────────────
from collections import defaultdict

# key: (neigh, instance) → list of makespans
inst_makespans = defaultdict(list)
inst_runtimes  = defaultdict(list)
for row in rows_runs:
    k = (row["neigh"], row["instance"])
    inst_makespans[k].append(row["makespan"])
    if row["runtime_s"] != "":
        inst_runtimes[k].append(row["runtime_s"])

# Collect all instances
all_insts  = sorted(BKS.keys())
all_neighs = ["n1", "n2", "n3", "n8", "next"]

pi_rows = []
for inst in all_insts:
    base_row = {"instance": inst, "group": instance_to_group(inst), "bks": BKS[inst]}
    for n in all_neighs:
        k  = (n, inst)
        ms = inst_makespans.get(k, [])
        if ms:
            best = min(ms)
            base_row[f"best_{n}"]   = best
            base_row[f"rpd_{n}"]    = round((best - BKS[inst]) / BKS[inst] * 100, 3)
            base_row[f"med_rt_{n}"] = round(statistics.median(inst_runtimes[k]), 3) if inst_runtimes[k] else ""
        else:
            base_row[f"best_{n}"] = ""
            base_row[f"rpd_{n}"]  = ""
            base_row[f"med_rt_{n}"] = ""
    pi_rows.append(base_row)

out_pi = os.path.join(OUT_DIR, "rpd_per_instance.csv")
pi_fields = (["instance", "group", "bks"] +
             [f"{x}_{n}" for n in all_neighs for x in ("best", "rpd", "med_rt")])
with open(out_pi, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=pi_fields)
    w.writeheader()
    w.writerows(pi_rows)
print(f"Wrote {len(pi_rows)} rows → {out_pi}")

# ── rpd_by_group.csv  (matches figure 7 format from exp8) ────────────────────
group_neigh_rpds = defaultdict(list)
group_neigh_rts  = defaultdict(list)
for row in rows_runs:
    k = (row["group"], row["neigh"])
    group_neigh_rpds[k].append(row["rpd"])
    if row["runtime_s"] != "":
        group_neigh_rts[k].append(row["runtime_s"])

all_groups = sorted({instance_to_group(i) for i in BKS})
grp_rows = []
for g in all_groups:
    n_inst = sum(1 for i in BKS if instance_to_group(i) == g)
    for n in all_neighs:
        k    = (g, n)
        rpds = group_neigh_rpds.get(k, [])
        rts  = group_neigh_rts.get(k, [])
        if not rpds:
            continue
        # Use best_of_30 RPD per instance, then average across instances
        # For a group with n_inst instances, each contributing 30 runs:
        # 'rpd_mean' = mean of per-run RPDs (matches exp8 convention)
        n_opt = sum(1 for r in rpds if r == 0.0)
        grp_rows.append({
            "group":     g,
            "neigh":     n,
            "n_inst":    n_inst,
            "rpd_mean":  round(statistics.mean(rpds),   3),
            "rpd_median":round(statistics.median(rpds), 3),
            "rpd_max":   round(max(rpds),               3),
            "n_optimal": n_opt,
            "med_rt":    round(statistics.median(rts),  3) if rts else "",
        })

out_grp = os.path.join(OUT_DIR, "rpd_by_group.csv")
with open(out_grp, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["group","neigh","n_inst","rpd_mean",
                                        "rpd_median","rpd_max","n_optimal","med_rt"])
    w.writeheader()
    w.writerows(grp_rows)
print(f"Wrote {len(grp_rows)} rows → {out_grp}")

# ── rpd_by_config.csv  (global summary for Table tab5) ───────────────────────
cfg_rows = []
for n in all_neighs:
    all_rpds = [row["rpd"] for row in rows_runs if row["neigh"] == n]
    all_rts  = [row["runtime_s"] for row in rows_runs
                if row["neigh"] == n and row["runtime_s"] != ""]
    n_inst   = len({row["instance"] for row in rows_runs if row["neigh"] == n})
    if not all_rpds:
        continue
    cfg_rows.append({
        "neigh":      n,
        "n_instances": n_inst,
        "rpd_mean":   round(statistics.mean(all_rpds),   3),
        "rpd_median": round(statistics.median(all_rpds), 3),
        "rpd_sd":     round(statistics.stdev(all_rpds),  3),
        "rpd_max":    round(max(all_rpds),               3),
        "n_optimal":  sum(1 for r in all_rpds if r == 0.0),
        "med_rt_s":   round(statistics.median(all_rts),  3) if all_rts else "",
    })

out_cfg = os.path.join(OUT_DIR, "rpd_by_config.csv")
with open(out_cfg, "w", newline="") as fh:
    w = csv.DictWriter(fh, fieldnames=["neigh","n_instances","rpd_mean","rpd_median",
                                        "rpd_sd","rpd_max","n_optimal","med_rt_s"])
    w.writeheader()
    w.writerows(cfg_rows)
print(f"Wrote {len(cfg_rows)} rows → {out_cfg}")

print(f"\n=== Summary ===")
print(f"Total rows: {len(rows_runs)}  |  Warnings: {warn}")
print(f"\nTable tab5 preview (rpd_by_config):")
print(f"{'Neigh':8s}  {'MeanRPD':>8s}  {'MedRPD':>8s}  {'SD':>6s}  {'MaxRPD':>8s}  {'MedRT(s)':>10s}")
for r in sorted(cfg_rows, key=lambda x: x['rpd_mean']):
    print(f"{r['neigh']:8s}  {r['rpd_mean']:8.3f}  {r['rpd_median']:8.3f}  "
          f"{r['rpd_sd']:6.3f}  {r['rpd_max']:8.3f}  {str(r['med_rt_s']):>10s}")
