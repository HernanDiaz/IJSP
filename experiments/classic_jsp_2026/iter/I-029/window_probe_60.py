#!/usr/bin/env python3
"""I-029 probe: B-12 in its literal window form. Exact reoptimisation of every
operation that starts inside a slice of time, on all machines at once, with
the rest of the schedule fixed.

A window is WINDOW consecutive operations in start-time order, slid by STEP
(both fixed before any result was read). On each machine the window's
operations are consecutive in its sequence, so each machine splits into a fixed
prefix, a free block and a fixed suffix. The graph keeps the job arcs, the
arcs inside prefixes and suffixes, an arc from the prefix's last operation to
every block operation and from every block operation to the suffix's first,
and drops the arcs inside the blocks. That graph is acyclic (the current
schedule satisfies it) and gives heads r, tails q and delayed precedences
L(a, b) between the free operations; the makespan of any ordering of the
blocks is max(C0, max S_b + p_b + q_b) with
S_b = max(r_b, C of b's block predecessor, max S_a + L(a, b)),
exactly the longest path of the full graph.

The best ordering is found by branch and bound over the active schedules of
the free operations (Giffler and Thompson over every machine with a block),
bounded by Jackson's preemptive schedule of each machine's block, strict
improvements only, each checked against the full graph. Windows are swept
until none improves.

Usage: b12_window.py <certificate glob> [...]
"""
import glob
import os
import sys
import time
from collections import defaultdict

sys.path.insert(0, "/opt/scratch")
from b12_probe import BKS, E, NEG, Schedule, jps_bound, load   # noqa: E402

WINDOW = 60
STEP = 30
NODE_LIMIT = 300000


def starts_of(sch):
    succ, pred = sch.succ_pred()
    order = sch.topo(succ, pred)
    S = {}
    for v in order:
        S[v] = max((S[u] + sch.p[u] for u in pred[v]), default=0)
    return S


def reoptimise_window(sch, free, ub):
    blocks, prefix_last, suffix_first = {}, {}, {}
    for m, s in sch.seq.items():
        idx = [k for k, v in enumerate(s) if v in free]
        if not idx:
            continue
        assert idx == list(range(idx[0], idx[-1] + 1)), "block not contiguous"
        blocks[m] = [s[k] for k in idx]
        prefix_last[m] = s[idx[0] - 1] if idx[0] > 0 else None
        suffix_first[m] = s[idx[-1] + 1] if idx[-1] + 1 < len(s) else None
    succ, pred = defaultdict(list), defaultdict(list)

    def arc(a, b):
        succ[a].append(b); pred[b].append(a)
    for j, ops in sch.jobops.items():
        for a, b in zip(ops, ops[1:]):
            arc((j, a), (j, b))
    for m, s in sch.seq.items():
        for a, b in zip(s, s[1:]):
            if a in free or b in free:
                continue
            arc(a, b)
        if m in blocks:
            for b in blocks[m]:
                if prefix_last[m] is not None:
                    arc(prefix_last[m], b)
                if suffix_first[m] is not None:
                    arc(b, suffix_first[m])
            if prefix_last[m] is not None and suffix_first[m] is not None:
                arc(prefix_last[m], suffix_first[m])
    order = sch.topo(succ, pred)
    assert order is not None
    r = {}
    for v in order:
        r[v] = max((r[u] + sch.p[u] for u in pred[v]), default=0)
    q = {}
    for v in reversed(order):
        q[v] = max((sch.p[w] + q[w] for w in succ[v]), default=0)
    ops = [v for m in blocks for v in blocks[m]]
    c0 = max((r[v] + sch.p[v] + q[v] for v in sch.nodes if v not in free), default=0)
    if c0 >= ub:
        return None, None, 0, True
    pos = {v: k for k, v in enumerate(order)}
    L = {}
    for a in ops:
        d = {a: 0}
        for v in order[pos[a]:]:
            if v not in d:
                continue
            for w in succ[v]:
                nd = d[v] + sch.p[v]
                if nd > d.get(w, NEG):
                    d[w] = nd
        for b in ops:
            if b != a and b in d:
                L[(a, b)] = d[b]
    preds_of = {b: [a for a in ops if (a, b) in L] for b in ops}
    mach = {v: sch.mach[v] for v in ops}
    machines = list(blocks)
    best = [ub, None]
    nodes = [0]

    def dfs(seqs, st, cmax, tfree, remaining):
        nodes[0] += 1
        if nodes[0] > NODE_LIMIT:
            return
        if not remaining:
            if cmax < best[0]:
                best[0], best[1] = cmax, {m: list(x) for m, x in seqs.items()}
            return
        lb = cmax
        per = defaultdict(list)
        for b in remaining:
            rb = max([r[b]] + [st[a] + L[(a, b)] for a in preds_of[b] if a in st])
            per[mach[b]].append((rb, sch.p[b], q[b]))
        for m, items in per.items():
            lb = max(lb, jps_bound(tfree[m], items))
            if lb >= best[0]:
                return
        elig = [b for b in remaining if all(a in st for a in preds_of[b])]
        if not elig:
            return
        est = {b: max([r[b], tfree[mach[b]]] + [st[a] + L[(a, b)] for a in preds_of[b]])
               for b in elig}
        star = min(elig, key=lambda b: est[b] + sch.p[b])
        ect = est[star] + sch.p[star]
        m = mach[star]
        conflict = sorted((b for b in elig if mach[b] == m and est[b] < ect),
                          key=lambda b: -q[b])
        for c in conflict:
            sc = est[c]
            nc = max(cmax, sc + sch.p[c] + q[c])
            if nc >= best[0]:
                continue
            seqs[m].append(c); st[c] = sc
            old = tfree[m]; tfree[m] = sc + sch.p[c]
            remaining.remove(c)
            dfs(seqs, st, nc, tfree, remaining)
            remaining.add(c)
            tfree[m] = old
            del st[c]; seqs[m].pop()
            if nodes[0] > NODE_LIMIT:
                return

    dfs({m: [] for m in machines}, {}, c0, {m: 0 for m in machines}, set(ops))
    if best[1] is None:
        return None, None, nodes[0], nodes[0] <= NODE_LIMIT
    new_seq = {}
    for m, s in sch.seq.items():
        if m in blocks:
            k0 = s.index(blocks[m][0])
            new_seq[m] = s[:k0] + best[1][m] + s[k0 + len(blocks[m]):]
    return best[0], new_seq, nodes[0], nodes[0] <= NODE_LIMIT


