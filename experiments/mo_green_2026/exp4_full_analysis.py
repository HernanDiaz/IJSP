#!/usr/bin/env python3
"""
exp4_full_analysis.py — definitive 82-instance Exp.4 analysis (tuned arms).
Reuses the front-loading of exp4_wilcoxon.py (imported) but discovers all 82
instances, aggregates by size group, and applies review-grade statistics:

  * HV ratio (headline): HV(arm global best-over-30 front)/HV(union+exact ref).
  * Omnibus: Friedman test across the three arms on per-instance median
    per-run HV (n=82 blocks), followed by pairwise Wilcoxon signed-rank
    post-hoc over instances (valid pairing: same instance) with Holm
    correction over the three pairwise comparisons.
  * Per-instance: unpaired Mann-Whitney U over the 30 per-run HV values
    (runs of different engines are independent; a per-run pairing by index
    is not defensible), Holm-corrected across the 82 instances per
    comparison.

Set EXP4_BASE (absolute) before running; default = results/EXP4_TUNED.
"""
import os, sys, re, math
import statistics as st
HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
os.environ.setdefault("EXP4_BASE",
                      os.path.join(HERE, "results", "EXP4_TUNED"))
BASE = os.environ["EXP4_BASE"]
from front_metrics import load_exact, hypervolume, nondominated
from exp4_wilcoxon import per_run_fronts   # reuse tested front loading
ARMS = ["LADDER", "ABC-P", "MA-P"]

SQRT2 = math.sqrt(2.0)
def _sf(z): return 0.5 * math.erfc(z / SQRT2)

def mwu(x, y):
    """Unpaired Mann-Whitney U, normal approx with tie correction -> p."""
    n1, n2 = len(x), len(y); N = n1 + n2
    comb = sorted([(v, 0) for v in x] + [(v, 1) for v in y])
    ranks = [0.0] * N; i = 0; ties = 0.0
    while i < N:
        j = i
        while j < N and comb[j][0] == comb[i][0]:
            j += 1
        avg = (i + 1 + j) / 2.0; t = j - i; ties += t ** 3 - t
        for k in range(i, j): ranks[k] = avg
        i = j
    R1 = sum(ranks[k] for k in range(N) if comb[k][1] == 0)
    U1 = R1 - n1 * (n1 + 1) / 2.0
    mu = n1 * n2 / 2.0
    sig2 = (n1 * n2 / 12.0) * ((N + 1) - ties / (N * (N - 1)))
    if sig2 <= 0: return 1.0
    return min(1.0, 2 * _sf(abs((U1 - mu) / math.sqrt(sig2))))

def signed_rank(a, b):
    """Paired Wilcoxon signed-rank (normal approx, ties/zeros dropped)."""
    d = [x - y for x, y in zip(a, b) if x != y]
    n = len(d)
    if n == 0: return 1.0
    vals = sorted(abs(x) for x in d)
    ranks = [0.0] * n; i = 0; ties = 0.0
    while i < n:
        j = i
        while j < n and vals[j] == vals[i]:
            j += 1
        avg = (i + 1 + j) / 2.0; t = j - i; ties += t ** 3 - t
        for k in range(i, j): ranks[k] = avg
        i = j
    from collections import defaultdict, deque
    rmap = defaultdict(deque)
    for v, r in zip(vals, ranks): rmap[v].append(r)
    Wp = 0.0
    for x in d:
        r = rmap[abs(x)].popleft()
        if x > 0: Wp += r
    mu = n * (n + 1) / 4.0
    sig2 = n * (n + 1) * (2 * n + 1) / 24.0 - ties / 48.0
    return min(1.0, 2 * _sf(abs((Wp - mu) / math.sqrt(sig2))))

def holm(pvals, alpha=0.05):
    """Holm step-down: returns list of booleans (rejected)."""
    m = len(pvals)
    order = sorted(range(m), key=lambda i: pvals[i])
    rejected = [False] * m
    for rank, idx in enumerate(order):
        if pvals[idx] <= alpha / (m - rank):
            rejected[idx] = True
        else:
            break
    return rejected

