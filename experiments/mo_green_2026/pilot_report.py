#!/usr/bin/env python3
"""
pilot_report.py — Pareto engine pilot: ABC archive vs MA/NSGA-II.
Per instance and engine: front size, objective-space coverage (midpoint
spans), best-makespan extreme, and HV ratio vs the exact front where
available (ft10, tai15_01).
"""
import glob, os, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from front_metrics import load_front, load_exact, hypervolume

HERE = os.path.dirname(os.path.abspath(__file__))
PILOT = os.environ.get("PILOT_DIR", os.path.join(HERE, "results", "pilot_pareto"))


def main():
    print(f"{'instance':22} {'eng':>4} {'n':>4} {'Cmax span':>18} "
          f"{'NPE span':>18} {'bestCmax':>9} {'HVr':>6}")
    insts = sorted(set(
        os.path.basename(p).split("_2026")[0]
        for p in glob.glob(os.path.join(PILOT, "abc", "*_Front.csv"))))
    for inst in insts:
        exact = load_exact(inst)
        for eng in ("abc", "ma"):
            files = glob.glob(os.path.join(PILOT, eng, inst + "*_Front.csv"))
            if not files:
                print(f"{inst:22} {eng:>4}  -- missing")
                continue
            pts = load_front(sorted(files)[-1])
            cs = [p[0] for p in pts]; es = [p[1] for p in pts]
            hv = ""
            if exact:
                union = exact + pts
                ref = (max(p[0] for p in union) * 1.05,
                       max(p[1] for p in union) * 1.05)
                hve = hypervolume(exact, ref)
                hv = f"{hypervolume(pts, ref) / hve:.3f}" if hve > 0 else ""
            print(f"{inst:22} {eng:>4} {len(pts):>4} "
                  f"{min(cs):>8.0f}-{max(cs):<8.0f} "
                  f"{min(es):>8.0f}-{max(es):<8.0f} {min(cs):>9.0f} {hv:>6}")


if __name__ == "__main__":
    main()
