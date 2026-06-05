#!/usr/bin/env python3
"""
scan_n8_bug.py
Independent scan of every run in selected Sols.csv files for TA54
(crisp and interval, N_2 and N_8), comparing the standard serial-SGS
makespan against the value FuzzyFW writes in the "Objective Value"
column. Any mismatch is flagged.

Imports the parser and decoder from verify_ta54_2792.py.
"""

import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from verify_ta54_2792 import parse_instance, serial_sgs, insertion_sgs, ROOT


CRISP_INSTANCE = ROOT / "SelectosYTaillardCrisp" / "tai50_15_04.F.15_01.txt"
INT_INSTANCE   = ROOT / "SelectosYTaillardIntervalos" / "tai50_15_04.F.15_01.txt"


def parse_all_runs(sols_path):
    """Yield (run_index, permutation, objective_string) for every row of a
    Sols.csv (skipping the header)."""
    with sols_path.open(encoding="utf-8") as fh:
        for raw in fh:
            parts = raw.rstrip("\n").split(";")
            if len(parts) < 3:
                continue
            try:
                run = int(parts[0])
            except ValueError:
                continue
            perm = list(map(int, parts[1].split()))
            obj = parts[2].strip()
            yield run, perm, obj


def parse_objective_low_high(obj_str):
    """Convert '(low, high)' into (low, high) ints."""
    s = obj_str.strip().lstrip("(").rstrip(")")
    a, b = s.split(",")
    return int(a.strip()), int(b.strip())


def scan(sols_path, n_jobs, n_machines, machines, durations, label):
    print(f"=== {label} ===")
    print(f"file: {sols_path.relative_to(ROOT)}")
    print(f"{'run':>4} {'fw_low':>8} {'fw_high':>8} {'serial':>8} {'insert':>8} {'verdict':>10}")
    mismatches = 0
    fw_below_bks = 0
    sgs_below_bks = 0
    total = 0
    for run, perm, obj in parse_all_runs(sols_path):
        try:
            cmax_serial, _ = serial_sgs(perm, n_jobs, n_machines, machines, durations)
            cmax_insert = insertion_sgs(perm, n_jobs, n_machines, machines, durations)
        except Exception as e:
            print(f"{run:>4}    PARSE-ERROR: {e}")
            continue
        fw_low, fw_high = parse_objective_low_high(obj)
        total += 1
        # FuzzyFW writes (low, high) which in crisp should equal each other and
        # equal the cmax. In interval mode they're the bounds on the makespan.
        if cmax_serial != cmax_insert:
            verdict = "sgs-diff"
        elif fw_low == cmax_serial == fw_high:
            verdict = "OK"
        elif fw_low == fw_high and fw_low < cmax_serial:
            verdict = "FW<SGS"  # FuzzyFW reports lower than what SGS can decode
            mismatches += 1
        elif fw_low < cmax_serial and fw_high >= cmax_serial:
            # Interval mode: low end below SGS computed value
            verdict = "FW_low<SGS"
            mismatches += 1
        else:
            verdict = "?"
        print(f"{run:>4} {fw_low:>8} {fw_high:>8} {cmax_serial:>8} {cmax_insert:>8} {verdict:>10}")
        if fw_low < 2839:
            fw_below_bks += 1
        if cmax_serial < 2839:
            sgs_below_bks += 1

    print(f"  total runs: {total}")
    print(f"  FW-reported low < 2839 (BKS): {fw_below_bks}")
    print(f"  serial-SGS    < 2839 (BKS): {sgs_below_bks}")
    print(f"  mismatches (FW reports < SGS-verified): {mismatches}")
    print()


def find_latest_sols(directory: Path, stem: str):
    """Return the latest <stem>_<timestamp>_Sols.csv in `directory`."""
    candidates = sorted(directory.glob(f"{stem}_*_Sols.csv"))
    return candidates[-1] if candidates else None


def main():
    n_jobs, n_machines, mach_c, dur_c = parse_instance(CRISP_INSTANCE)
    n_jobs2, n_machines2, mach_i, dur_i = parse_instance(INT_INSTANCE)

    base = ROOT / "experiments" / "results"
    stem = "tai50_15_04.F.15_01"

    # --- CRISP, all 5 neighbourhoods ------------------------------------
    crisp_dirs = {
        "N_1": "N1_ts_crisp",
        "N_2": "N2_ts_crisp",
        "N_3": "N3_ts_crisp",
        "N_8": "N8_ts_crisp",
        "N_ext": "Next_ts_crisp",
    }
    for label, dirname in crisp_dirs.items():
        path = find_latest_sols(base / "exp9_ts_crisp" / dirname, stem)
        if path is None:
            print(f"=== {label} CRISP, TA54 ===  NO Sols.csv FOUND in {dirname}/")
            print()
            continue
        scan(path, n_jobs, n_machines, mach_c, dur_c, f"{label} CRISP, TA54")


if __name__ == "__main__":
    main()
