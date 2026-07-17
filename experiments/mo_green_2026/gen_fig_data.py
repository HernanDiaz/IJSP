#!/usr/bin/env python3
"""
gen_fig_data.py — emits the .dat files for the paper's added figures:
  (a) probe_<short>.dat: normalised lower-envelope NPE vs makespan allowance
      (%) for the nine conflict-probe instances (fronts.csv + fronts_ext.csv,
      la29 re-probe superseding the original rows).
  (b) tai30_20_01_LADDERSTAR.dat: the definitive ladder's front on the
      showcase instance, for fig:fronts panel (b).
Writes only under Papers/GreenIJSP/figures/.
"""
import csv, os, sys, glob
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from front_metrics import load_front, nondominated
OUT = os.path.join(HERE, "..", "..", "Papers", "GreenIJSP", "figures")
os.makedirs(OUT, exist_ok=True)

# ---------- (a) probe curves ----------
def load(path, skip=()):
    rows = {}
    if not os.path.exists(path):
        return rows
    with open(path, newline="") as fh:
        for r in csv.DictReader(fh):
            if r["instance"] in skip:
                continue
            v = float(r["npe_x2"])
            if v >= 0:
                rows.setdefault(r["instance"], {})[int(r["eps_permil"])] = v / 2
    return rows

data = load(os.path.join(HERE, "results", "fronts.csv"),
            skip={"F0.15.0.la29_03"})
data.update(load(os.path.join(HERE, "results", "fronts_ext.csv")))

SHORT = {"F0.15.0.ft10_10": "ft10", "F0.15.0.ft20_05": "ft20",
         "F0.15.0.la21_04": "la21", "F0.15.0.la24_03": "la24",
         "F0.15.0.la25_04": "la25", "F0.15.0.la29_03": "la29",
         "F0.15.0.abz7_06": "abz7", "tai15_15_01.F.15_01": "tai15a",
         "tai15_15_02.F.15_01": "tai15b"}

for stem, short in SHORT.items():
    d = data.get(stem)
    if not d:
        print(f"{short}: sin datos"); continue
    solved = sorted(d)
    env, best = [], float("inf")
    for pm in solved:
        best = min(best, d[pm])
        env.append((pm, best))
    base = env[0][1]
    with open(os.path.join(OUT, f"probe_{short}.dat"), "w") as fh:
        fh.write("eps npe_rel\n")
        for pm, v in env:
            rel = v / base if base > 0 else 0.0
            fh.write(f"{pm/10:.1f} {rel:.4f}\n")
    print(f"probe_{short}.dat: {len(env)} pts, base={base:.0f}, "
          f"min_rel={min(v for _, v in env)/base if base>0 else 0:.3f}")

# ---------- (b) LADDER* front on tai30_20_01 ----------
stem = "tai30_20_01.F.15_01"
pts = []
for f in glob.glob(os.path.join(HERE, "results", "EXP4_TUNED_TSN2ANCHOR",
                                "LADDER", stem, "**", stem + "_*_Front.csv"),
                   recursive=True):
    pts += load_front(f)
g = sorted(nondominated(pts))
with open(os.path.join(OUT, "tai30_20_01_LADDERSTAR.dat"), "w") as fh:
    fh.write("cmax npe\n")
    for c, e in g:
        fh.write(f"{c:.1f} {e:.1f}\n")
print(f"tai30_20_01_LADDERSTAR.dat: {len(g)} pts, "
      f"Cmax[{g[0][0]:.0f},{g[-1][0]:.0f}]")
