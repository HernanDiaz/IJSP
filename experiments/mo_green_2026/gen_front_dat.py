import os, sys
sys.path.insert(0, ".")
os.environ["EXP4_BASE"] = os.path.abspath("results/EXP4_TUNED")
from front_metrics import nondominated
from exp4_wilcoxon import per_run_fronts
OUT = "../../Papers/GreenIJSP/figures"
os.makedirs(OUT, exist_ok=True)
ARMS = ["LADDER", "ABC-P", "MA-P"]
TAG = {"LADDER": "LADDER", "ABC-P": "ABCP", "MA-P": "MAP"}
for stem, short in [("F0.15.0.ft10_10", "ft10"), ("tai30_20_01.F.15_01", "tai30_20_01")]:
    for a in ARMS:
        fr = per_run_fronts(a, stem)
        g = nondominated([p for pts in fr.values() for p in pts])
        g = sorted(g)  # by Cmax
        path = os.path.join(OUT, f"{short}_{TAG[a]}.dat")
        with open(path, "w") as fh:
            fh.write("cmax npe\n")
            for c, e in g:
                fh.write(f"{c:.1f} {e:.1f}\n")
        print(f"{short:14} {a:7} {len(g):3} pts  Cmax[{g[0][0]:.0f},{g[-1][0]:.0f}]")
