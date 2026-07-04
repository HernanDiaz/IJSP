#!/usr/bin/env python3
"""
analyze_lexme.py — Phase 2a analysis: how much idle energy does the
lexicographic tie-break (Cmax -> NPE) recover "for free"?

Compares, per instance and per run:
- baseline: n2_worstcase_2026/results/N2Plus (makespan-only, same tuned
  config and LS) — (Cmax, NPE) recomputed uniformly from its _Sols orders.
- LexME:    mo_green_2026/results/LexME — same recomputation from its _Sols.

Both arms are decoded with the same semi-active component-wise decoder and
the same power data (Pp ~ U{2..8} seed 23 per instance), so the comparison
is apples-to-apples. Expectation: Cmax(LexME) ~= Cmax(baseline) (primary
objective untouched), NPE(LexME) < NPE(baseline).

Pure stdlib; works on partial data. Run: python3 analyze_lexme.py
"""
import glob, math, os, random, re, statistics as st, sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance, POWER_SEED
from validate_npe import decode_npe

# RS env var: "off" (default) | "heur" | "exact" — adds a right-shifted NPE
# column (timing re-optimised post-hoc per order at fixed Cmax). "exact"
# needs the OR-Tools venv; "heur" is the stdlib backward pass (near-optimal,
# within ~1% of exact in validation).
RS_MODE = os.environ.get("RS", "off")
if RS_MODE != "off":
    from rightshift import heuristic_rs_npe, exact_rs_npe

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
# Baseline arm = the PUBLISHED N2 raw results (exp4, old clone). Override with
# env BASE_DIR to compare against another arm (e.g. the unpublished N2Plus).
BASE_DIR = os.environ.get("BASE_DIR",
    r"C:\Users\diazhernan\CLionProjects\IJSP\experiments\results\exp4\N2_tuned"
    if os.name == "nt" else
    "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp4/N2_tuned")
LEX_DIR = os.environ.get("LEX_DIR", os.path.join(HERE, "results", "LexME"))

GROUPS = [
    ("Classical", re.compile(r"^F0\.15\.0\.")),
    ("15x15", re.compile(r"^tai15_15")), ("20x15", re.compile(r"^tai20_15")),
    ("20x20", re.compile(r"^tai20_20")), ("30x15", re.compile(r"^tai30_15")),
    ("30x20", re.compile(r"^tai30_20")), ("50x15", re.compile(r"^tai50_15")),
    ("50x20", re.compile(r"^tai50_20")),
]
STEM_RE = re.compile(r"^(.+)_(\d{14})_Sols\.csv$")


def load_sols_orders(directory):
    """{stem: [orders...]} from the latest _Sols per stem in a results dir."""
    latest = {}
    for p in glob.glob(os.path.join(directory, "*_Sols.csv")):
        m = STEM_RE.match(os.path.basename(p))
        if not m:
            continue
        stem, ts = m.group(1), m.group(2)
        if stem not in latest or ts > latest[stem][0]:
            latest[stem] = (ts, p)
    out = {}
    for stem, (_, p) in latest.items():
        orders = []
        with open(p, encoding="utf-8", errors="replace") as fh:
            for line in fh:
                parts = line.strip().split(";")
                if len(parts) >= 2 and parts[0].isdigit():
                    orders.append([int(x) for x in parts[1].split()])
        out[stem] = orders
    return out


def eval_orders(stem, orders):
    """[(cmax_mid, npe_mid[, npe_rs_mid]) per run] for one instance."""
    n, m, mach, dur = parse_instance(stem)
    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]
    pts = []
    for o in orders:
        if len(o) != n * m:
            continue
        cmax, npe = decode_npe(o, n, m, mach, dur, pp)
        row = [(cmax[0] + cmax[1]) / 2, (npe[0] + npe[1]) / 2]
        if RS_MODE == "heur":
            row.append((heuristic_rs_npe(o, n, m, mach, dur, pp, 0)[0]
                        + heuristic_rs_npe(o, n, m, mach, dur, pp, 1)[0]) / 2)
        elif RS_MODE == "exact":
            row.append((exact_rs_npe(o, n, m, mach, dur, pp, 0)[0]
                        + exact_rs_npe(o, n, m, mach, dur, pp, 1)[0]) / 2)
        pts.append(tuple(row))
    return pts


def wilcoxon(a, b):
    diffs = [x - y for x, y in zip(a, b) if x != y]
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
        for k in range(i, j + 1):
            ranks[order[k]] = (i + j) / 2.0 + 1
        i = j + 1
    Wp = sum(r for d, r in zip(diffs, ranks) if d > 0)
    Wm = sum(r for d, r in zip(diffs, ranks) if d < 0)
    W = min(Wp, Wm)
    mean, var = n * (n + 1) / 4.0, n * (n + 1) * (2 * n + 1) / 24.0
    z = (W - mean + 0.5) / math.sqrt(var)
    return 2 * (1 - 0.5 * (1 + math.erf(abs(z) / math.sqrt(2))))


