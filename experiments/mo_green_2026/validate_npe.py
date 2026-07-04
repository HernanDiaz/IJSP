#!/usr/bin/env python3
"""
validate_npe.py — cross-check the C++ NPE (EvaluationIJSP_Energy) against an
independent Python recomputation, using the task order stored in a _Sols file.

Usage: python3 validate_npe.py <sols_csv> <instance_stem> <expected_lo> <expected_hi>
"""
import os, random, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance, POWER_SEED


def decode_npe(order, n, m, mach, dur, pp):
    job_end = [(0, 0)] * n
    mac_end = [(0, 0)] * m
    gaps = [[0.0, 0.0] for _ in range(m)]
    started = [False] * m
    for tid in order:
        j, pos = divmod(tid, m)
        k = mach[j][pos]
        p = dur[j][pos]
        s = (max(job_end[j][0], mac_end[k][0]), max(job_end[j][1], mac_end[k][1]))
        if started[k]:
            gaps[k][0] += s[0] - mac_end[k][0]
            gaps[k][1] += s[1] - mac_end[k][1]
        started[k] = True
        e = (s[0] + p[0], s[1] + p[1])
        job_end[j] = e
        mac_end[k] = e
    lo = sum(pp[k] * gaps[k][0] for k in range(m))
    hi = sum(pp[k] * gaps[k][1] for k in range(m))
    cmax = (max(e[0] for e in job_end), max(e[1] for e in job_end))
    return cmax, (lo, hi)


def main():
    sols, stem, exp_lo, exp_hi = sys.argv[1], sys.argv[2], float(sys.argv[3]), float(sys.argv[4])
    n, m, mach, dur = parse_instance(stem)
    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]
    with open(sols, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            parts = line.strip().split(";")
            if len(parts) >= 2 and parts[0] == "1":
                order = [int(x) for x in parts[1].split()]
                break
    cmax, npe = decode_npe(order, n, m, mach, dur, pp)
    ok = abs(npe[0] - exp_lo) < 1e-6 and abs(npe[1] - exp_hi) < 1e-6
    print(f"python: Cmax=({cmax[0]},{cmax[1]})  NPE=({npe[0]:.0f},{npe[1]:.0f})  "
          f"expected NPE=({exp_lo:.0f},{exp_hi:.0f})  ->  {'MATCH' if ok else 'MISMATCH'}")
    sys.exit(0 if ok else 1)


if __name__ == "__main__":
    main()
