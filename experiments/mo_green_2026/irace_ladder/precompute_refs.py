#!/usr/bin/env python3
"""
precompute_refs.py — fixed per-instance HV reference points for irace tuning.

For every training instance, decode the LexME arm's 30 stored solutions
(Cmax, NPE midpoints) and set the reference point to
(max Cmax * 1.15, max NPE * 1.25) — a stable, data-derived nadir that
bounds every ladder front (which reaches higher Cmax up to +10% and lower
NPE than LexME). Written to ref_points.txt: "<stem> <cmax_ref> <npe_ref>".
"""
import glob, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, os.path.normpath(os.path.join(HERE, "..")))
from conflict_check import parse_instance, POWER_SEED
from validate_npe import decode_npe
import random

REPO = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
LEX = os.path.join(REPO, "experiments", "mo_green_2026", "results", "LexME")
INST_FILE = os.path.join(HERE, "instances.txt")


def stems():
    out = []
    with open(INST_FILE) as fh:
        for ln in fh:
            ln = ln.strip()
            if ln and not ln.startswith("#"):
                out.append(os.path.basename(ln)[:-4])  # drop .txt
    return out


def main():
    lines = []
    for stem in stems():
        n, m, mach, dur = parse_instance(stem)
        rng = random.Random(POWER_SEED)
        pp = [rng.randint(2, 8) for _ in range(m)]
        cmax_max = npe_max = 0.0
        for sols in glob.glob(os.path.join(LEX, stem + "_*_Sols.csv")):
            with open(sols, encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    p = line.strip().split(";")
                    if len(p) >= 2 and p[0].isdigit():
                        o = [int(x) for x in p[1].split()]
                        if len(o) != n * m:
                            continue
                        c, e = decode_npe(o, n, m, mach, dur, pp)
                        cmax_max = max(cmax_max, (c[0] + c[1]) / 2)
                        npe_max = max(npe_max, (e[0] + e[1]) / 2)
        if cmax_max > 0:
            lines.append(f"{stem} {cmax_max*1.15:.0f} {npe_max*1.25:.0f}")
        else:
            print(f"WARN: no LexME data for {stem}", file=sys.stderr)
    with open(os.path.join(HERE, "ref_points.txt"), "w", newline="\n") as fh:
        fh.write("\n".join(lines) + "\n")
    print(f"wrote {len(lines)} reference points")


if __name__ == "__main__":
    main()
