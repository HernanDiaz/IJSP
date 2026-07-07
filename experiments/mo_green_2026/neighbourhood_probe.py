#!/usr/bin/env python3
"""
neighbourhood_probe.py — decisive diagnostic for the sweep phase-B stall.

From the phase-A anchor of the instrumented sweep run, exhaustively
enumerate every 1-step move of two operator classes and evaluate the
component-wise semi-active (Cmax, NPE):

  R: reversal of every adjacent machine pair (u,v)      [what the TS has]
  I: insertion of every task at every position of its   [what the TS lacks]
     machine sequence

Feasibility by cycle detection (Kahn). For each cap of the ladder, report
how many moves are feasible, under-cap, and NPE-improving, plus the best
NPE reachable in one step. If R yields ~0 improving moves while I yields
many, the missing operator is insertion — mechanism identified and fix
prescribed.

Usage: python3 neighbourhood_probe.py [sols_csv] [instance_stem]
"""
import glob, os, random, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance, POWER_SEED

HERE = os.path.dirname(os.path.abspath(__file__))


def machine_seqs_from_order(order, n, m, mach):
    seqs = [[] for _ in range(m)]
    for tid in order:
        j, pos = divmod(tid, m)
        seqs[mach[j][pos]].append(tid)
    return seqs


def decode(seqs, n, m, mach, dur, pp, comp):
    """Kahn topological longest-path; None if cyclic."""
    ntasks = n * m
    p = [0] * ntasks
    preds = [[] for _ in range(ntasks)]
    succs = [[] for _ in range(ntasks)]
    for j in range(n):
        for pos in range(m):
            tid = j * m + pos
            p[tid] = dur[j][pos][comp]
            if pos > 0:
                preds[tid].append(tid - 1)
                succs[tid - 1].append(tid)
    for k in range(m):
        for a, b in zip(seqs[k], seqs[k][1:]):
            preds[b].append(a)
            succs[a].append(b)
    indeg = [len(preds[t]) for t in range(ntasks)]
    head = [0] * ntasks
    stack = [t for t in range(ntasks) if indeg[t] == 0]
    seen = 0
    while stack:
        t = stack.pop()
        seen += 1
        for s in succs[t]:
            head[s] = max(head[s], head[t] + p[t])
            indeg[s] -= 1
            if indeg[s] == 0:
                stack.append(s)
    if seen != ntasks:
        return None
    cmax = max(head[t] + p[t] for t in range(ntasks))
    npe = 0
    for k in range(m):
        for a, b in zip(seqs[k], seqs[k][1:]):
            npe += pp[k] * (head[b] - (head[a] + p[a]))
    return cmax, npe


def evaluate(seqs, n, m, mach, dur, pp):
    lo = decode(seqs, n, m, mach, dur, pp, 0)
    hi = decode(seqs, n, m, mach, dur, pp, 1)
    if lo is None or hi is None:
        return None
    return (lo[0], hi[0]), (lo[1], hi[1])


def moves(seqs, kind):
    m = len(seqs)
    for k in range(m):
        s = seqs[k]
        if kind == "R":
            for i in range(len(s) - 1):
                t = [list(x) for x in seqs]
                t[k][i], t[k][i + 1] = t[k][i + 1], t[k][i]
                yield t
        else:
            for i in range(len(s)):
                for j in range(len(s)):
                    if j == i or j == i - 1 and False:
                        continue
                    if j == i:
                        continue
                    t = [list(x) for x in seqs]
                    v = t[k].pop(i)
                    t[k].insert(j, v)
                    yield t


def main():
    sols = sys.argv[1] if len(sys.argv) > 1 else sorted(glob.glob(
        os.path.join(HERE, "results", "sweep_debug", "*_Sols.csv")))[-1]
    stem = sys.argv[2] if len(sys.argv) > 2 else "F0.15.0.ft10_10"
    n, m, mach, dur = parse_instance(stem)
    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]
    with open(sols, encoding="utf-8", errors="replace") as fh:
        for line in fh:
            parts = line.strip().split(";")
            if len(parts) >= 2 and parts[0] == "1":
                order = [int(x) for x in parts[1].split()]
                break
    seqs = machine_seqs_from_order(order, n, m, mach)
    (c0, c1), (e0, e1) = evaluate(seqs, n, m, mach, dur, pp)
    print(f"anchor: Cmax=({c0},{c1})  NPE=({e0},{e1})  npe_hi={e1}")

    caps = [(int(c0 * (1000 + pm) / 1000), int(c1 * (1000 + pm) / 1000), pm)
            for pm in (10, 50, 100)]
    for kind, label in (("R", "reversals "), ("I", "insertions")):
        tot = feas = 0
        stats = {pm: [0, e1] for _, _, pm in caps}
        for t in moves(seqs, kind):
            tot += 1
            r = evaluate(t, n, m, mach, dur, pp)
            if r is None:
                continue
            feas += 1
            (nc0, nc1), (ne0, ne1) = r
            for ca, cb, pm in caps:
                if nc0 <= ca and nc1 <= cb and ne1 < e1:
                    stats[pm][0] += 1
                    stats[pm][1] = min(stats[pm][1], ne1)
        line = f"{label}: total={tot:4d} feasible={feas:4d}"
        for _, _, pm in caps:
            line += f" | +{pm}pm: improving={stats[pm][0]:3d} bestNPE={stats[pm][1]}"
        print(line)


if __name__ == "__main__":
    main()
