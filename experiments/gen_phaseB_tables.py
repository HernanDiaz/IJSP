#!/usr/bin/env python3
"""stats_phaseB_corrected.py - Friedman + pairwise Wilcoxon signed-rank
with Holm-Bonferroni correction over the corrected Phase B dataset.

Reads:  statistical_results_exp4/runs_data.csv
Writes: statistical_results_exp4/tab4_wilcoxon.tex
        statistical_results_exp4/tab_phaseb_groups.tex
        statistical_results_exp4/wilcoxon_table.csv
        statistical_results_exp4/friedman.txt

Mirrors the existing paper Table 4 schema:
  A | B | Mean A | Mean B | Diff | p_adj | r | Magnitude
"""
import csv, math, statistics
from pathlib import Path
from collections import defaultdict
import os

_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_WSL if os.path.isdir(_WSL) else _WIN)
OUT  = ROOT / "experiments" / "statistical_results_exp4"

NBS = ["n1","n2","n3","next","n8"]
NB_LABEL = {"n1": r"$N_1$", "n2": r"$N_2$", "n3": r"$N_3$",
            "next": r"$N_{\rm ext}$", "n8": r"$N_8$"}
# Lower bounds (for per-class RE table)
CLASSICAL_LB = {"abz7":656,"abz8":645,"abz9":661,"ft10":930,"ft20":1165,
                "la21":1046,"la24":935,"la25":977,"la27":1235,"la29":1152,
                "la38":1196,"la40":1222}
TAILLARD_LB = {
    "tai15_15":[1231,1244,1218,1175,1224,1238,1227,1217,1274,1241],
    "tai20_15":[1357,1367,1342,1345,1339,1360,1462,1377,1332,1348],
    "tai20_20":[1642,1561,1518,1644,1558,1591,1652,1603,1583,1528],
    "tai30_15":[1764,1774,1788,1828,2007,1819,1771,1673,1795,1651],
    "tai30_20":[1906,1884,1809,1948,1997,1957,1807,1912,1931,1833],
    "tai50_15":[2760,2756,2717,2839,2679,2781,2943,2885,2655,2723],
    "tai50_20":[2868,2869,2755,2702,2725,2845,2825,2784,3071,2995],
}
import re
def lb_of(stem):
    if stem.startswith("F0.15.0."):
        m = re.match(r"F0\.15\.0\.([a-z]+[0-9]+)_", stem)
        return CLASSICAL_LB.get(m.group(1)) if m else None
    m = re.match(r"^(tai\d+_\d+)_(\d+)", stem)
    return TAILLARD_LB[m.group(1)][int(m.group(2))-1] if m else None

def fine_group_of(stem):
    if stem.startswith("F0.15.0."):
        m = re.search(r"\.(abz\d+|ft\d+|la\d+)_", stem)
        return m.group(1) if m else None
    m = re.match(r"^(tai\d+_\d+)_", stem)
    return m.group(1) if m else None


# --- normal CDF for p-value (no scipy dep) ---
def normal_sf(z):
    # survival function = 1 - Phi(z), two-sided p-value: 2 * sf(|z|)
    return 0.5 * math.erfc(z / math.sqrt(2))

def wilcoxon(diffs):
    """Wilcoxon signed-rank.  Returns (W, n, Z, p_two_sided, r_effect, sign)."""
    diffs = [d for d in diffs if d != 0]
    n = len(diffs)
    if n == 0: return 0, 0, 0.0, 1.0, 0.0, 0
    abs_pairs = sorted([(abs(d), i, d) for i, d in enumerate(diffs)])
    ranks = [0.0]*n
    i = 0
    while i < n:
        j = i
        while j+1 < n and abs_pairs[j+1][0] == abs_pairs[i][0]:
            j += 1
        ar = (i + j + 2) / 2.0
        for k in range(i, j+1):
            ranks[abs_pairs[k][1]] = ar
        i = j + 1
    Wp = sum(r for r, d in zip(ranks, diffs) if d > 0)
    Wn = sum(r for r, d in zip(ranks, diffs) if d < 0)
    W = min(Wp, Wn)
    mean_W = n*(n+1)/4.0
    sd_W   = (n*(n+1)*(2*n+1)/24.0)**0.5
    # Normal approximation with continuity correction
    Z = (W - mean_W + 0.5) / sd_W if sd_W else 0
    p = 2.0 * normal_sf(abs(Z))
    r_eff = abs(Z) / (n**0.5)
    sign = 1 if Wp > Wn else (-1 if Wn > Wp else 0)
    return W, n, Z, p, r_eff, sign


