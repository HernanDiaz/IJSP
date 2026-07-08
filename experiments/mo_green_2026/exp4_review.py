#!/usr/bin/env python3
"""
exp4_review.py — per-instance review of one Exp.4 arm.

For each instance, gathers every _Front.csv produced by the arm (for the
LADDER arm: anchor + all levels of all runs; for the free arms: the run's
_Front) and builds the arm's GLOBAL non-dominated front (best over 30
runs). Reports n points, Cmax/NPE spans, and HV vs the exact front where
available. Also reports the per-run mean front size (spread) for context.

Usage:  ARM_DIR=<results/EXP4/LADDER> python3 exp4_review.py
"""
import glob, os, re, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from front_metrics import load_front, load_exact, hypervolume, nondominated

HERE = os.path.dirname(os.path.abspath(__file__))
ARM = os.environ.get("ARM_DIR",
                     os.path.join(HERE, "results", "EXP4", "LADDER"))
STEMS = ["F0.15.0.abz7_06", "F0.15.0.abz8_05", "F0.15.0.abz9_10",
         "F0.15.0.ft10_10", "F0.15.0.ft20_05", "F0.15.0.la21_04",
         "F0.15.0.la24_03", "F0.15.0.la25_04", "F0.15.0.la27_09",
         "F0.15.0.la29_03", "F0.15.0.la38_06", "F0.15.0.la40_05"]


def all_fronts(stem):
    """Every _Front.csv under the arm dir belonging to this instance."""
    return glob.glob(os.path.join(ARM, stem, "**", stem + "_*_Front.csv"),
                     recursive=True) + \
           glob.glob(os.path.join(ARM, stem + "_*_Front.csv"))


def main():
    print(f"ARM = {ARM}")
    print(f"{'instance':20} {'runs':>4} {'globN':>5} {'Cmax span':>17} "
          f"{'NPE span':>17} {'HVr':>6} {'run_n':>6}")
    for stem in STEMS:
        files = all_fronts(stem)
        if not files:
            print(f"{stem:20}  -- no data")
            continue
        allpts, per_run = [], []
        # group by run dir to count runs / mean size
        runs = {}
        for f in files:
            pts = load_front(f)
            allpts += pts
            key = os.path.dirname(f).replace("\\", "/").split("/")
            rid = next((p for p in key if re.fullmatch(r"r\d+", p)), f)
            runs.setdefault(rid, []).extend(pts)
        glob_nd = nondominated(allpts)
        for rid, pts in runs.items():
            per_run.append(len(nondominated(pts)))
        cs = [p[0] for p in glob_nd]; es = [p[1] for p in glob_nd]
        exact = load_exact(stem)
        hv = ""
        if exact and glob_nd:
            union = exact + glob_nd
            ref = (max(p[0] for p in union) * 1.05,
                   max(p[1] for p in union) * 1.05)
            hve = hypervolume(exact, ref)
            hv = f"{hypervolume(glob_nd, ref)/hve:.3f}" if hve > 0 else ""
        run_mean = sum(per_run)/len(per_run) if per_run else 0
        print(f"{stem:20} {len(runs):>4} {len(glob_nd):>5} "
              f"{min(cs):>8.0f}-{max(cs):<8.0f} {min(es):>8.0f}-{max(es):<8.0f} "
              f"{hv:>6} {run_mean:>6.1f}")


if __name__ == "__main__":
    main()
