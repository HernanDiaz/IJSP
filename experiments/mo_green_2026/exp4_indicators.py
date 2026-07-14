#!/usr/bin/env python3
"""
exp4_indicators.py — complementary quality indicators for Exp.4 (tuned arms),
addressing the reviewer requests: (i) additive epsilon-indicator per arm vs
the union reference front (promised in Sec. 5.6); (ii) HV ratio against the
EXACT (midpoint-proxy) fronts on the probed instances; (iii) sensitivity of
the HV ranking to the reference-point factor (1.01 / 1.05 / 1.10 / 1.20).

Read-only over results/EXP4_TUNED and results/fronts.csv; writes nothing.
"""
import os, sys, re
import statistics as st
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.environ.setdefault("EXP4_BASE",
                      os.path.join(HERE, "results", "EXP4_TUNED"))
BASE = os.environ["EXP4_BASE"]
from front_metrics import load_exact, hypervolume, nondominated, eps_indicator
from exp4_wilcoxon import per_run_fronts
ARMS = ["LADDER", "ABC-P", "MA-P"]

def group_of(stem):
    if stem.startswith("F0.15.0."):
        return "classical"
    return re.match(r"(tai\d+_\d+)", stem).group(1)

ORDER = ["classical", "tai15_15", "tai20_15", "tai20_20",
         "tai30_15", "tai30_20", "tai50_15", "tai50_20"]

stems = sorted(d for d in os.listdir(os.path.join(BASE, "LADDER"))
               if os.path.isdir(os.path.join(BASE, "LADDER", d)))

# cache global fronts per stem
G, EX = {}, {}
for stem in stems:
    fr = {a: per_run_fronts(a, stem) for a in ARMS}
    G[stem] = {a: nondominated([p for pts in fr[a].values() for p in pts])
               for a in ARMS}
    EX[stem] = load_exact(stem)

# ---------- (i) additive epsilon-indicator vs union reference ----------
print("=" * 70)
print("EPSILON-INDICATOR aditivo (vs frente union; menor es mejor)")
print("=" * 70)
eps_g = {g: {a: [] for a in ARMS} for g in ORDER}
for stem in stems:
    allp = [p for a in ARMS for p in G[stem][a]] + (EX[stem] or [])
    union = nondominated(allp)
    if not union:
        continue
    # normalise by the spread of ALL arms' points (the union alone can
    # degenerate to a single point on instances one arm fully dominates)
    cs = [p[0] for p in allp]; es = [p[1] for p in allp]
    ranges = (max(max(cs) - min(cs), 1.0), max(max(es) - min(es), 1.0))
    for a in ARMS:
        if G[stem][a]:
            eps_g[group_of(stem)][a].append(
                eps_indicator(G[stem][a], union, ranges))
print(f"{'grupo':12} {'#':>3} | {'LADDER':>7} {'ABC-P':>7} {'MA-P':>7}")
allv = {a: [] for a in ARMS}
for g in ORDER:
    if not eps_g[g][ARMS[0]]:
        continue
    n = len(eps_g[g][ARMS[0]])
    for a in ARMS:
        allv[a] += eps_g[g][a]
    print(f"{g:12} {n:>3} | " + " ".join(
        f"{st.mean(eps_g[g][a]):>7.3f}" for a in ARMS))
print("-" * 46)
print(f"{'GLOBAL':12} {len(allv[ARMS[0]]):>3} | " + " ".join(
    f"{st.mean(allv[a]):>7.3f}" for a in ARMS))

# ---------- (ii) HV vs exact fronts on probed instances ----------
print("\n" + "=" * 70)
print("HV-ratio vs FRENTE EXACTO (proxy midpoint) en instancias sondeadas")
print("=" * 70)
print(f"{'instancia':22} | {'LADDER':>7} {'ABC-P':>7} {'MA-P':>7}")
for stem in stems:
    ex = EX[stem]
    if not ex:
        continue
    allpts = [p for a in ARMS for p in G[stem][a]] + ex
    nad = (max(p[0] for p in allpts), max(p[1] for p in allpts))
    ref = (nad[0] * 1.05, nad[1] * 1.05)
    hvex = hypervolume(nondominated(ex), ref)
    if hvex <= 0:
        continue
    row = " ".join(f"{hypervolume(G[stem][a], ref)/hvex:>7.3f}" for a in ARMS)
    print(f"{stem:22} | {row}")

# ---------- (iii) sensitivity to the reference-point factor ----------
print("\n" + "=" * 70)
print("SENSIBILIDAD del ranking HV al factor del punto de referencia")
print("=" * 70)
print(f"{'factor':>7} | {'LADDER':>7} {'ABC-P':>7} {'MA-P':>7} | "
      f"{'victorias L/A/M':>16}")
for fac in (1.01, 1.05, 1.10, 1.20):
    tot = {a: [] for a in ARMS}
    wins = {a: 0 for a in ARMS}
    for stem in stems:
        refpts = [p for a in ARMS for p in G[stem][a]] + (EX[stem] or [])
        if not refpts:
            continue
        nad = (max(p[0] for p in refpts), max(p[1] for p in refpts))
        ref = (nad[0] * fac, nad[1] * fac)
        refhv = hypervolume(nondominated(refpts), ref) or 1.0
        r = {a: (hypervolume(G[stem][a], ref) / refhv if G[stem][a] else 0.0)
             for a in ARMS}
        for a in ARMS:
            tot[a].append(r[a])
        wins[max(ARMS, key=lambda a: r[a])] += 1
    print(f"{fac:>7.2f} | " + " ".join(f"{st.mean(tot[a]):>7.3f}"
                                        for a in ARMS)
          + f" | {wins['LADDER']:>6}/{wins['ABC-P']}/{wins['MA-P']}")