def magnitude(r):
    a = abs(r)
    if a < 0.10: return "negligible"
    if a < 0.30: return "small"
    if a < 0.50: return "medium"
    return "large"


def holm_bonferroni(pvals):
    """Returns adjusted p-values in the same order as input."""
    m = len(pvals)
    order = sorted(range(m), key=lambda i: pvals[i])
    adj = [0.0]*m
    running = 0.0
    for rank, idx in enumerate(order):
        candidate = (m - rank) * pvals[idx]
        running = max(running, candidate)
        adj[idx] = min(1.0, running)
    return adj


# ====== Load runs_data.csv ======
runs = []
with (OUT / "runs_data.csv").open() as fh:
    rdr = csv.DictReader(fh)
    for row in rdr:
        runs.append({
            "neigh":    row["neigh"],
            "instance": row["instance"],
            "run":      int(row["run"]),
            "midpoint": float(row["midpoint"]),
        })

# index midpoint by (nb, stem, run)
mid_idx = defaultdict(dict)
for r in runs:
    mid_idx[r["neigh"]][(r["instance"], r["run"])] = r["midpoint"]

stems = sorted({r["instance"] for r in runs})
all_blocks = sorted({(r["instance"], r["run"]) for r in runs})
print(f"{len(stems)} instances, {len(all_blocks)} (instance,run) blocks, {len(runs)} total rows")

# ====== Friedman test (over per-(instance,run) blocks - matches paper) ======
inst_means = {nb: {} for nb in NBS}
for nb in NBS:
    by_inst = defaultdict(list)
    for r in runs:
        if r["neigh"] != nb: continue
        by_inst[r["instance"]].append(r["midpoint"])
    for s, v in by_inst.items():
        inst_means[nb][s] = statistics.mean(v)

# Build the matrix: each row = (instance, run), each col = NB
matrix = []
for (s, r_idx) in all_blocks:
    row = [mid_idx[nb].get((s, r_idx)) for nb in NBS]
    if any(v is None for v in row):
        continue
    matrix.append(row)

# Friedman: rank within each row, then test
k = len(NBS); N = len(matrix)
rank_sums = [0.0]*k
for row in matrix:
    # rank ascending: lowest midpoint gets rank 1
    order = sorted(range(k), key=lambda i: row[i])
    ranks = [0.0]*k
    i = 0
    while i < k:
        j = i
        while j+1 < k and row[order[j+1]] == row[order[i]]:
            j += 1
        avg_rank = (i + j + 2)/2.0
        for x in range(i, j+1):
            ranks[order[x]] = avg_rank
        i = j+1
    for c in range(k):
        rank_sums[c] += ranks[c]

mean_ranks = [rs / N for rs in rank_sums]
chi_sq = (12.0/(N*k*(k+1))) * sum(rs**2 for rs in rank_sums) - 3*N*(k+1)
# df = k-1 = 4. p-value approximated via incomplete gamma (no scipy)
def chi_sq_p(chi, df):
    # use gamma upper-tail series
    # P(X > chi) for chi-square df=4: 1 - regularized lower incomplete gamma(2, chi/2)
    # for df=4: SF = (1 + chi/2) * exp(-chi/2)
    if df != 4:
        return float("nan")
    x = chi / 2.0
    return (1 + x) * math.exp(-x)

p_fried = chi_sq_p(chi_sq, k-1)

print(f"\nFriedman: chi^2({k-1}) = {chi_sq:.2f}, p = {p_fried:.3e}")
for nb, mr in zip(NBS, mean_ranks):
    print(f"  {nb}: mean rank = {mr:.3f}")

