#!/usr/bin/env python3
"""
analyze.py — Aggregate N2Plus / N2Minus runs and compare to the published N2.

Extracts the per-run makespan midpoints (E[Cmax]) and runtimes from every
FuzzyFW result CSV (dedup by latest timestamp per instance), computes
RE(%) = (E[Cmax]-LB)/LB*100 against the paper lower bounds, and aggregates by
size class exactly like tab_phaseb_groups. Published N2 is recomputed from the
per-run midpoints in cor_tabu_2026 runs_data.csv (sanity: should match the paper).
Per-group paired Wilcoxon (per-instance avg-RE) of N2Plus-vs-N2 and N2Minus-vs-N2.

Pure stdlib. Works on partial data. Run:  python3 analyze.py
"""
import csv, glob, math, os, re, statistics as st
from collections import defaultdict

HERE = os.path.dirname(os.path.abspath(__file__))
RESULTS = os.path.join(HERE, "results")
BASELINE = os.path.join(HERE, "..", "cor_tabu_2026", "statistical_results", "runs_data.csv")

# --- Lower bounds -----------------------------------------------------------
LB = {  # classical (from tab_lit_classical.tex / DiazFEABC2023)
    "F0.15.0.abz7_06": 656, "F0.15.0.abz8_05": 645, "F0.15.0.abz9_10": 661,
    "F0.15.0.ft10_10": 930, "F0.15.0.ft20_05": 1165,
    "F0.15.0.la21_04": 1046, "F0.15.0.la24_03": 935, "F0.15.0.la25_04": 977,
    "F0.15.0.la27_09": 1235, "F0.15.0.la29_03": 1152, "F0.15.0.la38_06": 1196,
    "F0.15.0.la40_05": 1222,
}
# Taillard crisp LBs TA1..TA70 (from supp_taillard_tables.tex).
TA_LB = [
    1231,1244,1218,1175,1224,1238,1227,1217,1274,1241,
    1357,1367,1342,1345,1339,1360,1462,1377,1332,1348,
    1642,1561,1518,1644,1558,1591,1652,1603,1583,1528,
    1764,1774,1788,1828,2007,1819,1771,1673,1795,1651,
    1906,1884,1809,1948,1997,1957,1807,1912,1931,1833,
    2760,2756,2717,2839,2679,2781,2943,2885,2655,2723,
    2868,2869,2755,2702,2725,2845,2825,2784,3071,2995,
]
# size class -> (filename prefix, TA offset). TA(offset+nn) -> <prefix>_<nn>.F.15_01
SIZE_CLASSES = [
    ("15x15", "tai15_15", 0), ("20x15", "tai20_15", 10), ("20x20", "tai20_20", 20),
    ("30x15", "tai30_15", 30), ("30x20", "tai30_20", 40), ("50x15", "tai50_15", 50),
    ("50x20", "tai50_20", 60),
]
GROUPS = [("Classical", sorted(k for k in LB))]  # classical stems already in LB
for name, prefix, off in SIZE_CLASSES:
    stems = []
    for nn in range(1, 11):
        stem = f"{prefix}_{nn:02d}.F.15_01"
        LB[stem] = TA_LB[off + nn - 1]
        stems.append(stem)
    GROUPS.append((name, stems))

# Published N2 per-group mean RE and median runtime (tab_phaseb_groups.tex).
PAPER_N2 = {"Classical": 2.40, "15x15": 2.04, "20x15": 3.58, "20x20": 5.34,
            "30x15": 3.68, "30x20": 8.62, "50x15": 0.32, "50x20": 2.03}
PAPER_N2_T = {"Classical": 18, "15x15": 19, "20x15": 42, "20x20": 58,
              "30x15": 103, "30x20": 161, "50x15": 173, "50x20": 535}

STEM_RE = re.compile(r"^(.+)_(\d{14})\.csv$")


