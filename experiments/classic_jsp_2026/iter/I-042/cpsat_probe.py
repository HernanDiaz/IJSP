#!/usr/bin/env python3
"""I-042: CP-SAT warm-started from our best stored schedules.

For each target instance, the full job-shop model is built from the
certificate itself (job, operation, machine, duration of every task: the
instance data travels with the schedule, the instance files are not touched):
one interval per operation, job order as precedences, one no-overlap per
machine, minimise the makespan. The stored schedule is given as a complete
hint, so CP-SAT starts from it, and its large-neighbourhood workers try to
improve it. Fixed before any result: 14 workers, random seed 1, 600 s per
instance.

Any schedule strictly better than the hint is written as a certificate in the
solver's format (iter/I-042/found_<inst>_<makespan>_Certificate.csv), to be
checked by scripts/verify_certificate.py against the original OR-Library
data. A record is shown by the schedule itself, nothing else.

Usage: cpsat_probe.py <instance> <certificate> <run> [seconds] [seed]

The seed defaults to 1, as in I-042 to I-051 (added for I-052).
"""
import csv
import os
import sys
import time
from collections import defaultdict

from ortools.sat.python import cp_model

HERE = os.path.dirname(os.path.abspath(__file__))
E = os.path.abspath(os.path.join(HERE, "..", ".."))
BOUNDS = {r["instance"]: (int(r["lb"]), int(r["bks"]))
          for r in csv.DictReader(open(os.path.join(E, "taillard_bounds.csv")))}
WORKERS = 14
SEED = 1


def load_run(path, run):
    rows = [r for r in csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";")
            if r["run"] == str(run)]
    assert rows, "no rows for run %s in %s" % (run, path)
    return rows


def main():
    inst, cert, run = sys.argv[1], sys.argv[2], sys.argv[3]
    seconds = float(sys.argv[4]) if len(sys.argv) > 4 else 600.0
    seed = int(sys.argv[5]) if len(sys.argv) > 5 else SEED
    rows = load_run(cert, run)
    lb, bks = BOUNDS[inst]
    ops = {}
    for r in rows:
        ops[(int(r["job"]), int(r["operation"]))] = (int(r["machine"]), int(float(r["duration"])),
                                                     int(float(r["start"])), int(r["task"]))
    hint_ms = max(s + p for (_, p, s, _) in ops.values())
    horizon = hint_ms
    print("%s: %d operations, hint makespan %d, LB %d, BKS %d" % (inst, len(ops), hint_ms, lb, bks), flush=True)

    m = cp_model.CpModel()
    start, end, itv = {}, {}, {}
    per_machine = defaultdict(list)
    for v, (mach, p, s, _) in ops.items():
        start[v] = m.NewIntVar(0, horizon, "s%d_%d" % v)
        end[v] = m.NewIntVar(0, horizon, "e%d_%d" % v)
        itv[v] = m.NewIntervalVar(start[v], p, end[v], "i%d_%d" % v)
        per_machine[mach].append(itv[v])
        m.AddHint(start[v], s)
        m.AddHint(end[v], s + p)
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
    m.AddHint(ms, hint_ms)
    m.Minimize(ms)

    solver = cp_model.CpSolver()
    solver.parameters.num_search_workers = WORKERS
    solver.parameters.random_seed = seed
    solver.parameters.max_time_in_seconds = seconds

    class Progress(cp_model.CpSolverSolutionCallback):
        def __init__(self):
            super().__init__()
            self.t0 = time.time()
            self.best = None

        def on_solution_callback(self):
            v = int(self.ObjectiveValue())
            if self.best is None or v < self.best:
                self.best = v
                print("   t=%7.1f s  makespan %d  (bound %d)"
                      % (time.time() - self.t0, v, int(self.BestObjectiveBound())), flush=True)
                if v < hint_ms:
                    self.save(v)

        def save(self, v):
            path = os.path.join(HERE, "found_%s_%d_Certificate.csv" % (inst, v))
            with open(path, "w") as f:
                f.write("run;task;job;operation;machine;start;duration;completion\n")
                for (j, o), (mach, p, _, task) in sorted(ops.items(), key=lambda kv: kv[1][3]):
                    s = self.Value(start[(j, o)])
                    f.write("1;%d;%d;%d;%d;%d;%d;%d\n" % (task, j, o, mach, s, p, s + p))

    cb = Progress()
    t0 = time.time()
    status = solver.Solve(m, cb)
    best = cb.best if cb.best is not None else hint_ms
    print("%s: status %s, best %d (hint %d, BKS %d), proven bound %d, %.0f s"
          % (inst, solver.StatusName(status), best, hint_ms, bks,
             int(solver.BestObjectiveBound()), time.time() - t0), flush=True)
    verdict = "RECORD" if best < bks else ("match" if best == bks else "above BKS")
    print("%s: %s" % (inst, verdict), flush=True)


if __name__ == "__main__":
    main()
