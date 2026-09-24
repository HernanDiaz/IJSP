#!/usr/bin/env python3
"""B-12 probe: does an EXACT one-machine reoptimisation ever improve the local
optima the tabu search ends in?

For a schedule, fix every machine sequence but one machine M, and find the
sequence of M that minimises the makespan exactly. With M's machine arcs
removed the disjunctive graph G' is a DAG; for M's operations it gives heads r
(longest path from the source), tails q (longest path to the sink) and, for
each ordered pair (a, b), the longest path L(a, b) from a's start to b's start
through the other machines, which is a delayed precedence: a must precede b
and b cannot start before S_a + L(a, b). For a sequence of M the earliest
starts are then

    S_k = max(r_k, C_prev, max over earlier a of S_a + L(a, k))

and the makespan is max(C0, max_k S_k + p_k + q_k), C0 being the longest path
of G' through operations not on M. That is exact: it is the longest path of
the full graph. The best sequence is found by depth-first branch and bound,
pruned with Jackson's preemptive schedule on the unscheduled operations and
the current makespan as the incumbent, so only STRICT improvements survive.

Applied machine by machine until no machine improves (a local optimum of the
exact one-machine neighbourhood). A machine is skipped when C0 alone already
reaches the incumbent. A node limit bounds each search and is reported.

Reads certificates; nothing is run. Usage: b12_probe.py <certificate glob> [...]
"""
import csv
import glob
import os
import sys
import time
from collections import defaultdict

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
BKS = {r["instance"]: int(r["bks"]) for r in csv.DictReader(open(os.path.join(E, "taillard_bounds.csv")))}
NODE_LIMIT = 200000
NEG = float("-inf")


def load(path):
    runs = defaultdict(list)
    for r in csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";"):
        runs[r["run"]].append(r)
    return runs


class Schedule:
    def __init__(self, rows):
        self.p, self.mach, self.jobops = {}, {}, defaultdict(list)
        seqs = defaultdict(list)
        for r in rows:
            v = (int(r["job"]), int(r["operation"]))
            self.p[v] = int(float(r["duration"]))
            self.mach[v] = int(r["machine"])
            self.jobops[v[0]].append(v[1])
            seqs[self.mach[v]].append((float(r["start"]), v))
        self.seq = {m: [v for _, v in sorted(s)] for m, s in seqs.items()}
        for j in self.jobops:
            self.jobops[j].sort()
        self.nodes = list(self.p)

    def succ_pred(self, skip=None):
        succ, pred = defaultdict(list), defaultdict(list)
        for j, ops in self.jobops.items():
            for a, b in zip(ops, ops[1:]):
                succ[(j, a)].append((j, b)); pred[(j, b)].append((j, a))
        for m, s in self.seq.items():
            if m == skip:
                continue
            for a, b in zip(s, s[1:]):
                succ[a].append(b); pred[b].append(a)
        return succ, pred

    def topo(self, succ, pred):
        indeg = {v: len(pred[v]) for v in self.nodes}
        stack = [v for v in self.nodes if indeg[v] == 0]
        order = []
        while stack:
            u = stack.pop()
            order.append(u)
            for w in succ[u]:
                indeg[w] -= 1
                if indeg[w] == 0:
                    stack.append(w)
        return order if len(order) == len(self.nodes) else None

    def makespan(self):
        succ, pred = self.succ_pred()
        order = self.topo(succ, pred)
        assert order is not None, "cyclic schedule"
        S = {}
        for v in order:
            S[v] = max((S[u] + self.p[u] for u in pred[v]), default=0)
        return max(S[v] + self.p[v] for v in self.nodes)


def jps_bound(t0, items):
    """Jackson's preemptive schedule: items (r, p, q); lower bound on
    max(C + q) with no machine available before t0."""
    items = sorted(([max(r, t0), p, q] for r, p, q in items), key=lambda x: x[0])
    import heapq
    t, i, heap, lb = t0, 0, [], NEG
    n = len(items)
    while i < n or heap:
        if not heap and t < items[i][0]:
            t = items[i][0]
        while i < n and items[i][0] <= t:
            heapq.heappush(heap, [-items[i][2], items[i][1]])
            i += 1
        negq, rem = heap[0]
        nxt = items[i][0] if i < n else float("inf")
        run = min(rem, nxt - t)
        t += run
        if run == rem:
            heapq.heappop(heap)
            lb = max(lb, t - negq)
        else:
            heap[0][1] = rem - run
    return lb


