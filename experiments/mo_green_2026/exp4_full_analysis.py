#!/usr/bin/env python3
"""
exp4_full_analysis.py — definitive 82-instance Exp.4 analysis (tuned arms).
Reuses the EXACT methodology of exp4_compare.py / exp4_wilcoxon.py (imported,
not reimplemented) but discovers all 82 instances and aggregates by size group.
Route-only additive script; the classical scripts are untouched.

  * HV ratio (headline): HV(arm global best-over-30 front)/HV(union+exact ref).
  * Per-run paired Wilcoxon (LADDER vs each baseline), pairing by run index.
Set EXP4_BASE (absolute) before running.
"""
import os, sys, glob, re
import statistics as st
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.environ.setdefault("EXP4_BASE",
                      os.path.join(HERE, "results", "EXP4_TUNED"))
BASE = os.environ["EXP4_BASE"]
from front_metrics import load_exact, hypervolume, nondominated
from exp4_wilcoxon import per_run_fronts, wilcoxon   # reuse tested code
ARMS = ["LADDER", "ABC-P", "MA-P"]

def group_of(stem):
    if stem.startswith("F0.15.0."):
        return "classical"
    return re.match(r"(tai\d+_\d+)", stem).group(1)

ORDER = ["classical", "tai15_15", "tai20_15", "tai20_20",
         "tai30_15", "tai30_20", "tai50_15", "tai50_20"]

# discover 82 stems from LADDER dirs
stems = sorted(d for d in os.listdir(os.path.join(BASE, "LADDER"))
               if os.path.isdir(os.path.join(BASE, "LADDER", d)))

rows = []           # (stem, group, {arm:ratio}, winner, pLA, pLM, medHV{})
for stem in stems:
    fr = {a: per_run_fronts(a, stem) for a in ARMS}
    # global best-over-30 front per arm
    gfront = {a: nondominated([p for pts in fr[a].values() for p in pts])
              for a in ARMS}
    allpts = [p for a in ARMS for p in gfront[a]]
    ex = load_exact(stem)
    refpts = allpts + (ex or [])
    if not refpts:
        continue
    nad = (max(p[0] for p in refpts), max(p[1] for p in refpts))
    ref = (nad[0] * 1.05, nad[1] * 1.05)
    refhv = hypervolume(nondominated(refpts), ref) or 1.0
    ratio = {a: (hypervolume(gfront[a], ref) / refhv if gfront[a] else 0.0)
             for a in ARMS}
    winner = max(ARMS, key=lambda a: ratio[a])
    # per-run HV vectors for Wilcoxon
    hv = {}
    for a in ARMS:
        keys = sorted(fr[a], key=lambda k: (len(k), k))
        hv[a] = [hypervolume(fr[a][k], ref) / refhv for k in keys]
    n = min(len(hv[a]) for a in ARMS) if all(hv[a] for a in ARMS) else 0
    pLA = wilcoxon(hv["LADDER"][:n], hv["ABC-P"][:n]) if n else 1.0
    pLM = wilcoxon(hv["LADDER"][:n], hv["MA-P"][:n]) if n else 1.0
    medHV = {a: (st.median(hv[a]) if hv[a] else 0.0) for a in ARMS}
    rows.append((stem, group_of(stem), ratio, winner, pLA, pLM, medHV, hv))

# ---------- per-group table ----------
print("=" * 72)
print("TABLA HV-ratio (frente global best-of-30) por GRUPO — media por arm")
print("=" * 72)
print(f"{'grupo':12} {'#':>3} | {'LADDER':>7} {'ABC-P':>7} {'MA-P':>7} | "
      f"{'victorias LADDER/ABC/MA':>24}")
tot = {a: [] for a in ARMS}
wins_tot = {a: 0 for a in ARMS}
for g in ORDER:
    gr = [r for r in rows if r[1] == g]
    if not gr: continue
    m = {a: st.mean(r[2][a] for r in gr) for a in ARMS}
    w = {a: sum(1 for r in gr if r[3] == a) for a in ARMS}
    for a in ARMS:
        tot[a] += [r[2][a] for r in gr]; wins_tot[a] += w[a]
    print(f"{g:12} {len(gr):>3} | {m['LADDER']:>7.3f} {m['ABC-P']:>7.3f} "
          f"{m['MA-P']:>7.3f} | {w['LADDER']:>8}/{w['ABC-P']}/{w['MA-P']}")
print("-" * 72)
print(f"{'GLOBAL 82':12} {len(rows):>3} | "
      f"{st.mean(tot['LADDER']):>7.3f} {st.mean(tot['ABC-P']):>7.3f} "
      f"{st.mean(tot['MA-P']):>7.3f} | "
      f"{wins_tot['LADDER']:>8}/{wins_tot['ABC-P']}/{wins_tot['MA-P']}")

# ---------- Wilcoxon significance summary ----------
print("\n" + "=" * 72)
print("WILCOXON pareado por-run (alpha=0.05): LADDER vs cada baseline")
print("=" * 72)
def summ(gr):
    la = sum(1 for r in gr if r[6]['LADDER'] > r[6]['ABC-P'] and r[4] < 0.05)
    lm = sum(1 for r in gr if r[6]['LADDER'] > r[6]['MA-P']  and r[5] < 0.05)
    loss = sum(1 for r in gr if
               (r[6]['ABC-P'] > r[6]['LADDER'] and r[4] < 0.05) or
               (r[6]['MA-P']  > r[6]['LADDER'] and r[5] < 0.05))
    return la, lm, loss
print(f"{'grupo':12} {'#':>3} | {'L>ABC-P':>8} {'L>MA-P':>8} {'derrotas L':>11}")
for g in ORDER:
    gr = [r for r in rows if r[1] == g]
    if not gr: continue
    la, lm, loss = summ(gr)
    print(f"{g:12} {len(gr):>3} | {la:>8} {lm:>8} {loss:>11}")
la, lm, loss = summ(rows)
print("-" * 48)
print(f"{'GLOBAL 82':12} {len(rows):>3} | {la:>8} {lm:>8} {loss:>11}")

# pooled per-run medians
pooled = {a: [x for r in rows for x in r[7][a]] for a in ARMS}
print("\nMediana HV por-run (pooled 82x30): "
      + "  ".join(f"{a}={st.median(pooled[a]):.3f}" for a in ARMS))