def improve(sch):
    cur = sch.makespan()
    start = cur
    stat = {"searches": 0, "complete": 0, "improvements": 0}
    improved = True
    while improved:
        improved = False
        S = starts_of(sch)
        by_start = sorted(sch.nodes, key=lambda v: (S[v], sch.mach[v]))
        for k in range(0, max(1, len(by_start) - WINDOW + 1), STEP):
            free = set(by_start[k:k + WINDOW])
            val, new_seq, nodes, complete = reoptimise_window(sch, free, cur)
            stat["searches"] += 1
            stat["complete"] += complete
            if new_seq is not None and val < cur:
                saved = {m: sch.seq[m] for m in new_seq}
                sch.seq.update(new_seq)
                real = sch.makespan()
                assert real == val, "model %d != graph %d" % (val, real)
                cur = real
                stat["improvements"] += 1
                improved = True
                break
    return start, cur, stat


def main():
    paths = []
    for g in sys.argv[1:]:
        paths += sorted(glob.glob(g))
    tot = defaultdict(int)
    t_all = time.time()
    for path in paths:
        inst = os.path.basename(path).split("_")[0]
        for run, rows in load(path).items():
            sch = Schedule(rows)
            t0 = time.time()
            start, end, st = improve(sch)
            tot["schedules"] += 1
            tot["improved"] += end < start
            tot["units"] += start - end
            tot["searches"] += st["searches"]
            tot["complete"] += st["complete"]
            mark = ""
            if end < BKS[inst]:
                mark = "  *** BELOW THE BEST KNOWN ***"
            elif end == BKS[inst] and start > BKS[inst]:
                mark = "  (reaches the best known)"
            if end < start or mark:
                print("%-5s %-58s run %s: %d -> %d in %d moves, %.1f s%s"
                      % (inst, os.path.relpath(path, E), run, start, end,
                         st["improvements"], time.time() - t0, mark), flush=True)
    print("\n%d schedules; improved %d (%.1f %%), %d units; %d window searches, "
          "%.1f %% completed within the node limit; %.0f s"
          % (tot["schedules"], tot["improved"],
             100.0 * tot["improved"] / max(1, tot["schedules"]), tot["units"],
             tot["searches"], 100.0 * tot["complete"] / max(1, tot["searches"]),
             time.time() - t_all))


if __name__ == "__main__":
    main()