# Save friedman summary
with (OUT / "friedman.txt").open("w") as fh:
    fh.write(f"Friedman test (per-instance mean midpoint, n_inst={N}, k={k} neighbourhoods)\n")
    fh.write(f"  chi^2({k-1}) = {chi_sq:.4f}\n")
    fh.write(f"  p = {p_fried:.6e}\n")
    fh.write(f"\nMean ranks (ascending = better):\n")
    sorted_idx = sorted(range(k), key=lambda i: mean_ranks[i])
    for i in sorted_idx:
        fh.write(f"  {NBS[i]}: {mean_ranks[i]:.4f}\n")

# ====== Pairwise Wilcoxon (per-(instance,run) blocks) ======
pairs = []
raw_pvals = []
for i in range(len(NBS)):
    for j in range(i+1, len(NBS)):
        nbA, nbB = NBS[i], NBS[j]
        diffs = []
        for (s, r) in all_blocks:
            a = mid_idx[nbA].get((s, r))
            b = mid_idx[nbB].get((s, r))
            if a is None or b is None: continue
            diffs.append(a - b)
        meanA = statistics.mean([mid_idx[nbA][(s, r)] for (s, r) in all_blocks if (s, r) in mid_idx[nbA]])
        meanB = statistics.mean([mid_idx[nbB][(s, r)] for (s, r) in all_blocks if (s, r) in mid_idx[nbB]])
        W, n, Z, p, r_eff, sign = wilcoxon(diffs)
        pairs.append({
            "A": nbA, "B": nbB,
            "meanA": meanA, "meanB": meanB,
            "diff": meanA - meanB,
            "W": W, "n": n, "Z": Z, "p": p, "r": r_eff,
            "sign": sign, "magnitude": magnitude(r_eff),
        })
        raw_pvals.append(p)

adj_p = holm_bonferroni(raw_pvals)
for pi, ap in zip(pairs, adj_p):
    pi["p_adj"] = ap

# Sort by descending |r| (effect size) — for nicer table presentation
pairs_sorted = sorted(pairs, key=lambda p: -p["r"])

# ====== Write Wilcoxon CSV ======
with (OUT / "wilcoxon_table.csv").open("w", newline="") as fh:
    w = csv.writer(fh)
    w.writerow(["A","B","meanA","meanB","diff","p_raw","p_adj","r","Z","n","magnitude","winner"])
    for p in pairs_sorted:
        winner = p["A"] if p["sign"] > 0 else (p["B"] if p["sign"] < 0 else "tie")
        # Note: 'A better' if A's midpoint is lower (sign of diff)
        w.writerow([p["A"], p["B"], f"{p['meanA']:.2f}", f"{p['meanB']:.2f}",
                    f"{p['diff']:+.2f}", f"{p['p']:.3e}", f"{p['p_adj']:.3e}",
                    f"{p['r']:.4f}", f"{p['Z']:.2f}", p["n"], p["magnitude"], winner])
print(f"\nWilcoxon pairwise written ({len(pairs)} pairs).")
for p in pairs_sorted:
    print(f"  {p['A']} vs {p['B']}: r={p['r']:.3f}  Z={p['Z']:+.2f}  p_adj={p['p_adj']:.2e}  ({p['magnitude']})")

# ====== Generate Table 4 LaTeX ======
def fmt_p(p_adj):
    if p_adj == 0.0:
        return r"$<10^{-300}$"
    if p_adj < 1e-3:
        # show as "<10^{exp}" if very small
        log_p = math.log10(p_adj)
        if log_p < -300: return r"$<10^{-300}$"
        return f"$<10^{{{int(math.ceil(log_p))}}}$"
    return f"${p_adj:.3f}$"

def fmt_p_paper(p_adj):
    """Match paper format: '$<10^{-N}$' for very small, '$0.001$' otherwise."""
    if p_adj <= 0.0 or p_adj < 1e-300:
        return r"$<10^{-300}$"
    log_p = math.log10(p_adj)
    if log_p < -3:
        # ceil toward zero for a representable upper bound
        return f"$<10^{{{int(math.ceil(log_p))}}}$"
    return f"${p_adj:.3f}$"

