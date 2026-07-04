#!/usr/bin/env python3
"""
epsilon_probe.py — Phase 0 gate: exact Pareto front (Cmax, NPE) by
epsilon-constraint with CP-SAT, replicating the conflict probe of
Afsar et al. 2022 (SwEvo, Sec. 3.5 / Fig. 1) on our interval JSP instances.

Model (crisp probe on interval midpoints, x2 scale to stay integer):
- standard JSP: job precedence chains + NoOverlap per machine, FREE start
  times (right-shifts allowed — NPE is not regular, cf. #17).
- NPE = sum_m Pp_m * idle_m, idle_m = (max end_m - min start_m) - busy_m
  (window minus constant busy = sum of gaps between consecutive tasks).
- Pp_m ~ U{2..8} seed 23 (same provisional scheme as conflict_check.py).

Sweep: min Cmax -> C*; then for eps in C* * (1 + k/100), k = 0..10:
min NPE s.t. Cmax <= eps. A substantially decreasing NPE(eps) curve
= genuine conflict = green MO on interval JSP is viable.

Run inside the OR-Tools venv:  ~/ortools-venv/bin/python3 epsilon_probe.py
"""
import random, sys, time, os

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance  # dual-format instance parser

from ortools.sat.python import cp_model

TIME_PER_SOLVE = 60          # seconds per epsilon point
POWER_SEED = 23
EPS_STEPS = list(range(0, 11))   # % relaxation over C*


def build(n, m, mach, dur):
    """Returns (model, cmax, npe, x2-scaled busy) with free start times."""
    mdl = cp_model.CpModel()
    mid2 = [[a + b for (a, b) in row] for row in dur]     # 2*midpoint, int
    horizon = sum(sum(r) for r in mid2)
    starts, ends = {}, {}
    machine_tasks = [[] for _ in range(m)]
    for j in range(n):
        for pos in range(m):
            s = mdl.NewIntVar(0, horizon, f"s{j}_{pos}")
            e = mdl.NewIntVar(0, horizon, f"e{j}_{pos}")
            iv = mdl.NewIntervalVar(s, mid2[j][pos], e, f"iv{j}_{pos}")
            starts[j, pos], ends[j, pos] = s, e
            machine_tasks[mach[j][pos]].append((j, pos, iv))
            if pos > 0:
                mdl.Add(s >= ends[j, pos - 1])
    for k in range(m):
        mdl.AddNoOverlap([iv for _, _, iv in machine_tasks[k]])

    cmax = mdl.NewIntVar(0, horizon, "cmax")
    mdl.AddMaxEquality(cmax, [ends[j, m - 1] for j in range(n)])

    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]
    idle_terms = []
    busy = []
    for k in range(m):
        first = mdl.NewIntVar(0, horizon, f"first{k}")
        last = mdl.NewIntVar(0, horizon, f"last{k}")
        mdl.AddMinEquality(first, [starts[j, p] for j, p, _ in machine_tasks[k]])
        mdl.AddMaxEquality(last, [ends[j, p] for j, p, _ in machine_tasks[k]])
        b = sum(mid2[j][p] for j, p, _ in machine_tasks[k])
        busy.append(b)
        idle = mdl.NewIntVar(0, horizon, f"idle{k}")
        mdl.Add(idle == last - first - b)
        idle_terms.append(pp[k] * idle)
    npe = sum(idle_terms)
    return mdl, cmax, npe, sum(busy)


def solve(mdl, objective, extra=None, hint_time=TIME_PER_SOLVE):
    mdl2 = mdl.Clone() if extra else mdl
    solver = cp_model.CpSolver()
    solver.parameters.max_time_in_seconds = hint_time
    solver.parameters.num_search_workers = 4
    if extra:
        extra(mdl2)
    return mdl2, solver


def analyse(stem):
    n, m, mach, dur = parse_instance(stem)
    print(f"\n=== {stem} ({n}x{m})  [x2 scale, {TIME_PER_SOLVE}s/point] ===",
          flush=True)

    # Step 1: min Cmax
    mdl, cmax, npe, _ = build(n, m, mach, dur)
    mdl.Minimize(cmax)
    solver = cp_model.CpSolver()
    solver.parameters.max_time_in_seconds = TIME_PER_SOLVE
    solver.parameters.num_search_workers = 4
    st = solver.Solve(mdl)
    cstar = int(solver.Value(cmax))
    print(f"C* = {cstar/2:.1f}  ({solver.StatusName(st)}, "
          f"{solver.WallTime():.1f}s)", flush=True)

    # Step 2: epsilon sweep on NPE
    print(f"{'eps(%)':>7} {'Cmax<=':>9} {'NPE*':>9} {'status':>9} {'t(s)':>6}")
    front = []
    for k in EPS_STEPS:
        eps = cstar * (100 + k) // 100
        mdl, cmax, npe, _ = build(n, m, mach, dur)
        mdl.Add(cmax <= eps)
        mdl.Minimize(npe)
        solver = cp_model.CpSolver()
        solver.parameters.max_time_in_seconds = TIME_PER_SOLVE
        solver.parameters.num_search_workers = 4
        t0 = time.time()
        st = solver.Solve(mdl)
        val = int(solver.Value(npe)) if st in (cp_model.OPTIMAL, cp_model.FEASIBLE) else -1
        front.append((k, eps, val))
        print(f"{k:>7} {eps/2:>9.1f} {val/2:>9.1f} "
              f"{solver.StatusName(st):>9} {time.time()-t0:>6.1f}", flush=True)

    # Verdict
    vals = [v for _, _, v in front if v >= 0]
    if len(vals) >= 2 and vals[0] > 0:
        drop = (vals[0] - min(vals)) / vals[0] * 100
        print(f"\nNPE drop across 10% Cmax relaxation: {drop:.1f}%  "
              f"({'CONFLICT' if drop > 5 else 'weak/no conflict'})", flush=True)
    elif vals and vals[0] == 0:
        print("\nNPE = 0 already at C* — objectives NOT in conflict here.",
              flush=True)


if __name__ == "__main__":
    stems = sys.argv[1:] or ["F0.15.0.ft10_10", "F0.15.0.la29_03"]
    for s in stems:
        analyse(s)
    print("\nDONE", flush=True)
