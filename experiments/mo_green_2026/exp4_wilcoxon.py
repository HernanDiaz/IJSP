#!/usr/bin/env python3
"""
exp4_wilcoxon.py — per-run hypervolume + paired Wilcoxon for one Exp.4 tier.

Per instance:
  * fixed reference point = 1.05x the nadir over ALL points of ALL arms and
    runs, so per-run HV values are comparable; HV normalised by the
    instance's union-front HV (so ~[0,1], 1 = matches combined best).
  * per arm, one HV per run (LADDER run = merged anchor+levels of that
    pipeline dir r1..r30; baseline run = its single _Front.csv).
  * paired Wilcoxon signed-rank (normal approx, tie/zero corrected),
    LADDER-vs-ABC-P and LADDER-vs-MA-P, pairing by run index.
Reports per-instance median HV per arm, both p-values, and a grand summary
(pooled per-run HV + significant wins/losses at alpha=0.05).
"""
import glob, math, os, re, statistics as st, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from front_metrics import load_front, load_exact, hypervolume, nondominated

HERE = os.path.dirname(os.path.abspath(__file__))
BASE = os.path.join(HERE, "results", "EXP4")
ARMS = ["LADDER", "ABC-P", "MA-P"]
STEMS = ["F0.15.0.abz7_06", "F0.15.0.abz8_05", "F0.15.0.abz9_10",
         "F0.15.0.ft10_10", "F0.15.0.ft20_05", "F0.15.0.la21_04",
         "F0.15.0.la24_03", "F0.15.0.la25_04", "F0.15.0.la27_09",
         "F0.15.0.la29_03", "F0.15.0.la38_06", "F0.15.0.la40_05"]
RUN_RE = re.compile(r"(?:^|/)r(\d+)(?:/|$)")
TS_RE = re.compile(r"_(\d{14})(\d+)?_Front\.csv$")


def per_run_fronts(arm, stem):
    """{run_key: [points]} for one arm/instance."""
    runs = {}
    # LADDER: pipeline dirs <stem>/r<k>/**
    for f in glob.glob(os.path.join(BASE, arm, stem, "**",
                                    stem + "_*_Front.csv"), recursive=True):
        m = RUN_RE.search(f.replace("\\", "/"))
        key = "r" + m.group(1) if m else f
        runs.setdefault(key, []).extend(load_front(f))
    # Baselines: flat, one _Front per run (timestamp+run suffix as key)
    for f in glob.glob(os.path.join(BASE, arm, stem + "_*_Front.csv")):
        m = TS_RE.search(os.path.basename(f))
        key = (m.group(1) + (m.group(2) or "")) if m else f
        runs.setdefault(key, []).extend(load_front(f))
    return {k: nondominated(v) for k, v in runs.items() if v}


def wilcoxon(a, b):
    """Paired signed-rank, normal approx, tie+continuity corrected -> p."""
    d = [x - y for x, y in zip(a, b) if x != y]
    n = len(d)
    if n == 0:
        return 1.0
    order = sorted(range(n), key=lambda i: abs(d[i]))
    rank = [0.0] * n
    i = 0
    while i < n:
        j = i
        while j + 1 < n and abs(d[order[j + 1]]) == abs(d[order[i]]):
            j += 1
        for k in range(i, j + 1):
            rank[order[k]] = (i + j) / 2.0 + 1
        i = j + 1
    Wp = sum(r for x, r in zip(d, rank) if x > 0)
    Wm = sum(r for x, r in zip(d, rank) if x < 0)
    W = min(Wp, Wm)
    mean = n * (n + 1) / 4.0
    var = n * (n + 1) * (2 * n + 1) / 24.0
    z = (W - mean + 0.5) / math.sqrt(var) if var > 0 else 0.0
    return 2 * (1 - 0.5 * (1 + math.erf(abs(z) / math.sqrt(2))))


def main():
    print(f"{'instance':18} {'n':>3} | {'LADDER':>7} {'ABC-P':>7} {'MA-P':>7}"
          f" | {'p:L-A':>7} {'p:L-M':>7}")
    print("-" * 66)
    pooled = {a: [] for a in ARMS}
    sig = {"LADDER>ABC-P": 0, "LADDER>MA-P": 0, "loss": 0}
    for stem in STEMS:
        fr = {a: per_run_fronts(a, stem) for a in ARMS}
        allpts = [p for a in ARMS for pts in fr[a].values() for p in pts]
        ex = load_exact(stem)
        refpts = allpts + (ex or [])
        if not refpts:
            print(f"{stem:18}  -- no data"); continue
        nad = (max(p[0] for p in refpts), max(p[1] for p in refpts))
        ref = (nad[0] * 1.05, nad[1] * 1.05)
        unionhv = hypervolume(nondominated(refpts), ref) or 1.0
        # per-run HV vectors, ordered by run index
        hv = {}
        for a in ARMS:
            keys = sorted(fr[a], key=lambda k: (len(k), k))
            hv[a] = [hypervolume(fr[a][k], ref) / unionhv for k in keys]
            pooled[a] += hv[a]
        n = min(len(hv[a]) for a in ARMS)
        med = {a: (st.median(hv[a]) if hv[a] else float('nan')) for a in ARMS}
        pLA = wilcoxon(hv["LADDER"][:n], hv["ABC-P"][:n])
        pLM = wilcoxon(hv["LADDER"][:n], hv["MA-P"][:n])
        if med["LADDER"] > med["ABC-P"] and pLA < 0.05:
            sig["LADDER>ABC-P"] += 1
        if med["LADDER"] > med["MA-P"] and pLM < 0.05:
            sig["LADDER>MA-P"] += 1
        if (med["ABC-P"] > med["LADDER"] and pLA < 0.05) or \
           (med["MA-P"] > med["LADDER"] and pLM < 0.05):
            sig["loss"] += 1
        print(f"{stem:18} {n:>3} | {med['LADDER']:>7.3f} {med['ABC-P']:>7.3f}"
              f" {med['MA-P']:>7.3f} | {pLA:>7.4f} {pLM:>7.4f}")
    print("-" * 66)
    print(f"{'MEDIAN (pooled)':18} {'':>3} | "
          + " ".join(f"{st.median(pooled[a]):>7.3f}" for a in ARMS))
    print(f"\nLADDER significantly > ABC-P (p<0.05): "
          f"{sig['LADDER>ABC-P']}/{len(STEMS)} instances")
    print(f"LADDER significantly > MA-P  (p<0.05): "
          f"{sig['LADDER>MA-P']}/{len(STEMS)} instances")
    print(f"instances where a baseline significantly beats LADDER: {sig['loss']}")


if __name__ == "__main__":
    main()
