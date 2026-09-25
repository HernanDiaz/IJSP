#!/usr/bin/env python3
"""I-054: exact recombination of two attractor schedules with CP-SAT.

For two feasible schedules A and B of the same instance, every pair of
operations on the same machine that A and B order the same way keeps that
order as a hard precedence; the pairs they disagree on are free. CP-SAT then
minimises the makespan over that subspace, which holds A, B and every schedule
that agrees with both wherever they agree. The better parent is the hint.

Printed: the share of machine pairs the parents agree on, the best makespan in
the subspace, and the status (OPTIMAL means the subspace holds nothing better
than what was found: a proof about that region, not a guess). A better schedule
is written as a certificate for verify_certificate.py.

Usage: recombine.py <instance> <certA> <runA> <certB> <runB> <seconds> <tag>
"""
import csv
import itertools
import os
import sys
import time
from collections import defaultdict

from ortools.sat.python import cp_model

HERE = os.path.dirname(os.path.abspath(__file__))
E = os.path.abspath(os.path.join(HERE, "..", ".."))
BOUNDS = {r["instance"]: (int(r["lb"]), int(r["bks"]))
          for r in csv.DictReader(open(os.path.join(E, "taillard_bounds.csv")))}


def load(path, run):
    ops = {}
    for r in csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";"):
        if r["run"] == str(run):
            ops[(int(r["job"]), int(r["operation"]))] = (int(r["machine"]), int(float(r["duration"])),
                                                         int(float(r["start"])), int(r["task"]))
    assert ops, (path, run)
    return ops


def main():
    inst, ca, ra, cb, rb, seconds, tag = sys.argv[1:8]
    A, B = load(ca, ra), load(cb, rb)
    assert set(A) == set(B)
    msA = max(s + p for (_, p, s, _) in A.values())
    msB = max(s + p for (_, p, s, _) in B.values())
    hint, hint_ms = (A, msA) if msA <= msB else (B, msB)
    lb, bks = BOUNDS[inst]

    by_machine = defaultdict(list)
    for v, (m, _, _, _) in A.items():
        by_machine[m].append(v)
    model = cp_model.CpModel()
    horizon = max(msA, msB)
    start, end = {}, {}
    per_machine = defaultdict(list)
    for v, (m, p, _, _) in A.items():
        start[v] = model.NewIntVar(0, horizon, "s%d_%d" % v)
        end[v] = model.NewIntVar(0, horizon, "e%d_%d" % v)
        per_machine[m].append(model.NewIntervalVar(start[v], p, end[v], "i%d_%d" % v))
        model.AddHint(start[v], hint[v][2])
    jobs = defaultdict(list)
    for (j, o) in A:
        jobs[j].append(o)
    for j, seq in jobs.items():
        seq.sort()
        for a, b in zip(seq, seq[1:]):
            model.Add(start[(j, b)] >= end[(j, a)])
    for m, ivs in per_machine.items():
        model.AddNoOverlap(ivs)
    agree = total = 0
    for m, ops in by_machine.items():
        for u, v in itertools.combinations(ops, 2):
            total += 1
            a_uv = A[u][2] < A[v][2]
            b_uv = B[u][2] < B[v][2]
            if a_uv == b_uv:
                agree += 1
                first, second = (u, v) if a_uv else (v, u)
                model.Add(start[second] >= end[first])
    ms = model.NewIntVar(lb, horizon, "makespan")
    model.AddMaxEquality(ms, list(end.values()))
    model.Minimize(ms)

    solver = cp_model.CpSolver()
    solver.parameters.num_search_workers = 14
    solver.parameters.random_seed = 1
    solver.parameters.max_time_in_seconds = float(seconds)
    t0 = time.time()
    status = solver.Solve(model)
    best = int(solver.ObjectiveValue()) if status in (cp_model.OPTIMAL, cp_model.FEASIBLE) else None
    print("%s %s: parents %d and %d, agree on %d of %d machine pairs (%.1f %%), free %d; "
          "best %s, status %s, bound %d, %.0f s"
          % (inst, tag, msA, msB, agree, total, 100.0 * agree / total, total - agree,
             best, solver.StatusName(status), int(solver.BestObjectiveBound()), time.time() - t0), flush=True)
    if best is not None and best < min(msA, msB):
        path = os.path.join(HERE, "found_%s_%s_%d_Certificate.csv" % (tag, inst, best))
        with open(path, "w") as f:
            f.write("run;task;job;operation;machine;start;duration;completion\n")
            for (j, o), (m, p, _, task) in sorted(A.items(), key=lambda kv: kv[1][3]):
                s = solver.Value(start[(j, o)])
                f.write("1;%d;%d;%d;%d;%d;%d;%d\n" % (task, j, o, m, s, p, s + p))
        print("   written %s" % path, flush=True)


if __name__ == "__main__":
    main()
