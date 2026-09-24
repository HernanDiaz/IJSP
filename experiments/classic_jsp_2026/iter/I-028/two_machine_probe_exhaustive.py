#!/usr/bin/env python3
"""I-028 probe: B-12 on two machines. Exact reoptimisation of the sequences of
two machines at once, every other machine fixed, on stored schedules.

With the machine arcs of M1 and M2 removed the disjunctive graph is acyclic
and gives every operation on either machine a head r, a tail q and, for each
ordered pair, the longest path L(a, b) between their starts through the rest:
a delayed precedence. For any sequences of M1 and M2 the earliest starts are

    S_b = max(r_b, C of b's machine predecessor, max over a of S_a + L(a, b))

and the makespan is max(C0, max_b S_b + p_b + q_b), C0 being the longest path
through operations on neither machine: exactly the longest path of the full
graph.

The best pair of sequences is found by depth-first branch and bound over
active schedules of the two machines (Giffler and Thompson restricted to them:
take the eligible operation with the earliest completion, branch on the
operations of its machine that could start before that completion). Bound: the
current partial makespan against Jackson's preemptive schedule of each
machine's remaining operations. The incumbent is the current makespan, so only
strict improvements survive; every one is checked against the full graph.

Pairs: among the four machines with most operations on a critical path, all
six pairs, as B-12 put it ("the two machines contributing most critical
arcs"). Pairs are retried until none improves. A node limit bounds each search
and the share of searches completed within it is reported.

Usage: b12_two.py <certificate glob> [...]
"""
import heapq
import itertools
import os
import sys
import time
from collections import defaultdict

sys.path.insert(0, "/opt/scratch")
from b12_probe import BKS, E, NEG, Schedule, jps_bound, load   # noqa: E402

NODE_LIMIT = 300000
TOP_MACHINES = 99


def critical_machines(sch):
    succ, pred = sch.succ_pred()
    order = sch.topo(succ, pred)
    S, arg = {}, {}
    for v in order:
        S[v], arg[v] = 0, None
        for u in pred[v]:
            if S[u] + sch.p[u] > S[v]:
                S[v], arg[v] = S[u] + sch.p[u], u
    end = max(sch.nodes, key=lambda v: S[v] + sch.p[v])
    count = defaultdict(int)
    v = end
    while v is not None:
        count[sch.mach[v]] += 1
        v = arg[v]
    return [m for m, _ in sorted(count.items(), key=lambda x: -x[1])][:TOP_MACHINES]


def reoptimise_pair(sch, M1, M2, ub):
    succ, pred = sch.succ_pred(skip=None)
    # rebuild without the arcs of both machines
    succ, pred = defaultdict(list), defaultdict(list)
    for j, ops in sch.jobops.items():
        for a, b in zip(ops, ops[1:]):
            succ[(j, a)].append((j, b)); pred[(j, b)].append((j, a))
    for m, s in sch.seq.items():
        if m in (M1, M2):
            continue
        for a, b in zip(s, s[1:]):
            succ[a].append(b); pred[b].append(a)
    order = sch.topo(succ, pred)
    r = {}
    for v in order:
        r[v] = max((r[u] + sch.p[u] for u in pred[v]), default=0)
    q = {}
    for v in reversed(order):
        q[v] = max((sch.p[w] + q[w] for w in succ[v]), default=0)
    ops = sch.seq[M1] + sch.seq[M2]
    free = set(ops)
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
    best = [ub, None]
    nodes = [0]

    def est(b, starts, tfree):
        return max([r[b], tfree[mach[b]]] + [starts[a] + L[(a, b)] for a in preds_of[b]])

    def dfs(seqs, starts, cmax, tfree, remaining):
        nodes[0] += 1
        if nodes[0] > NODE_LIMIT:
            return
        if not remaining:
            if cmax < best[0]:
                best[0], best[1] = cmax, {m: list(s) for m, s in seqs.items()}
            return
        # bound: Jackson's preemptive schedule on each machine
        lb = cmax
        for m in (M1, M2):
            items = []
            for b in remaining:
                if mach[b] != m:
                    continue
                rb = max([r[b]] + [starts[a] + L[(a, b)] for a in preds_of[b] if a in starts])
                items.append((rb, sch.p[b], q[b]))
            if items:
                lb = max(lb, jps_bound(tfree[m], items))
        if lb >= best[0]:
            return
        elig = [b for b in remaining if all(a in starts for a in preds_of[b])]
        if not elig:
            return   # cannot happen in an acyclic selection; guard anyway
        ests = {b: est(b, starts, tfree) for b in elig}
        star = min(elig, key=lambda b: ests[b] + sch.p[b])
        ect = ests[star] + sch.p[star]
        m = mach[star]
        conflict = [b for b in elig if mach[b] == m and ests[b] < ect]
        conflict.sort(key=lambda b: -q[b])
        for c in conflict:
            sc = ests[c]
            nc = max(cmax, sc + sch.p[c] + q[c])
            if nc >= best[0]:
                continue
            seqs[m].append(c); starts[c] = sc
            old = tfree[m]; tfree[m] = sc + sch.p[c]
            remaining.remove(c)
            dfs(seqs, starts, nc, tfree, remaining)
            remaining.add(c)
            tfree[m] = old
            del starts[c]; seqs[m].pop()
            if nodes[0] > NODE_LIMIT:
                return

    dfs({M1: [], M2: []}, {}, c0, {M1: 0, M2: 0}, set(ops))
    return (best[0] if best[1] else None), best[1], nodes[0], nodes[0] <= NODE_LIMIT


def improve(sch):
    cur = sch.makespan()
    start = cur
    st = {"searches": 0, "complete": 0, "improvements": 0}
    improved = True
    while improved:
        improved = False
        crit = set(critical_machines(sch))
        pairs = [(a, b) for a, b in itertools.combinations(sorted(sch.seq), 2)
                 if a in crit or b in crit]
        for M1, M2 in pairs:
            val, seqs, nodes, complete = reoptimise_pair(sch, M1, M2, cur)
            st["searches"] += 1
            st["complete"] += complete
            if seqs is not None and val < cur:
                sch.seq[M1], sch.seq[M2] = seqs[M1], seqs[M2]
                real = sch.makespan()
                assert real == val, "model %d != graph %d" % (val, real)
                cur = real
                st["improvements"] += 1
                improved = True
                break          # the critical machines may have changed
    return start, cur, st


def main():
    paths = []
    import glob
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
    print("\n%d schedules; improved %d (%.1f %%), %d units; %d pair searches, "
          "%.1f %% completed within the node limit; %.0f s"
          % (tot["schedules"], tot["improved"],
             100.0 * tot["improved"] / max(1, tot["schedules"]), tot["units"],
             tot["searches"], 100.0 * tot["complete"] / max(1, tot["searches"]),
             time.time() - t_all))


if __name__ == "__main__":
    main()
