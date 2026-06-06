#!/usr/bin/env python3
"""validate_phaseA.py - Quick SGS check on Phase A (5x4 HC operator
comparison) for the bug-prone neighbourhoods N_3 and N_8 across all
four ranking operators (EV, LEX1, LEX2, YX).

Reports the fraction of (instance, run) pairs where the FuzzyFW-reported
makespan interval is strictly smaller than the SGS-decoded one, which is
the signature of the BFS-seeding bug fixed in NeighbourhoodIJSP_Cmax.cpp.
"""
import os, re, statistics
from pathlib import Path
from collections import defaultdict

_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_WSL if os.path.isdir(_WSL) else _WIN)
INSTANCES_DIR = ROOT / "SelectosYTaillardIntervalos"
RESULTS_BASE  = ROOT / "experiments" / "results"

TARGETS = []
for nb_prefix, label in [("n3", "N_3"), ("n8_HC", "N_8")]:
    for op in ["EV", "LEX1", "LEX2", "YX"]:
        TARGETS.append((f"{label} / {op}", RESULTS_BASE / f"{nb_prefix}_{op}"))

TS_RE = re.compile(r'^(.+)_(\d{14})_Sols\.csv$')


def parse_interval(path: Path):
    raw = path.read_text(encoding="utf-8").splitlines()
    def to_ints(s):
        return [int(x) for x in s.replace("\t", " ").split() if x]
    sec_idx = next((i for i, l in enumerate(raw) if "SECUENCIA" in l.upper()), None)
    dur_idx = next((i for i, l in enumerate(raw) if "DURACIONES" in l.upper()), None)
    if sec_idx is not None and dur_idx is not None:
        n_jobs = int(raw[1].strip()); n_machines = int(raw[3].strip())
        machines = [to_ints(raw[sec_idx + 1 + j]) for j in range(n_jobs)]
        pair_re = re.compile(r"\(\s*(\d+)\s*,\s*(\d+)\s*\)")
        dur_low, dur_high = [], []
        for j in range(n_jobs):
            line = raw[dur_idx + 1 + j]; pairs = pair_re.findall(line)
            dur_low.append([int(l) for l, _ in pairs])
            dur_high.append([int(h) for _, h in pairs])
        return n_jobs, n_machines, machines, dur_low, dur_high
    lines = [l for l in raw if l.strip() != ""]
    n_jobs = int(lines[0]); n_machines = int(lines[1])
    machines = [to_ints(lines[2 + j]) for j in range(n_jobs)]
    pair_re = re.compile(r"\(\s*(\d+)\s*,\s*(\d+)\s*\)")
    dur_low, dur_high = [], []
    for j in range(n_jobs):
        line = lines[2 + n_jobs + j]; pairs = pair_re.findall(line)
        dur_low.append([int(l) for l, _ in pairs])
        dur_high.append([int(h) for _, h in pairs])
    return n_jobs, n_machines, machines, dur_low, dur_high


def serial_sgs(perm, n_jobs, n_machines, machines, durations):
    jnf = [0]*n_jobs; jno = [0]*n_jobs; mnf = [0]*n_machines
    for op_id in perm:
        j, o = divmod(op_id, n_machines)
        if o != jno[j]:
            raise RuntimeError("precedence violation")
        m = machines[j][o]; dur = durations[j][o]
        s = max(jnf[j], mnf[m]); e = s + dur
        jnf[j] = e; jno[j] += 1; mnf[m] = e
    return max(jnf)


def latest_sols(folder: Path):
    out = {}
    if not folder.exists(): return out
    for fname in os.listdir(folder):
        if not fname.endswith("_Sols.csv"): continue
        m = TS_RE.match(fname)
        if not m: continue
        stem, ts = m.group(1), m.group(2)
        if stem not in out or ts > out[stem][0]:
            out[stem] = (ts, fname)
    return {s: folder / f for s, (t, f) in out.items()}


def validate_dir(label, folder: Path):
    sols = latest_sols(folder)
    runs = ok = bug = 0
    gaps = []
    first = None
    for stem in sorted(sols):
        inst = INSTANCES_DIR / f"{stem}.txt"
        if not inst.exists(): continue
        try:
            n_jobs, n_mach, mach, dl, dh = parse_interval(inst)
        except Exception:
            continue
        with sols[stem].open(encoding="utf-8", errors="replace") as fh:
            next(fh, None)
            for raw in fh:
                parts = raw.rstrip("\n").split(";")
                if len(parts) < 3 or not parts[0].isdigit(): continue
                try:
                    perm = list(map(int, parts[1].split()))
                    obj = parts[2].strip().lstrip("(").rstrip(")")
                    fw_low, fw_high = (int(x) for x in obj.split(","))
                except Exception:
                    continue
                try:
                    sgs_low = serial_sgs(perm, n_jobs, n_mach, mach, dl)
                    sgs_high = serial_sgs(perm, n_jobs, n_mach, mach, dh)
                except Exception:
                    continue
                runs += 1
                if fw_low < sgs_low or fw_high < sgs_high:
                    bug += 1
                    g = max(sgs_low - fw_low, sgs_high - fw_high, 0)
                    gaps.append(g)
                    if first is None:
                        first = (stem, int(parts[0]), fw_low, fw_high, sgs_low, sgs_high)
                else:
                    ok += 1
    pct = 100.0 * bug / runs if runs else 0.0
    mean_gap = statistics.mean(gaps) if gaps else 0
    return runs, ok, bug, pct, mean_gap, first


print(f"{'Config':>15} {'runs':>6} {'OK':>6} {'BUG':>6} {'%BUG':>6} {'mean_gap':>9} sample mismatch")
total_runs = total_bug = 0
for label, folder in TARGETS:
    runs, ok, bug, pct, mg, first = validate_dir(label, folder)
    total_runs += runs; total_bug += bug
    sample = ""
    if first:
        stem, r, fl, fh_, sl, sh = first
        sample = f"{stem[:18]} run{r}:FW({fl},{fh_}) SGS({sl},{sh})"
    print(f"{label:>15} {runs:>6} {ok:>6} {bug:>6} {pct:>5.1f}% {mg:>9.1f} {sample}")
print()
print(f"COMBINED: {total_runs} runs, {total_bug} buggy ({100.0*total_bug/total_runs:.2f}%)")
