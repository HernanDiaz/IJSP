#!/usr/bin/env python3
"""
gap_profile.py — WHERE does the residual gap between the tuned ladder and the
exact (midpoint-proxy) fronts live? For each exact-front point (sorted by
Cmax), report the ladder's relative NPE excess at that makespan allowance:
  gap%(c) = (bestNPE_ladder(Cmax<=c) / NPE_exact(c) - 1) * 100
If the gap grows toward the relaxed-makespan (deep-energy) end, the miss is
in energy-deep basins (neighbourhood/recombination target). If it peaks
mid-front, it is the ladder's cap-grid resolution. If at the tight end, the
anchor. Read-only.
"""
import os, sys, glob
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from front_metrics import load_exact, load_front, nondominated

HERE = os.path.dirname(os.path.abspath(__file__))
LAD = os.path.join(HERE, "results", "EXP4_TUNED", "LADDER")

def ladder_front(stem):
    pts = []
    for f in glob.glob(os.path.join(LAD, stem, "**", stem + "_*_Front.csv"),
                       recursive=True):
        pts += load_front(f)
    return nondominated(pts)

STEMS = ["F0.15.0.ft10_10", "F0.15.0.la21_04", "F0.15.0.la24_03",
         "F0.15.0.la25_04", "F0.15.0.la29_03", "tai15_15_01.F.15_01"]

for stem in STEMS:
    ex = load_exact(stem)
    if not ex:
        print(f"{stem}: sin frente exacto"); continue
    lf = sorted(ladder_front(stem))
    if not lf:
        print(f"{stem}: sin frente ladder"); continue
    print(f"\n== {stem} ==")
    print(f"{'pos':>6} {'Cmax_ex':>8} {'NPE_ex':>8} {'NPE_lad':>8} {'gap%':>7}")
    n = len(ex)
    for i, (c, e) in enumerate(sorted(ex)):
        elig = [pe for pc, pe in lf if pc <= c]
        if not elig:
            print(f"{'tight':>6} {c:>8.0f} {e:>8.0f} {'--':>8}  (ladder no llega a este Cmax)")
            continue
        best = min(elig)
        gap = (best / e - 1) * 100 if e > 0 else float('inf')
        pos = "tight" if i == 0 else ("deep" if i == n-1 else f"{i}/{n-1}")
        print(f"{pos:>6} {c:>8.0f} {e:>8.0f} {best:>8.0f} {gap:>+7.1f}")
