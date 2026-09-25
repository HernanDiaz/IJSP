#!/usr/bin/env python3
"""I-059: CP-SAT COLD, without any hint.

The full job-shop model is built from the instance data a certificate carries
(job, operation, machine, duration of every task), exactly as in I-042, but no
hint is given: CP-SAT starts from nothing. After the time limit, its final
schedule is compared with a reference schedule (the ABC attractor) as the
share of machine pairs the two order alike, and written as a certificate.

The question is whether a generator other than the ABC lands in other regions
than the ABC's common core (I-055: two thirds of the machine decisions shared
by 107 floors on ta25).

Usage: cold_probe.py <instance> <certificate> <run> <seconds> <seed> <reference certificate> <reference run>
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
    inst, cert, run, seconds, seed, ref_cert, ref_run = sys.argv[1:8]
    ops = load(cert, run)
    ref = load(ref_cert, ref_run)
    lb, bks = BOUNDS[inst]
    horizon = sum(p for (_, p, _, _) in ops.values())

    m = cp_model.CpModel()
    start, end = {}, {}
    per_machine = defaultdict(list)
    for v, (mach, p, _, _) in ops.items():
        start[v] = m.NewIntVar(0, horizon, "s%d_%d" % v)
        end[v] = m.NewIntVar(0, horizon, "e%d_%d" % v)
        per_machine[mach].append(m.NewIntervalVar(start[v], p, end[v], "i%d_%d" % v))
    jobs = defaultdict(list)
    for (j, o) in ops:
        jobs[j].append(o)
    for j, seq in jobs.items():
        seq.sort()
        for a, b in zip(seq, seq[1:]):
            m.Add(start[(j, b)] >= end[(j, a)])
    for mach, ivs in per_machine.items():
        m.AddNoOverlap(ivs)
    ms = m.NewIntVar(lb, horizon, "makespan")
    m.AddMaxEquality(ms, list(end.values()))
    m.Minimize(ms)

    solver = cp_model.CpSolver()
    solver.parameters.num_search_workers = 14
    solver.parameters.random_seed = int(seed)
    solver.parameters.max_time_in_seconds = float(seconds)
    t0 = time.time()
    status = solver.Solve(m)
    if status not in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        print("%s seed %s: no solution, status %s" % (inst, seed, solver.StatusName(status)), flush=True)
        return
    best = int(solver.ObjectiveValue())
    mine = {v: solver.Value(start[v]) for v in ops}

    by_m = defaultdict(list)
    for v, (mach, _, _, _) in ops.items():
        by_m[mach].append(v)
    agree = total = 0
    for mach, vs in by_m.items():
        for a, b in itertools.combinations(vs, 2):
            total += 1
            agree += (mine[a] < mine[b]) == (ref[a][2] < ref[b][2])
    print("%s seed %s: cold best %d (BKS %d, reference %d), agreement with the reference %.1f %% (%d of %d), "
          "bound %d, %.0f s"
          % (inst, seed, best, bks, max(s + p for (_, p, s, _) in ref.values()),
             100.0 * agree / total, agree, total, int(solver.BestObjectiveBound()), time.time() - t0), flush=True)

    path = os.path.join(HERE, "cold_%s_seed%s_%d_Certificate.csv" % (inst, seed, best))
    with open(path, "w") as f:
        f.write("run;task;job;operation;machine;start;duration;completion\n")
        for (j, o), (mach, p, _, task) in sorted(ops.items(), key=lambda kv: kv[1][3]):
            s = mine[(j, o)]
            f.write("1;%d;%d;%d;%d;%d;%d;%d\n" % (task, j, o, mach, s, p, s + p))


if __name__ == "__main__":
    main()
