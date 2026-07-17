#!/usr/bin/env python3
"""
ladderstar_analysis.py — definitive evaluation of LADDER* (TSN2-anchor +
tuned levels, results/EXP4_TUNED_TSN2ANCHOR) on the FULL 82-instance
benchmark against the three uniform-configuration arms of EXP4_TUNED.

Per instance: global best-over-30 fronts; reference = non-dominated union of
the FOUR arms (+ exact where available), 1.05x nadir. Reports group means,
wins, per-instance Mann-Whitney (LADDER* vs each control, Holm across 82),
and Friedman over the four arms on per-instance median per-run HV.
Read-only.
"""
import os, sys, re, math, glob
import statistics as st
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.environ.setdefault("EXP4_BASE", os.path.join(HERE, "results", "EXP4_TUNED"))
from front_metrics import load_exact, hypervolume, nondominated, load_front

STAR = os.path.join(HERE, "results", "EXP4_TUNED_TSN2ANCHOR")
TUNED = os.path.join(HERE, "results", "EXP4_TUNED")
ARMS = ["LADDER*", "LADDER", "ABC-P", "MA-P"]
BASE_OF = {"LADDER*": (STAR, "LADDER"), "LADDER": (TUNED, "LADDER"),
           "ABC-P": (TUNED, "ABC-P"), "MA-P": (TUNED, "MA-P")}

SQRT2 = math.sqrt(2.0)
def _sf(z): return 0.5 * math.erfc(z / SQRT2)

def mwu(x, y):
    n1, n2 = len(x), len(y); N = n1 + n2
    comb = sorted([(v, 0) for v in x] + [(v, 1) for v in y])
    ranks = [0.0] * N; i = 0; ties = 0.0
    while i < N:
        j = i
        while j < N and comb[j][0] == comb[i][0]: j += 1
        avg = (i + 1 + j) / 2.0; t = j - i; ties += t ** 3 - t
        for k in range(i, j): ranks[k] = avg
        i = j
    R1 = sum(ranks[k] for k in range(N) if comb[k][1] == 0)
    U1 = R1 - n1 * (n1 + 1) / 2.0
    mu = n1 * n2 / 2.0
    sig2 = (n1 * n2 / 12.0) * ((N + 1) - ties / (N * (N - 1)))
    if sig2 <= 0: return 1.0
    return min(1.0, 2 * _sf(abs((U1 - mu) / math.sqrt(sig2))))

def holm(pvals, alpha=0.05):
    m = len(pvals)
    order = sorted(range(m), key=lambda i: pvals[i])
    rej = [False] * m
    for rank, idx in enumerate(order):
        if pvals[idx] <= alpha / (m - rank): rej[idx] = True
        else: break
    return rej

def friedman(cols):
    arms = list(cols); k = len(arms); n = len(cols[arms[0]])
    Rsum = {a: 0.0 for a in arms}
    for i in range(n):
        vals = sorted((cols[a][i], a) for a in arms)
        j = 0
        while j < k:
            l = j
            while l < k and vals[l][0] == vals[j][0]: l += 1
            avg = (j + 1 + l) / 2.0
            for t in range(j, l): Rsum[vals[t][1]] += avg
            j = l
    chi2 = 12.0 * n / (k * (k + 1)) * sum(
        (Rsum[a] / n - (k + 1) / 2.0) ** 2 for a in arms)
    # k=4 -> chi2 with 3 df: sf via Wilson-Hilferty approx
    z = ((chi2 / 3.0) ** (1.0/3.0) - (1 - 2.0/27.0)) / math.sqrt(2.0/27.0)
    p = _sf(z)
    return chi2, p, {a: Rsum[a] / n for a in arms}

def per_run_fronts(base, arm, stem):
    runs = {}
    for f in glob.glob(os.path.join(base, arm, stem, "**",
                                    stem + "_*_Front.csv"), recursive=True):
        m = re.search(r"(?:^|/|\\)r(\d+)(?:/|\\|$)", f.replace("\\", "/"))
        key = "r" + m.group(1) if m else f
        runs.setdefault(key, []).extend(load_front(f))
    for f in glob.glob(os.path.join(base, arm, stem + "_*_Front.csv")):
        runs.setdefault(f, []).extend(load_front(f))
    return {k: nondominated(v) for k, v in runs.items() if v}

def group_of(stem):
    if stem.startswith("F0.15.0."): return "classical"
    return re.match(r"(tai\d+_\d+)", stem).group(1)

ORDER = ["classical", "tai15_15", "tai20_15", "tai20_20",
         "tai30_15", "tai30_20", "tai50_15", "tai50_20"]

