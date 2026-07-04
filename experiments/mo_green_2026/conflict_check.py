#!/usr/bin/env python3
"""
conflict_check.py — Phase 0 sanity check for the green MO interval-JSP line.

Question: do makespan and passive/idle energy actually CONFLICT in the
interval JSP? If they are strongly correlated, a Pareto front degenerates and
the MO idea needs re-framing (cf. Afsar et al. 2022, Sec. 3.5 "Conflict
between the objectives", who ran this same check on the fuzzy JSP).

Energy model (from Afsar et al. 2024/2025, interval green FJSP, adapted to
non-flexible JSP): machine k is on from the start of its first task to the
completion of its last task; PE_k = Pp_k * (ct_last_k - st_first_k) with
passive power Pp_k. Active energy AE = sum Pa*p is ORDER-INVARIANT in the
non-flexible JSP (same tasks always on the same machines), so the meaningful
green objective reduces to total passive energy PE = sum_k PE_k (interval).

Data: needs NO solver runs. Reuses the 30+30 near-optimal schedules per
instance stored in n2_worstcase_2026 results (_Sols task orders, decoded
semi-actively component-wise) plus R random schedules for global spread.
Power coefficients: provisional documented scheme Pp_k ~ U{2..8}, seed 23.

Usage: python3 conflict_check.py <instance_stem> [more stems...]
   e.g. python3 conflict_check.py F0.15.0.ft10_10 F0.15.0.la29_03
"""
import glob, os, random, re, statistics as st, sys

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
INSTDIR = os.path.join(REPO, "SelectosYTaillardIntervalos")
SOLS_DIRS = [os.path.join(REPO, "experiments", "n2_worstcase_2026", "results", c)
             for c in ("N2Plus", "N2Minus")]
N_RANDOM = 200
POWER_SEED = 23


def parse_instance(stem):
    """Handles both formats found in SelectosYTaillardIntervalos:
    A) headered  (NUMERO DE TRABAJOS / ... / SECUENCIA / DURACIONES, '(a,b)')
    B) headerless (njobs, nmach, machine matrix, duration matrix, '(a, b)')."""
    path = os.path.join(INSTDIR, stem + ".txt")
    with open(path, encoding="utf-8", errors="replace") as fh:
        lines = [ln.strip() for ln in fh.read().splitlines() if ln.strip()]
    tup = re.compile(r"\((\d+)\s*,\s*(\d+)\)")

    if lines[0].upper().startswith("NUMERO"):          # format A
        idx = {ln.upper().split()[-1]: k for k, ln in enumerate(lines)
               if ln.upper().startswith(("NUMERO", "SECUENCIA", "DURACIONES"))}
        n = int(lines[idx["TRABAJOS"] + 1])
        m = int(lines[idx["RECURSOS"] + 1])
        s = idx["MAQUINAS"] + 1
        mach = [[int(x) for x in lines[s + j].split()] for j in range(n)]
        d = idx["DURACIONES"] + 1
    else:                                              # format B
        n, m = int(lines[0]), int(lines[1])
        mach = [[int(x) for x in lines[2 + j].split()] for j in range(n)]
        d = 2 + n
    dur = [[(int(a), int(b)) for a, b in tup.findall(lines[d + j])]
           for j in range(n)]
    assert all(len(r) == m for r in mach) and all(len(r) == m for r in dur), \
        f"parse error in {stem}: expected {n}x{m}"
    return n, m, mach, dur


def decode(order, n, m, mach, dur):
    """Semi-active schedule from a task-id priority order (component-wise).
    Task id = job*m + pos. Returns (Cmax_lo, Cmax_hi, machine windows)."""
    job_end = [(0, 0)] * n
    mac_end = [(0, 0)] * m
    mac_first = [None] * m           # start of first task per machine
    for tid in order:
        j, pos = divmod(tid, m)
        k = mach[j][pos]
        p = dur[j][pos]
        s = (max(job_end[j][0], mac_end[k][0]), max(job_end[j][1], mac_end[k][1]))
        e = (s[0] + p[0], s[1] + p[1])
        job_end[j] = e
        mac_end[k] = e
        if mac_first[k] is None:
            mac_first[k] = s
    cmax = (max(e[0] for e in job_end), max(e[1] for e in job_end))
    windows = [(mac_end[k][0] - mac_first[k][0], mac_end[k][1] - mac_first[k][1])
               for k in range(m)]
    return cmax, windows


