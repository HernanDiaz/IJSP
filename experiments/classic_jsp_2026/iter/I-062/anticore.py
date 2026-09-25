#!/usr/bin/env python3
"""I-062: search AGAINST the core with CP-SAT.

The core is the set of machine pairs that every schedule of a group orders
alike (I-055/I-060). Each core pair (u first, v second) gets a Boolean x:
x forces u before v, not x forces v before u. The model asks for a schedule
with makespan at most TARGET that flips at least D core pairs (sum of not x
>= D), and minimises the makespan within that. I-061 proved that no ta22
schedule of makespan <= 1606 keeps the whole core, so a schedule of makespan
<= 1612 that flips core pairs is exactly what the region outside the core
must hold if our 1613 can be beaten there.

A schedule found is written as a certificate for verify_certificate.py.

Usage: anticore.py <instance> <group list> <K> <target makespan> <D> <seconds>
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
    for r in csv.DictReader((l for l in open(os.path.join(E, path)) if not l.startswith("#")), delimiter=";"):
        if r["run"] == str(run):
            ops[(int(r["job"]), int(r["operation"]))] = (int(r["machine"]), int(float(r["duration"])),
                                                         int(float(r["start"])), int(r["task"]))
    assert ops, (path, run)
    return ops


def main():
    inst, listfile, k, target, d, seconds = (sys.argv[1], sys.argv[2], int(sys.argv[3]),
                                             int(sys.argv[4]), int(sys.argv[5]), float(sys.argv[6]))
    entries = [l.split() for l in open(listfile) if l.strip()][:k]
    group = [load(p, r) for (p, r, _) in entries]
    base = group[0]
    lb, bks = BOUNDS[inst]

    model = cp_model.CpModel()
    horizon = target
    start, end = {}, {}
    per_machine = defaultdict(list)
    by_machine = defaultdict(list)
    for v, (m, p, _, _) in base.items():
        start[v] = model.NewIntVar(0, horizon, "s%d_%d" % v)
        end[v] = model.NewIntVar(0, horizon, "e%d_%d" % v)
        per_machine[m].append(model.NewIntervalVar(start[v], p, end[v], "i%d_%d" % v))
        by_machine[m].append(v)
    jobs = defaultdict(list)
    for (j, o) in base:
        jobs[j].append(o)
    for j, seq in jobs.items():
        seq.sort()
        for a, b in zip(seq, seq[1:]):
            model.Add(start[(j, b)] >= end[(j, a)])
    for m, ivs in per_machine.items():
        model.AddNoOverlap(ivs)
    keep = []
    for m, ops in by_machine.items():
        for u, v in itertools.combinations(ops, 2):
            orders = set(g[u][2] < g[v][2] for g in group)
            if len(orders) == 1:
                first, second = (u, v) if orders.pop() else (v, u)
                x = model.NewBoolVar("core_%d_%d_%d_%d" % (first + second))
                model.Add(start[second] >= end[first]).OnlyEnforceIf(x)
                model.Add(start[first] >= end[second]).OnlyEnforceIf(x.Not())
                keep.append(x)
    model.Add(sum(keep) <= len(keep) - d)      # at least d core pairs flipped
    ms = model.NewIntVar(lb, horizon, "makespan")
    model.AddMaxEquality(ms, list(end.values()))
    model.Minimize(ms)

    solver = cp_model.CpSolver()
    solver.parameters.num_search_workers = 14
    solver.parameters.random_seed = 1
    solver.parameters.max_time_in_seconds = seconds
    t0 = time.time()
    status = solver.Solve(model)
    if status in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        best = int(solver.ObjectiveValue())
        flipped = sum(1 for x in keep if not solver.Value(x))
        print("%s D=%d target<=%d: core %d pairs; FOUND makespan %d flipping %d core pairs, status %s, bound %d, %.0f s"
              % (inst, d, target, len(keep), best, flipped, solver.StatusName(status),
                 int(solver.BestObjectiveBound()), time.time() - t0), flush=True)
        path = os.path.join(HERE, "found_D%d_%s_%d_Certificate.csv" % (d, inst, best))
        with open(path, "w") as f:
            f.write("run;task;job;operation;machine;start;duration;completion\n")
            for (j, o), (m, p, _, task) in sorted(base.items(), key=lambda kv: kv[1][3]):
                s = solver.Value(start[(j, o)])
                f.write("1;%d;%d;%d;%d;%d;%d;%d\n" % (task, j, o, m, s, p, s + p))
    else:
        print("%s D=%d target<=%d: core %d pairs; nothing found, status %s, %.0f s"
              % (inst, d, target, len(keep), solver.StatusName(status), time.time() - t0), flush=True)


if __name__ == "__main__":
    main()