stems = sorted(d for d in os.listdir(os.path.join(STAR, "LADDER"))
               if os.path.isdir(os.path.join(STAR, "LADDER", d)))

rows = []
for stem in stems:
    fr = {a: per_run_fronts(*BASE_OF[a], stem) for a in ARMS}
    g = {a: nondominated([p for pts in fr[a].values() for p in pts])
         for a in ARMS}
    allp = [p for a in ARMS for p in g[a]] + (load_exact(stem) or [])
    nad = (max(p[0] for p in allp), max(p[1] for p in allp))
    ref = (nad[0] * 1.05, nad[1] * 1.05)
    refhv = hypervolume(nondominated(allp), ref) or 1.0
    ratio = {a: (hypervolume(g[a], ref) / refhv if g[a] else 0.0)
             for a in ARMS}
    winner = max(ARMS, key=lambda a: ratio[a])
    hv = {a: [hypervolume(fr[a][k], ref) / refhv
              for k in sorted(fr[a], key=lambda x: (len(x), x))]
          for a in ARMS}
    pSA = mwu(hv["LADDER*"], hv["ABC-P"])
    pSM = mwu(hv["LADDER*"], hv["MA-P"])
    pSL = mwu(hv["LADDER*"], hv["LADDER"])
    med = {a: (st.median(hv[a]) if hv[a] else 0.0) for a in ARMS}
    rows.append((stem, group_of(stem), ratio, winner, pSA, pSM, pSL, med))

print("=" * 78)
print("DEFINITIVO: LADDER* (ancla TSN2) vs brazos uniformes — 82 instancias")
print("=" * 78)
print(f"{'grupo':12} {'#':>3} | {'LADDER*':>8} {'LADDER':>7} {'ABC-P':>7} "
      f"{'MA-P':>7} | wins *|L|A|M")
tot = {a: [] for a in ARMS}; wins = {a: 0 for a in ARMS}
for gk in ORDER:
    gr = [r for r in rows if r[1] == gk]
    if not gr: continue
    m = {a: st.mean(r[2][a] for r in gr) for a in ARMS}
    w = {a: sum(1 for r in gr if r[3] == a) for a in ARMS}
    for a in ARMS:
        tot[a] += [r[2][a] for r in gr]; wins[a] += w[a]
    print(f"{gk:12} {len(gr):>3} | {m['LADDER*']:>8.3f} {m['LADDER']:>7.3f} "
          f"{m['ABC-P']:>7.3f} {m['MA-P']:>7.3f} | "
          f"{w['LADDER*']}/{w['LADDER']}/{w['ABC-P']}/{w['MA-P']}")
print("-" * 78)
print(f"{'GLOBAL 82':12} {len(rows):>3} | {st.mean(tot['LADDER*']):>8.3f} "
      f"{st.mean(tot['LADDER']):>7.3f} {st.mean(tot['ABC-P']):>7.3f} "
      f"{st.mean(tot['MA-P']):>7.3f} | "
      f"{wins['LADDER*']}/{wins['LADDER']}/{wins['ABC-P']}/{wins['MA-P']}")

print("\nFRIEDMAN (4 brazos, mediana HV por-run, n=82):")
cols = {a: [r[7][a] for r in rows] for a in ARMS}
chi2, p, mr = friedman(cols)
print(f"  chi2={chi2:.2f} p={p:.3e}  rangos: "
      + "  ".join(f"{a}={mr[a]:.2f}" for a in ARMS))

print("\nPOR INSTANCIA (MWU 30v30 + Holm sobre 82): LADDER* significativamente >")
rejA = holm([r[4] for r in rows]); rejM = holm([r[5] for r in rows])
rejL = holm([r[6] for r in rows])
cA = sum(1 for r, x in zip(rows, rejA) if x and r[7]['LADDER*'] > r[7]['ABC-P'])
cM = sum(1 for r, x in zip(rows, rejM) if x and r[7]['LADDER*'] > r[7]['MA-P'])
cL = sum(1 for r, x in zip(rows, rejL) if x and r[7]['LADDER*'] > r[7]['LADDER'])
lA = sum(1 for r, x in zip(rows, rejA) if x and r[7]['LADDER*'] < r[7]['ABC-P'])
lM = sum(1 for r, x in zip(rows, rejM) if x and r[7]['LADDER*'] < r[7]['MA-P'])
lL = sum(1 for r, x in zip(rows, rejL) if x and r[7]['LADDER*'] < r[7]['LADDER'])
print(f"  vs ABC-P: {cA}/82 (derrotas {lA})   vs MA-P: {cM}/82 (derrotas {lM})"
      f"   vs LADDER-uniforme: {cL}/82 (derrotas {lL})")