with (OUT / "tab4_wilcoxon.tex").open("w") as fh:
    fh.write("% Auto-generated by stats_phaseB_corrected.py\n")
    fh.write(r"\begin{table}[t]" + "\n")
    fh.write(r"\centering" + "\n")
    fh.write(r"\caption{Phase B: pairwise Wilcoxon tests for tuned neighbourhoods ($n = 2460$ paired blocks, Holm--Bonferroni corrected). All differences are statistically significant; for the three $N_3$ comparisons the $p$-value underflows double precision, so a representable bound is shown.}" + "\n")
    fh.write(r"\label{tab4}" + "\n")
    fh.write(r"\begin{tabular}{l l c c c c c l}" + "\n")
    fh.write(r"\hline" + "\n")
    fh.write(r"A & B & Mean A & Mean B & Diff & $p_{\rm adj}$ & $r$ & Magnitude \\" + "\n")
    fh.write(r"\hline" + "\n")
    for p in pairs_sorted:
        A = NB_LABEL[p["A"]]; B = NB_LABEL[p["B"]]
        fh.write(f"{A:<13} & {B:<13} & {p['meanA']:.1f} & {p['meanB']:.1f} & "
                 f"${p['diff']:+.1f}$ & {fmt_p_paper(p['p_adj'])} & "
                 f"{p['r']:.3f} & {p['magnitude']} \\\\\n")
    fh.write(r"\hline" + "\n")
    fh.write(r"\end{tabular}" + "\n")
    fh.write(r"\end{table}" + "\n")
print(f"\nWrote tab4_wilcoxon.tex")

# ====== Generate tab_phaseb_groups.tex (Table 6 equivalent) ======
# Group order (rows) and per-NB cells (mean RE, median runtime)
# Build per-(stem, nb) RE and group medians
# Read per_instance_stats.csv for med_rt
per_inst = {}
with (OUT / "per_instance_stats.csv").open() as fh:
    rdr = csv.DictReader(fh)
    for row in rdr:
        per_inst[(row["neigh"], row["instance"])] = row

# Group RE = average over runs in that group; median runtime = median over instance runtimes
GROUP_ORDER = ["Classical","tai15_15","tai20_15","tai20_20","tai30_15","tai30_20","tai50_15","tai50_20"]
def collapse(stem): return "Classical" if stem.startswith("F0.15.0.") else fine_group_of(stem)

group_runs_re = defaultdict(lambda: defaultdict(list))
group_runtimes = defaultdict(lambda: defaultdict(list))
for r in runs:
    g = collapse(r["instance"])
    lb = lb_of(r["instance"])
    if lb is None: continue
    group_runs_re[g][r["neigh"]].append((r["midpoint"] - lb)/lb * 100.0)
for (nb, stem), row in per_inst.items():
    g = collapse(stem)
    if row["med_rt"] != "NA":
        group_runtimes[g][nb].append(float(row["med_rt"]))

group_labels = {"Classical":"Classical",
                "tai15_15": r"$15{\times}15$",
                "tai20_15": r"$20{\times}15$",
                "tai20_20": r"$20{\times}20$",
                "tai30_15": r"$30{\times}15$",
                "tai30_20": r"$30{\times}20$",
                "tai50_15": r"$50{\times}15$",
                "tai50_20": r"$50{\times}20$"}
group_n = {"Classical": 12, "tai15_15": 10, "tai20_15": 10, "tai20_20": 10,
           "tai30_15": 10, "tai30_20": 10, "tai50_15": 10, "tai50_20": 10}

# Column order in paper: N_2 | N_8 | N_ext | N_1 | N_3
COL_ORDER = ["n2","n8","next","n1","n3"]
COL_LABEL = ["$N_2$", "$N_8$", "$N_{\\rm ext}$", "$N_1$", "$N_3$"]

