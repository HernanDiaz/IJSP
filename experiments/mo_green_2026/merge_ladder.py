#!/usr/bin/env python3
"""Merge the per-level ladder fronts into one non-dominated front per
instance, write results/pilot_ladder/merged/<stem>_Front.csv and report
size, spans and HV vs the exact front where available."""
import csv, glob, os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from front_metrics import load_exact, hypervolume, nondominated

HERE = os.path.dirname(os.path.abspath(__file__))
LAD = os.path.join(HERE, "results", "pilot_ladder")
MERGED = os.path.join(LAD, "merged")
os.makedirs(MERGED, exist_ok=True)

stems = sorted(set(os.path.basename(p).split("_2026")[0]
                   for p in glob.glob(os.path.join(LAD, "L*", "*_Front.csv"))))
print(f"{'instance':22} {'n':>3} {'Cmax span':>18} {'NPE span':>18} {'HVr':>6}")
for stem in stems:
    rows = []
    for p in glob.glob(os.path.join(LAD, "L*", stem + "*_Front.csv")):
        with open(p, newline="") as fh:
            for r in csv.DictReader(fh, delimiter=";"):
                rows.append(((float(r["cmax_lo"]) + float(r["cmax_hi"])) / 2,
                             (float(r["npe_lo"]) + float(r["npe_hi"])) / 2,
                             r))
    pts = nondominated([(c, e) for c, e, _ in rows])
    keep = {p: True for p in pts}
    with open(os.path.join(MERGED, stem + "_Front.csv"), "w", newline="") as fh:
        w = csv.writer(fh, delimiter=";")
        w.writerow(["cmax_lo", "cmax_hi", "npe_lo", "npe_hi", "solution"])
        seen = set()
        for c, e, r in rows:
            if (c, e) in keep and (c, e) not in seen:
                seen.add((c, e))
                w.writerow([r["cmax_lo"], r["cmax_hi"], r["npe_lo"],
                            r["npe_hi"], r["solution"]])
    cs = [p[0] for p in pts]; es = [p[1] for p in pts]
    exact = load_exact(stem)
    hv = ""
    if exact:
        union = exact + pts
        ref = (max(p[0] for p in union) * 1.05, max(p[1] for p in union) * 1.05)
        hve = hypervolume(exact, ref)
        hv = f"{hypervolume(pts, ref) / hve:.3f}" if hve > 0 else ""
    print(f"{stem:22} {len(pts):>3} {min(cs):>8.0f}-{max(cs):<8.0f} "
          f"{min(es):>8.0f}-{max(es):<8.0f} {hv:>6}")