def parse_csv(path):
    mids, times = [], []
    with open(path, newline="", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            f = line.rstrip("\n").split(";")
            if f and f[0] == "Best solution":
                mids = [float(x) for x in f[5:] if x not in ("", None)]
            elif f and f[0] == "Total runtime":
                times = [float(x) for x in f[5:] if x not in ("", None)]
    return mids, times


def load_config(cfg):
    latest = {}
    for p in glob.glob(os.path.join(RESULTS, cfg, "*.csv")):
        base = os.path.basename(p)
        if re.search(r"_(Sols|Robustness|Scenarios)\.csv$", base):
            continue
        m = STEM_RE.match(base)
        if not m:
            continue
        stem, ts = m.group(1), m.group(2)
        if stem not in latest or ts > latest[stem][0]:
            latest[stem] = (ts, p)
    return {stem: parse_csv(p) for stem, (_, p) in latest.items()}


def load_baseline_n2():
    d = defaultdict(list)
    with open(BASELINE, newline="", encoding="utf-8") as fh:
        for row in csv.DictReader(fh):
            if row.get("neigh") == "n2":
                d[row["instance"]].append(float(row["midpoint"]))
    return d


def wilcoxon(a, b):
    diffs = [x - y for x, y in zip(a, b) if x - y != 0]
    n = len(diffs)
    if n == 0:
        return 1.0
    order = sorted(range(n), key=lambda i: abs(diffs[i]))
    ranks = [0.0] * n
    i = 0
    while i < n:
        j = i
        while j + 1 < n and abs(diffs[order[j + 1]]) == abs(diffs[order[i]]):
            j += 1
        avg = (i + j) / 2.0 + 1
        for k in range(i, j + 1):
            ranks[order[k]] = avg
        i = j + 1
    Wp = sum(r for d, r in zip(diffs, ranks) if d > 0)
    Wm = sum(r for d, r in zip(diffs, ranks) if d < 0)
    W = min(Wp, Wm)
    mean = n * (n + 1) / 4.0
    var = n * (n + 1) * (2 * n + 1) / 24.0
    if var <= 0:
        return 1.0
    z = (W - mean + 0.5) / math.sqrt(var)
    return 2 * (1 - 0.5 * (1 + math.erf(abs(z) / math.sqrt(2))))


def avg_re(stem, mids):
    return (st.mean(mids) - LB[stem]) / LB[stem] * 100


def main():
    plus = load_config("N2Plus")
    minus = load_config("N2Minus")
    n2 = load_baseline_n2()

    print("=" * 92)
    print("N2 worst-case — mean RE(%) by size class (lower better).  N2 recomputed from paper data.")
    print("=" * 92)
    print(f"{'Group':10} {'n':>3} | {'N2(paper)':>10} {'N2 calc':>8} | {'N2Plus':>8} {'p+':>7} | {'N2Minus':>8} {'p-':>7} | {'t+ ':>6} {'t- ':>6}")
    print("-" * 92)

    grand = defaultdict(list)
    tp_all, tm_all = [], []          # per-instance median runtimes (for paired test)
    tgroups = []                     # (gname, n, paper_t, mean_tp, mean_tm)
    for gname, stems in GROUPS:
        done = [s for s in stems if s in plus and s in minus and s in n2]
        if not done:
            print(f"{gname:10} {0:3d} | {PAPER_N2.get(gname, float('nan')):10.2f} {'--':>8} | {'--':>8} {'':>7} | {'--':>8} {'':>7} |")
            continue
        re_n2 = [avg_re(s, n2[s]) for s in done]
        re_p = [avg_re(s, plus[s][0]) for s in done]
        re_m = [avg_re(s, minus[s][0]) for s in done]
        tp = [st.median(plus[s][1]) for s in done]
        tm = [st.median(minus[s][1]) for s in done]
        pp = wilcoxon(re_p, re_n2)
        pm = wilcoxon(re_m, re_n2)
        grand["N2"] += re_n2; grand["N2Plus"] += re_p; grand["N2Minus"] += re_m
        tp_all += tp; tm_all += tm
        tgroups.append((gname, len(done), PAPER_N2_T.get(gname, float("nan")), st.mean(tp), st.mean(tm)))
        mark = "" if len(done) == len(stems) else f"*{len(done)}/{len(stems)}"
        print(f"{gname:10} {len(done):3d} | {PAPER_N2.get(gname, float('nan')):10.2f} {st.mean(re_n2):8.2f} | "
              f"{st.mean(re_p):8.2f} {pp:7.3f} | {st.mean(re_m):8.2f} {pm:7.3f} | {st.mean(tp):6.1f} {st.mean(tm):6.1f}  {mark}")

    print("-" * 92)
    if grand["N2"]:
        gp = wilcoxon(grand["N2Plus"], grand["N2"])
        gm = wilcoxon(grand["N2Minus"], grand["N2"])
        print(f"{'GRAND':10} {len(grand['N2']):3d} | {3.47:10.2f} {st.mean(grand['N2']):8.2f} | "
              f"{st.mean(grand['N2Plus']):8.2f} {gp:7.3f} | {st.mean(grand['N2Minus']):8.2f} {gm:7.3f} | "
              f"{st.mean(tp_all):6.1f} {st.mean(tm_all):6.1f}")
    print("\np+ = Wilcoxon N2Plus-vs-N2 (per-instance avg-RE);  p- = N2Minus-vs-N2.")
    print("'*k/10' = group still partial.")

    # ---- Runtime comparison -------------------------------------------------
    print("\n" + "=" * 92)
    print("RUNTIME  (mean of per-instance median runtime, s).  N2 = published (indicative, other run).")
    print("=" * 92)
    print(f"{'Group':10} {'n':>3} | {'N2(paper)':>10} | {'N2Plus':>8} {'vsN2%':>7} | {'N2Minus':>8} {'vsN2%':>7} | {'+/- %':>7}")
    print("-" * 92)
    for gname, n, pt, mtp, mtm in tgroups:
        dp = (mtp - pt) / pt * 100 if pt == pt else float("nan")
        dm = (mtm - pt) / pt * 100 if pt == pt else float("nan")
        pm_diff = (mtp - mtm) / mtm * 100
        print(f"{gname:10} {n:3d} | {pt:10.0f} | {mtp:8.1f} {dp:+7.1f} | {mtm:8.1f} {dm:+7.1f} | {pm_diff:+7.1f}")
    print("-" * 92)
    if tp_all:
        p_time = wilcoxon(tp_all, tm_all)
        faster = sum(1 for a, b in zip(tp_all, tm_all) if a < b)
        print(f"{'GRAND':10} {len(tp_all):3d} | {'':>10} | {st.mean(tp_all):8.1f} {'':>7} | {st.mean(tm_all):8.1f} {'':>7} | "
              f"{(st.mean(tp_all)-st.mean(tm_all))/st.mean(tm_all)*100:+7.1f}")
        print(f"\nPaired Wilcoxon N2Plus-vs-N2Minus runtime (same machine, same load):")
        print(f"  N2Plus faster on {faster}/{len(tp_all)} instances,  mean {st.mean(tp_all):.1f}s vs {st.mean(tm_all):.1f}s,  p = {p_time:.4f}")
        print("  (+/- % = N2Plus relative to N2Minus; negative = N2Plus faster.)")


if __name__ == "__main__":
    main()