with (OUT / "tab_phaseb_groups.tex").open("w") as fh:
    fh.write("% Auto-generated by stats_phaseB_corrected.py\n")
    fh.write(r"\begin{table*}[t]" + "\n")
    fh.write(r"\centering\small\setlength{\tabcolsep}{4pt}" + "\n")
    fh.write(r"\caption{Phase~B: mean RE\,(\%) and median runtime $t$\,(s) per instance group and neighbourhood (irace-tuned TS, 30 runs per instance). Bold: minimum RE\,(\%) in each row. The Classical row aggregates the 12 OR-Library instances; Taillard rows aggregate the 10 instances of each size class. Per-instance results are provided as supplementary material~\cite{DiazSupp2026}.}" + "\n")
    fh.write(r"\label{tab:phaseb_groups}" + "\n")
    fh.write(r"\begin{tabular}{l r | rr | rr | rr | rr | rr}" + "\n")
    fh.write(r"\hline" + "\n")
    fh.write(r" &  & " + " & ".join(f"\\multicolumn{{2}}{{c{('|' if i<4 else '')}}}{{{L}}}" for i, L in enumerate(COL_LABEL)) + r" \\" + "\n")
    fh.write(r"Group & $n$ & " + " & ".join("RE & $t$" for _ in COL_ORDER) + r" \\" + "\n")
    fh.write(r"\hline" + "\n")
    grand_re = defaultdict(list)
    grand_rt = defaultdict(list)
    for g in GROUP_ORDER:
        if g not in group_runs_re: continue
        cells = []
        # Determine winner (min RE) for bolding
        re_per_nb = {nb: statistics.mean(group_runs_re[g][nb]) for nb in COL_ORDER if group_runs_re[g][nb]}
        winner = min(re_per_nb, key=re_per_nb.get) if re_per_nb else None
        for nb in COL_ORDER:
            re_vals = group_runs_re[g].get(nb, [])
            rt_vals = group_runtimes[g].get(nb, [])
            if re_vals:
                re_str = f"{statistics.mean(re_vals):.2f}"
                if nb == winner: re_str = r"\textbf{" + re_str + "}"
                grand_re[nb].extend(re_vals)
            else:
                re_str = "--"
            if rt_vals:
                rt_str = f"{int(round(statistics.median(rt_vals)))}"
                grand_rt[nb].extend(rt_vals)
            else:
                rt_str = "--"
            cells.append(f"{re_str} & {rt_str}")
        fh.write(f"{group_labels[g]:<24} & {group_n[g]} & " + " & ".join(cells) + r" \\" + "\n")
    fh.write(r"\hline" + "\n")
    gm_cells = []
    gm_per_nb = {nb: statistics.mean(grand_re[nb]) for nb in COL_ORDER}
    gm_winner = min(gm_per_nb, key=gm_per_nb.get)
    for nb in COL_ORDER:
        gm_re = statistics.mean(grand_re[nb])
        gm_rt = int(round(statistics.median(grand_rt[nb]))) if grand_rt[nb] else "--"
        re_str = f"{gm_re:.2f}"
        if nb == gm_winner: re_str = r"\textbf{" + re_str + "}"
        gm_cells.append(f"{re_str} & {gm_rt}")
    fh.write(r"\textit{Grand mean} &    & " + " & ".join(gm_cells) + r" \\" + "\n")
    fh.write(r"\hline" + "\n")
    fh.write(r"\end{tabular}" + "\n")
    fh.write(r"\end{table*}" + "\n")
print(f"Wrote tab_phaseb_groups.tex")

# Per-instance wins for the prose
print("\n=== Per-instance wins (mean midpoint over 30 runs) ===")
wins = {nb: 0 for nb in NBS}
ties = 0
for s in stems:
    means = {nb: inst_means[nb][s] for nb in NBS}
    best = min(means.values())
    winners = [nb for nb in NBS if means[nb] == best]
    if len(winners) == 1:
        wins[winners[0]] += 1
    else:
        ties += 1
        for w in winners: wins[w] += 1.0/len(winners)
for nb in NBS:
    print(f"  {nb}: {wins[nb]} wins")
print(f"  Total ties: {ties}")
