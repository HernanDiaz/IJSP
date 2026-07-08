#!/usr/bin/env python3
"""
exp4_compare.py — three-arm comparison for one Exp.4 tier.

For each instance, builds each arm's GLOBAL non-dominated front (best over
its 30 runs), then a common reference = non-dominated union of the three
arms (+ exact envelope where available). Reports, per arm, the hypervolume
ratio HV(arm)/HV(reference) w.r.t. a shared reference point (1.05x the
union nadir) — 1.0 means the arm alone matches the combined front. Also
flags the winner per instance and a coverage summary.
"""
import glob, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from front_metrics import load_front, load_exact, hypervolume, nondominated

HERE = os.path.dirname(os.path.abspath(__file__))
BASE = os.path.join(HERE, "results", "EXP4")
ARMS = ["LADDER", "ABC-P", "MA-P"]
STEMS = ["F0.15.0.abz7_06", "F0.15.0.abz8_05", "F0.15.0.abz9_10",
         "F0.15.0.ft10_10", "F0.15.0.ft20_05", "F0.15.0.la21_04",
         "F0.15.0.la24_03", "F0.15.0.la25_04", "F0.15.0.la27_09",
         "F0.15.0.la29_03", "F0.15.0.la38_06", "F0.15.0.la40_05"]


def arm_front(arm, stem):
    files = glob.glob(os.path.join(BASE, arm, stem, "**",
                                   stem + "_*_Front.csv"), recursive=True) + \
            glob.glob(os.path.join(BASE, arm, stem + "_*_Front.csv"))
    pts = []
    for f in files:
        pts += load_front(f)
    return nondominated(pts) if pts else []


def main():
    print(f"{'instance':20} | " + " ".join(f"{a:>8}" for a in ARMS) +
          " | winner")
    print("-" * 62)
    wins = {a: 0 for a in ARMS}
    hv_acc = {a: [] for a in ARMS}
    for stem in STEMS:
        fronts = {a: arm_front(a, stem) for a in ARMS}
        if not any(fronts.values()):
            print(f"{stem:20} | no data")
            continue
        union = [p for a in ARMS for p in fronts[a]]
        ex = load_exact(stem)
        ref_pts = union + (ex or [])
        nad = (max(p[0] for p in ref_pts), max(p[1] for p in ref_pts))
        ref = (nad[0] * 1.05, nad[1] * 1.05)
        refhv = hypervolume(nondominated(ref_pts), ref)
        row, best, bestarm = "", -1, ""
        for a in ARMS:
            if fronts[a] and refhv > 0:
                r = hypervolume(fronts[a], ref) / refhv
                hv_acc[a].append(r)
                row += f"{r:>9.3f}"
                if r > best:
                    best, bestarm = r, a
            else:
                row += f"{'--':>9}"
        wins[bestarm] += 1
        print(f"{stem:20} |{row} | {bestarm}")
    print("-" * 62)
    mean = {a: (sum(hv_acc[a]) / len(hv_acc[a]) if hv_acc[a] else 0)
            for a in ARMS}
    print(f"{'MEAN HV ratio':20} | " +
          " ".join(f"{mean[a]:>8.3f}" for a in ARMS))
    print(f"{'wins':20} | " + " ".join(f"{wins[a]:>8d}" for a in ARMS))


if __name__ == "__main__":
    main()