def main():
    base = load_sols_orders(BASE_DIR)
    lex = load_sols_orders(LEX_DIR)
    common = sorted(set(base) & set(lex))
    if not common:
        print(f"No common instances yet. baseline={len(base)}  LexME={len(lex)}")
        return

    rs_on = RS_MODE != "off"
    print("=" * (128 if rs_on else 96))
    print("LexME (Cmax->NPE lexicographic) vs makespan-only N2 baseline — per-run means")
    print("Same decoder, same powers. dCmax% ~ 0 expected; dNPE% < 0 = energy recovered for free."
          + (f"  [RS={RS_MODE}: post-hoc timing]" if rs_on else ""))
    print("=" * (128 if rs_on else 96))
    hdr = (f"{'Group':10} {'n':>3} | {'Cmax base':>10} {'Cmax lex':>10} {'dCmax%':>7} {'p':>6} | "
           f"{'NPE base':>10} {'NPE lex':>10} {'dNPE%':>7} {'p':>6}")
    if rs_on:
        hdr += f" | {'RS base':>9} {'RS lex':>9} {'dRS%':>7} {'p':>6}"
    print(hdr)
    print("-" * len(hdr))

    g = defaultdict(list)
    for gname, rx in GROUPS:
        stems = [s for s in common if rx.match(s)]
        if not stems:
            continue
        cb, cl, nb, nl, rb, rl = [], [], [], [], [], []
        for s in stems:
            pb, pl = eval_orders(s, base[s]), eval_orders(s, lex[s])
            if not pb or not pl:
                continue
            cb.append(st.mean(p[0] for p in pb)); cl.append(st.mean(p[0] for p in pl))
            nb.append(st.mean(p[1] for p in pb)); nl.append(st.mean(p[1] for p in pl))
            if rs_on:
                rb.append(st.mean(p[2] for p in pb)); rl.append(st.mean(p[2] for p in pl))
        if not cb:
            continue
        dc = (st.mean(cl) - st.mean(cb)) / st.mean(cb) * 100
        dn = (st.mean(nl) - st.mean(nb)) / st.mean(nb) * 100
        pc, pn = wilcoxon(cl, cb), wilcoxon(nl, nb)
        for key, arr in (("cb", cb), ("cl", cl), ("nb", nb), ("nl", nl), ("rb", rb), ("rl", rl)):
            g[key] += arr
        line = (f"{gname:10} {len(cb):>3} | {st.mean(cb):10.1f} {st.mean(cl):10.1f} {dc:+7.2f} {pc:6.3f} | "
                f"{st.mean(nb):10.0f} {st.mean(nl):10.0f} {dn:+7.1f} {pn:6.3f}")
        if rs_on:
            dr = (st.mean(rl) - st.mean(rb)) / st.mean(rb) * 100
            line += f" | {st.mean(rb):9.0f} {st.mean(rl):9.0f} {dr:+7.1f} {wilcoxon(rl, rb):6.3f}"
        print(line)

    print("-" * len(hdr))
    dc = (st.mean(g['cl']) - st.mean(g['cb'])) / st.mean(g['cb']) * 100
    dn = (st.mean(g['nl']) - st.mean(g['nb'])) / st.mean(g['nb']) * 100
    line = (f"{'GRAND':10} {len(g['cb']):>3} | {st.mean(g['cb']):10.1f} {st.mean(g['cl']):10.1f} {dc:+7.2f} "
            f"{wilcoxon(g['cl'], g['cb']):6.3f} | {st.mean(g['nb']):10.0f} {st.mean(g['nl']):10.0f} {dn:+7.1f} "
            f"{wilcoxon(g['nl'], g['nb']):6.3f}")
    if rs_on:
        dr = (st.mean(g['rl']) - st.mean(g['rb'])) / st.mean(g['rb']) * 100
        line += f" | {st.mean(g['rb']):9.0f} {st.mean(g['rl']):9.0f} {dr:+7.1f} {wilcoxon(g['rl'], g['rb']):6.3f}"
    print(line)
    if rs_on:
        semi_vs_rs_b = (st.mean(g['rb']) - st.mean(g['nb'])) / st.mean(g['nb']) * 100
        semi_vs_rs_l = (st.mean(g['rl']) - st.mean(g['nl'])) / st.mean(g['nl']) * 100
        print(f"\nTiming value (RS vs semi-active NPE): baseline {semi_vs_rs_b:+.1f}%   LexME {semi_vs_rs_l:+.1f}%")


if __name__ == "__main__":
    main()