def friedman(cols):
    """Friedman test over n blocks x k treatments -> (chi2, p). cols is a
    dict arm -> list of n values (same order)."""
    k = len(cols); arms = list(cols)
    n = len(cols[arms[0]])
    Rsum = {a: 0.0 for a in arms}
    for i in range(n):
        vals = sorted((cols[a][i], a) for a in arms)
        j = 0
        while j < k:
            l = j
            while l < k and vals[l][0] == vals[j][0]:
                l += 1
            avg = (j + 1 + l) / 2.0
            for t in range(j, l): Rsum[vals[t][1]] += avg
            j = l
    chi2 = 12.0 * n / (k * (k + 1)) * sum(
        (Rsum[a] / n - (k + 1) / 2.0) ** 2 for a in arms)
    # k=3 -> chi-square with 2 df: sf(x) = exp(-x/2)
    p = math.exp(-chi2 / 2.0) if k == 3 else float("nan")
    return chi2, p, {a: Rsum[a] / n for a in arms}

def group_of(stem):
    if stem.startswith("F0.15.0."):
        return "classical"
    return re.match(r"(tai\d+_\d+)", stem).group(1)

ORDER = ["classical", "tai15_15", "tai20_15", "tai20_20",
         "tai30_15", "tai30_20", "tai50_15", "tai50_20"]

# discover 82 stems from LADDER dirs
stems = sorted(d for d in os.listdir(os.path.join(BASE, "LADDER"))
               if os.path.isdir(os.path.join(BASE, "LADDER", d)))

rows = []           # (stem, group, ratio{}, winner, pLA, pLM, medHV{}, hv{})
for stem in stems:
    fr = {a: per_run_fronts(a, stem) for a in ARMS}
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
    hv = {}
    for a in ARMS:
        keys = sorted(fr[a], key=lambda k: (len(k), k))
        hv[a] = [hypervolume(fr[a][k], ref) / refhv for k in keys]
    pLA = mwu(hv["LADDER"], hv["ABC-P"])
    pLM = mwu(hv["LADDER"], hv["MA-P"])
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

# ---------- omnibus Friedman + post-hoc ----------
print("\n" + "=" * 72)
print("FRIEDMAN (n=82 bloques, k=3 brazos, sobre mediana HV por-run)")
print("=" * 72)
cols = {a: [r[6][a] for r in rows] for a in ARMS}
chi2, pF, meanrank = friedman(cols)
print(f"chi2_F = {chi2:.2f}  p = {pF:.3e}")
print("rangos medios: " + "  ".join(f"{a}={meanrank[a]:.3f}" for a in ARMS))
print("\npost-hoc pareado por instancia (signed-rank sobre medianas, "
      "Holm sobre las 3 comparaciones):")
pairs = [("LADDER", "ABC-P"), ("LADDER", "MA-P"), ("ABC-P", "MA-P")]
praw = [signed_rank(cols[a], cols[b]) for a, b in pairs]
rej = holm(praw)
for (a, b), p, r in zip(pairs, praw, rej):
    print(f"  {a} vs {b}: p = {p:.3e}  {'SIGNIFICATIVO' if r else 'n.s.'}")

# ---------- per-instance MWU + Holm ----------
print("\n" + "=" * 72)
print("POR INSTANCIA: Mann-Whitney U no pareado (30 vs 30 HV por-run),")
print("Holm sobre las 82 instancias por comparacion (alpha=0.05)")
print("=" * 72)
rejLA = holm([r[4] for r in rows])
rejLM = holm([r[5] for r in rows])
def summ(gr, idxs):
    la = sum(1 for i in idxs if rows[i][6]['LADDER'] > rows[i][6]['ABC-P']
             and rejLA[i])
    lm = sum(1 for i in idxs if rows[i][6]['LADDER'] > rows[i][6]['MA-P']
             and rejLM[i])
    loss = sum(1 for i in idxs if
               (rows[i][6]['ABC-P'] > rows[i][6]['LADDER'] and rejLA[i]) or
               (rows[i][6]['MA-P'] > rows[i][6]['LADDER'] and rejLM[i]))
    return la, lm, loss
print(f"{'grupo':12} {'#':>3} | {'L>ABC-P':>8} {'L>MA-P':>8} {'derrotas L':>11}")
for g in ORDER:
    idxs = [i for i, r in enumerate(rows) if r[1] == g]
    if not idxs: continue
    la, lm, loss = summ(rows, idxs)
    print(f"{g:12} {len(idxs):>3} | {la:>8} {lm:>8} {loss:>11}")
la, lm, loss = summ(rows, list(range(len(rows))))
print("-" * 48)
print(f"{'GLOBAL 82':12} {len(rows):>3} | {la:>8} {lm:>8} {loss:>11}")

pooled = {a: [x for r in rows for x in r[7][a]] for a in ARMS}
print("\nMediana HV por-run (pooled 82x30): "
      + "  ".join(f"{a}={st.median(pooled[a]):.3f}" for a in ARMS))
