#!/usr/bin/env python3
"""I-068: break the consensus core on a SET of machines (from I-057's break_machine.py).

As consensus.py, but the consensus pairs on every machine of the fifth argument
(comma-separated, e.g. 3,7) are left free too; every other consensus pair
stays fixed.

Original description follows.

I-055: consensus recombination of a group of schedules with CP-SAT.

Given the first K schedules of a list (one '<certificate> <run> <makespan>' per
line, lowest first), every pair of operations on the same machine that ALL K
order the same way is fixed as a precedence; every other pair is free. CP-SAT
minimises the makespan over that subspace, hinted with the best schedule of the
group. OPTIMAL means the subspace holds nothing better: a proof about the
region spanned by the group's consensus.

Usage: break_machines.py <instance> <list file> <K> <seconds> <m1,m2,...>
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
    inst, listfile, k, seconds = sys.argv[1], sys.argv[2], int(sys.argv[3]), float(sys.argv[4])
    free_machines = set(int(x) for x in sys.argv[5].split(","))
    entries = [l.split() for l in open(listfile) if l.strip()][:k]
    group = [load(p, r) for (p, r, _) in entries]
    best_ms = min(max(s + p for (_, p, s, _) in g.values()) for g in group)
    hint = min(group, key=lambda g: max(s + p for (_, p, s, _) in g.values()))
    lb, bks = BOUNDS[inst]
    base = group[0]

    by_machine = defaultdict(list)
    for v, (m, _, _, _) in base.items():
        by_machine[m].append(v)
    model = cp_model.CpModel()
    horizon = max(max(s + p for (_, p, s, _) in g.values()) for g in group)
    start, end = {}, {}
    per_machine = defaultdict(list)
    for v, (m, p, _, _) in base.items():
        start[v] = model.NewIntVar(0, horizon, "s%d_%d" % v)
        end[v] = model.NewIntVar(0, horizon, "e%d_%d" % v)
        per_machine[m].append(model.NewIntervalVar(start[v], p, end[v], "i%d_%d" % v))
        model.AddHint(start[v], hint[v][2])
    jobs = defaultdict(list)
    for (j, o) in base:
        jobs[j].append(o)
    for j, seq in jobs.items():
        seq.sort()
        for a, b in zip(seq, seq[1:]):
            model.Add(start[(j, b)] >= end[(j, a)])
    for m, ivs in per_machine.items():
        model.AddNoOverlap(ivs)
    fixed = total = 0
    for m, ops in by_machine.items():
        for u, v in itertools.combinations(ops, 2):
            total += 1
            orders = set(g[u][2] < g[v][2] for g in group)
            if len(orders) == 1 and m not in free_machines:
                fixed += 1
                first, second = (u, v) if orders.pop() else (v, u)
                model.Add(start[second] >= end[first])
    ms = model.NewIntVar(lb, horizon, "makespan")
    model.AddMaxEquality(ms, list(end.values()))
    model.Minimize(ms)

    solver = cp_model.CpSolver()
    solver.parameters.num_search_workers = 14
    solver.parameters.random_seed = 1
    solver.parameters.max_time_in_seconds = seconds
    t0 = time.time()
    status = solver.Solve(model)
    best = int(solver.ObjectiveValue()) if status in (cp_model.OPTIMAL, cp_model.FEASIBLE) else None
    print("%s machines %s freed, K=%d: consensus fixes %d of %d machine pairs (%.1f %%), free %d; group best %d; "
          "subspace best %s, status %s, bound %d, %.0f s"
          % (inst, sys.argv[5], k, fixed, total, 100.0 * fixed / total, total - fixed, best_ms,
             best, solver.StatusName(status), int(solver.BestObjectiveBound()), time.time() - t0), flush=True)
    if best is not None and best < best_ms:
        path = os.path.join(HERE, "found_m%s_%s_%d_Certificate.csv" % (sys.argv[5].replace(",", "-"), inst, best))
        with open(path, "w") as f:
            f.write("run;task;job;operation;machine;start;duration;completion\n")
            for (j, o), (m, p, _, task) in sorted(base.items(), key=lambda kv: kv[1][3]):
                s = solver.Value(start[(j, o)])
                f.write("1;%d;%d;%d;%d;%d;%d;%d\n" % (task, j, o, m, s, p, s + p))
        print("   written %s" % path, flush=True)


if __name__ == "__main__":
    main()