def reoptimise_machine(sch, M, ub):
    """Best sequence for machine M with every other machine fixed, if it is
    strictly better than ub. Returns (value, sequence, nodes, complete)."""
    succ, pred = sch.succ_pred(skip=M)
    order = sch.topo(succ, pred)
    r = {}
    for v in order:
        r[v] = max((r[u] + sch.p[u] for u in pred[v]), default=0)
    q = {}
    for v in reversed(order):
        q[v] = max((sch.p[w] + q[w] for w in succ[v]), default=0)
    ops = sch.seq[M]
    onM = set(ops)
    c0 = max((r[v] + sch.p[v] + q[v] for v in sch.nodes if v not in onM), default=0)
    if c0 >= ub:
        return None, None, 0, True
    # delayed precedences L(a, b): longest path from a's start to b's start in G'
    L = {}
    pos = {v: k for k, v in enumerate(order)}
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
    best = [ub, None]
    nodes = [0]

    def dfs(seq, starts, cmax, tfree, remaining):
        nodes[0] += 1
        if nodes[0] > NODE_LIMIT:
            return
        if not remaining:
            if cmax < best[0]:
                best[0], best[1] = cmax, list(seq)
            return
        items = []
        for b in remaining:
            rb = max([r[b]] + [starts[a] + L[(a, b)] for a in preds_of[b] if a in starts])
            items.append((rb, sch.p[b], q[b]))
        if max(cmax, jps_bound(tfree, items)) >= best[0]:
            return
        cands = [b for b in remaining if all(a in starts for a in preds_of[b])]
        cands.sort(key=lambda b: -q[b])
        for b in cands:
            sb = max([r[b], tfree] + [starts[a] + L[(a, b)] for a in preds_of[b]])
            nc = max(cmax, sb + sch.p[b] + q[b])
            if nc >= best[0]:
                continue
            seq.append(b); starts[b] = sb
            remaining.remove(b)
            dfs(seq, starts, nc, sb + sch.p[b], remaining)
            remaining.add(b)
            del starts[b]; seq.pop()
            if nodes[0] > NODE_LIMIT:
                return

    dfs([], {}, c0, 0, set(ops))
    return best[0] if best[1] else None, best[1], nodes[0], nodes[0] <= NODE_LIMIT


def improve(sch):
    """Exact one-machine reoptimisation, machine by machine, to a local optimum."""
    cur = sch.makespan()
    start = cur
    stats = {"searches": 0, "complete": 0, "nodes": 0, "improvements": 0}
    improved = True
    while improved:
        improved = False
        for M in sorted(sch.seq):
            val, seq, nodes, complete = reoptimise_machine(sch, M, cur)
            stats["searches"] += 1
            stats["nodes"] += nodes
            stats["complete"] += complete
            if seq is not None and val < cur:
                sch.seq[M] = seq
                real = sch.makespan()
                assert real == val, "model %d != graph %d" % (val, real)
                cur = real
                stats["improvements"] += 1
                improved = True
    return start, cur, stats


def main():
    paths = []
    for g in sys.argv[1:]:
        paths += sorted(glob.glob(g))
    tot = defaultdict(int)
    for path in paths:
        inst = os.path.basename(path).split("_")[0]
        for run, rows in load(path).items():
            sch = Schedule(rows)
            t0 = time.time()
            start, end, st = improve(sch)
            dt = time.time() - t0
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
                print("%-5s %-60s run %s: %d -> %d in %d moves, %.1f s%s"
                      % (inst, os.path.relpath(path, E), run, start, end,
                         st["improvements"], dt, mark), flush=True)
    print("\n%d schedules; improved %d (%.1f %%), %d units in total; "
          "%d machine searches, %.1f %% completed within the node limit"
          % (tot["schedules"], tot["improved"],
             100.0 * tot["improved"] / max(1, tot["schedules"]), tot["units"],
             tot["searches"], 100.0 * tot["complete"] / max(1, tot["searches"])))


if __name__ == "__main__":
    main()