def passive_energy(windows, pp):
    lo = sum(p * w[0] for p, w in zip(pp, windows))
    hi = sum(p * w[1] for p, w in zip(pp, windows))
    return lo, hi


def load_sols(stem, n_tasks):
    """Latest _Sols file per config; returns list of task orders."""
    orders = []
    for d in SOLS_DIRS:
        cands = sorted(glob.glob(os.path.join(d, stem + "_*_Sols.csv")))
        if not cands:
            continue
        with open(cands[-1], encoding="utf-8", errors="replace") as fh:
            for line in fh:
                parts = line.strip().split(";")
                if len(parts) >= 2 and parts[0].isdigit():
                    order = [int(x) for x in parts[1].split()]
                    if len(order) == n_tasks:
                        orders.append(order)
    return orders


def random_order(n, m, rng):
    """Random topological order: repeatedly pick a random available task."""
    nxt = [0] * n
    order = []
    while len(order) < n * m:
        j = rng.choice([x for x in range(n) if nxt[x] < m])
        order.append(j * m + nxt[j])
        nxt[j] += 1
    return order


def pearson(xs, ys):
    mx, my = st.mean(xs), st.mean(ys)
    num = sum((x - mx) * (y - my) for x, y in zip(xs, ys))
    dx = sum((x - mx) ** 2 for x in xs) ** 0.5
    dy = sum((y - my) ** 2 for y in ys) ** 0.5
    return num / (dx * dy) if dx > 0 and dy > 0 else float("nan")


def nondominated(points):
    """Count non-dominated (min, min) points."""
    nd = []
    for p in points:
        if not any(q[0] <= p[0] and q[1] <= p[1] and q != p for q in points):
            nd.append(p)
    return sorted(set(nd))


def analyse(stem):
    n, m, mach, dur = parse_instance(stem)
    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]

    sols = load_sols(stem, n * m)
    rng2 = random.Random(1)
    rand = [random_order(n, m, rng2) for _ in range(N_RANDOM)]

    def evaluate(orders):
        pts = []
        for o in orders:
            cmax, win = decode(o, n, m, mach, dur)
            pe = passive_energy(win, pp)
            pts.append(((cmax[0] + cmax[1]) / 2, (pe[0] + pe[1]) / 2))
        return pts

    p_near, p_rand = evaluate(sols), evaluate(rand)

    print(f"\n=== {stem}  ({n}x{m}, Pp~U{{2..8}} seed {POWER_SEED}) ===")
    print(f"near-optimal schedules: {len(p_near)}   random: {len(p_rand)}")
    corr = pearson(*zip(*p_rand))
    print(f"[global, random cloud]   corr(Cmax_mid, PE_mid) = {corr:+.3f}")
    if p_near:
        corr_n = pearson(*zip(*p_near)) if len(set(p_near)) > 2 else float("nan")
        nd = nondominated(p_near)
        cbest = min(p[0] for p in p_near)
        ties = [p for p in p_near if p[0] == cbest]
        pes = [p[1] for p in p_near]
        spread = (max(pes) - min(pes)) / min(pes) * 100
        print(f"[near-optimal set]       corr = {corr_n:+.3f}   "
              f"non-dominated: {len(nd)}/{len(set(p_near))} distinct")
        print(f"  PE spread across near-optimal schedules: {spread:.1f}%   "
              f"(PE range {min(pes):.0f}..{max(pes):.0f})")
        if len(ties) > 1:
            tspread = (max(t[1] for t in ties) - min(t[1] for t in ties)) / min(t[1] for t in ties) * 100
            print(f"  among Cmax-BEST ties ({len(ties)} sols): PE spread {tspread:.1f}%")
        print(f"  Pareto shape (near-opt): " +
              " | ".join(f"({c:.0f},{e:.0f})" for c, e in nd[:8]) +
              (" ..." if len(nd) > 8 else ""))


if __name__ == "__main__":
    stems = sys.argv[1:] or ["F0.15.0.ft10_10", "F0.15.0.la29_03"]
    for s in stems:
        analyse(s)
