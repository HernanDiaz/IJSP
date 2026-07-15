#!/usr/bin/env python3
"""
m9_analysis.py — did strengthening the ladder's anchor recover the two large
x20 groups (tai30_20, tai50_20)? Compares the strengthened-anchor ladder
(results/EXP4_TUNED_STRONGANCHOR) against the three original tuned arms
(results/EXP4_TUNED) on those 20 instances, on:
  (a) HV ratio vs a common reference = non-dominated union of ALL four arms,
      1.05x nadir (fixed, so all four are comparable);
  (b) best expected makespan of the ladder anchor vs the original ladder and
      vs the dedicated makespan-only TS-N2 (runs_data.csv).
Read-only.
"""
import os, sys, csv, re
import statistics as st
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from front_metrics import hypervolume, nondominated

STRONG = os.path.join(HERE, "results", "EXP4_TUNED_STRONGANCHOR")
TUNED = os.path.join(HERE, "results", "EXP4_TUNED")
import glob
def per_run_fronts(base, arm, stem):
    from front_metrics import load_front
    runs = {}
    for f in glob.glob(os.path.join(base, arm, stem, "**",
                                    stem + "_*_Front.csv"), recursive=True):
        m = re.search(r"(?:^|/|\\)r(\d+)(?:/|\\|$)", f.replace("\\", "/"))
        key = "r" + m.group(1) if m else f
        runs.setdefault(key, []).extend(load_front(f))
    for f in glob.glob(os.path.join(base, arm, stem + "_*_Front.csv")):
        runs.setdefault(f, []).extend(load_front(f))
    return {k: nondominated(v) for k, v in runs.items() if v}

def gfront(base, arm, stem):
    fr = per_run_fronts(base, arm, stem)
    return nondominated([p for pts in fr.values() for p in pts])

# TS-N2 best expected makespan per instance
n2 = {}
with open(os.path.join(HERE, "..", "cor_tabu_2026", "statistical_results",
                       "runs_data.csv"), newline="") as fh:
    for r in csv.reader(fh):
        if r and r[0] == "n2_TS":
            n2.setdefault(r[3], []).append(float(r[7]))
n2best = {k: min(v) for k, v in n2.items()}

stems = sorted(d for d in os.listdir(os.path.join(STRONG, "LADDER"))
               if os.path.isdir(os.path.join(STRONG, "LADDER", d)))
ARMS = ["LADDER+", "LADDER", "ABC-P", "MA-P"]  # LADDER+ = strengthened
def group_of(s): return re.match(r"(tai\d+_\d+)", s).group(1)

print("=" * 78)
print("M9: escalera ANCLA-REFORZADA (LADDER+) vs brazos originales — HV ratio")
print("=" * 78)
print(f"{'instancia':22} | " + " ".join(f"{a:>8}" for a in ARMS) + " | winner")
rows = []
for stem in stems:
    fronts = {"LADDER+": gfront(STRONG, "LADDER", stem),
              "LADDER": gfront(TUNED, "LADDER", stem),
              "ABC-P": gfront(TUNED, "ABC-P", stem),
              "MA-P": gfront(TUNED, "MA-P", stem)}
    allp = [p for a in ARMS for p in fronts[a]]
    nad = (max(p[0] for p in allp), max(p[1] for p in allp))
    ref = (nad[0] * 1.05, nad[1] * 1.05)
    refhv = hypervolume(nondominated(allp), ref) or 1.0
    r = {a: hypervolume(fronts[a], ref) / refhv if fronts[a] else 0.0
         for a in ARMS}
    win = max(ARMS, key=lambda a: r[a])
    rows.append((stem, group_of(stem), r, win))
    print(f"{stem:22} | " + " ".join(f"{r[a]:>8.3f}" for a in ARMS)
          + f" | {win}")

print("\n" + "=" * 78)
print("POR GRUPO — media HV ratio y victorias")
print("=" * 78)
print(f"{'grupo':12} {'#':>3} | " + " ".join(f"{a:>8}" for a in ARMS)
      + " | wins L+/L/A/M")
for g in ("tai30_20", "tai50_20"):
    gr = [x for x in rows if x[1] == g]
    m = {a: st.mean(x[2][a] for x in gr) for a in ARMS}
    w = {a: sum(1 for x in gr if x[3] == a) for a in ARMS}
    print(f"{g:12} {len(gr):>3} | " + " ".join(f"{m[a]:>8.3f}" for a in ARMS)
          + f" | {w['LADDER+']}/{w['LADDER']}/{w['ABC-P']}/{w['MA-P']}")

print("\n" + "=" * 78)
print("MAKESPAN del ancla: best expected Cmax (mid) LADDER+ vs LADDER vs TS-N2")
print("=" * 78)
print(f"{'instancia':22} | {'TS-N2':>8} {'LADDER':>8} {'LADDER+':>8} "
      f"{'L+ gap%':>8}")
def anchor_best(base, stem):
    best = None
    for f in glob.glob(os.path.join(base, "LADDER", stem, "r*", "anchor",
                                    stem + "_*_Front.csv")):
        from front_metrics import load_front
        for c, _e in load_front(f):
            if best is None or c < best:
                best = c
    return best
for g in ("tai30_20", "tai50_20"):
    gaps = []
    for stem in [s for s in stems if group_of(s) == g]:
        t = n2best.get(stem); lo = anchor_best(TUNED, stem)
        ln = anchor_best(STRONG, stem)
        if None in (t, lo, ln): continue
        gap = (ln - t) / t * 100
        gaps.append(gap)
        print(f"{stem:22} | {t:>8.1f} {lo:>8.1f} {ln:>8.1f} {gap:>+8.2f}")
    if gaps:
        print(f"  -> {g} media gap L+ vs TS-N2: {st.mean(gaps):+.2f}%")
