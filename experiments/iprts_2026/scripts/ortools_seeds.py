#!/usr/bin/env python3
"""Cross-solver diverse warm-start seeds for IPRTS (OR-Tools CP-SAT).

Generates structurally diverse, high-quality task orders for the interval JSP
by solving CRISP reductions of each instance with CP-SAT and harvesting the
operation order. OR-Tools never sees the intervals: each interval duration
[lo, up] is scalarised to a single integer (lo / midpoint / up), the resulting
deterministic JSP is solved, and only the operation SEQUENCE is kept. That
sequence is re-decoded under the true interval arithmetic by the framework's
insertion SGS (CreationFromFileSchedule), so a crisp-optimal order is just a
good, structurally distinct STARTING point -- exactly what de-clusters the
elite pool that plain TS-N2/N8 warm seeds could not (see DESIGN.md / the
warm-seed saturation finding).

Why this gives diversity: each scalarisation is a DIFFERENT deterministic
problem with a different optimum, and on the hard x20 instances CP-SAT does
not prove optimality within the time limit, so different random seeds yield
different incumbents too.

Output matches the loader's format, one line per solution:
    run;<space-separated operation ids>;(lo, up)
where operation id = job * n_machines + position_in_job (the convention
ProblemIJSP assigns on load) and (lo, up) is the crisp makespan placeholder
-- used only by rank-biased selection; max-min diversity selection ignores it
and re-decodes every order, so for the x20 max-min plan the placeholder is
immaterial. Files are named <instance_stem>_Sols.csv to match
problem->getName() + "_Sols.csv" the loader resolves.

Usage:
    python3 ortools_seeds.py --instances <dir|glob|file> --out <dir> \
        [--scalarizations lo,mid,up] [--seeds-per 4] \
        [--time-limit 60] [--workers 8] [--base-seed 0]
    python3 ortools_seeds.py --instances <file> --out x --dry-run   # parse only

Requires (only for actual solving): pip install ortools
"""
import argparse
import glob
import os
import random
import re
import sys

INT_RE = re.compile(r"-?\d+")
PAIR_RE = re.compile(r"\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)")


def parse_instance(path):
    """Parse an interval JSP instance (both header dialects).

    Returns (n_jobs, n_machines, machine[job][k], lo[job][k], up[job][k]).
    Mirrors ProblemIJSP::loadFile: op id = job * n_machines + k, row-major.
    """
    with open(path) as fh:
        raw = [ln.rstrip("\n") for ln in fh]

    # n_jobs / n_machines = first two lines that are a single bare integer
    # (machine rows have many ints, duration rows are parenthesised pairs).
    singles = [s for s in (ln.strip() for ln in raw)
               if INT_RE.fullmatch(s or "")]
    if len(singles) < 2:
        raise ValueError("%s: could not read n_jobs / n_machines" % path)
    n_jobs, n_machines = int(singles[0]), int(singles[1])

    # Machine matrix: first n_jobs lines holding exactly n_machines ints.
    machines, idx = [], 0
    while idx < len(raw) and len(machines) < n_jobs:
        toks = INT_RE.findall(raw[idx])
        if len(toks) == n_machines:
            machines.append([int(t) for t in toks])
        idx += 1
    if len(machines) != n_jobs:
        raise ValueError("%s: found %d machine rows, expected %d"
                         % (path, len(machines), n_jobs))

    # Durations: first n_jobs*n_machines interval pairs AFTER the matrix
    # (skips the DURACIONES heading; stops before any trailing bounds line).
    pairs = []
    need = n_jobs * n_machines
    while idx < len(raw) and len(pairs) < need:
        for lo_s, up_s in PAIR_RE.findall(raw[idx]):
            pairs.append((int(lo_s), int(up_s)))
            if len(pairs) == need:
                break
        idx += 1
    if len(pairs) != need:
        raise ValueError("%s: found %d duration pairs, expected %d"
                         % (path, len(pairs), need))

    lo = [[pairs[j * n_machines + k][0] for k in range(n_machines)]
          for j in range(n_jobs)]
    up = [[pairs[j * n_machines + k][1] for k in range(n_machines)]
          for j in range(n_jobs)]

    # scalarize(): make machine ids 0-based (no-op when already 0-based).
    mn = min(m for row in machines for m in row)
    if mn:
        machines = [[m - mn for m in row] for row in machines]

    return n_jobs, n_machines, machines, lo, up


def crisp_durations(lo, up, mode, rng=None):
    """Scalarise the interval durations to a single integer per operation.

    mode is 'lo'/'mid'/'up', an explicit lambda in [0,1] (dur = lo + lam*(up-lo)),
    or 'sample' (uniform random in [lo,up] per op). 'sample' is the real
    diversity lever: each draw is a genuinely different crisp instance, whereas
    the monotone lambda modes barely differ when many ops have lo==up."""
    nj, nm = len(lo), len(lo[0])
    if mode == "sample":
        return [[rng.randint(lo[j][k], up[j][k]) for k in range(nm)]
                for j in range(nj)]
    lam = {"lo": 0.0, "mid": 0.5, "up": 1.0}.get(mode)
    if lam is None:
        lam = float(mode)                       # explicit lambda, e.g. 0.25
        if not 0.0 <= lam <= 1.0:
            raise ValueError("lambda out of [0,1]: %s" % mode)
    return [[int(round(lo[j][k] + lam * (up[j][k] - lo[j][k])))
             for k in range(nm)] for j in range(nj)]


def solve_order(n_jobs, n_machines, machines, dur, time_limit, workers, seed):
    """Solve the crisp JSP with CP-SAT; return (operation order, makespan)
    or None if no feasible solution was found in the time limit."""
    try:
        from ortools.sat.python import cp_model
    except ImportError:
        sys.exit("OR-Tools not found. Install with: pip install ortools")

    model = cp_model.CpModel()
    horizon = sum(sum(row) for row in dur)
    starts, ends, per_machine, job_ends = {}, {}, {m: [] for m in
                                                   range(n_machines)}, []
    for j in range(n_jobs):
        for k in range(n_machines):
            s = model.NewIntVar(0, horizon, "s_%d_%d" % (j, k))
            e = model.NewIntVar(0, horizon, "e_%d_%d" % (j, k))
            iv = model.NewIntervalVar(s, dur[j][k], e, "iv_%d_%d" % (j, k))
            starts[(j, k)], ends[(j, k)] = s, e
            per_machine[machines[j][k]].append(iv)
            if k > 0:
                model.Add(s >= ends[(j, k - 1)])
        job_ends.append(ends[(j, n_machines - 1)])
    for m in range(n_machines):
        model.AddNoOverlap(per_machine[m])
    cmax = model.NewIntVar(0, horizon, "cmax")
    model.AddMaxEquality(cmax, job_ends)
    model.Minimize(cmax)

    solver = cp_model.CpSolver()
    solver.parameters.max_time_in_seconds = time_limit
    solver.parameters.num_search_workers = workers
    solver.parameters.random_seed = seed
    status = solver.Solve(model)
    if status not in (cp_model.OPTIMAL, cp_model.FEASIBLE):
        return None

    # Topological order = ops by start time; (start, op_id) tie-break keeps
    # within-job precedence (op k+1 starts after op k ends, dur >= 1).
    ops = [(j, k) for j in range(n_jobs) for k in range(n_machines)]
    ops.sort(key=lambda jk: (solver.Value(starts[jk]),
                             jk[0] * n_machines + jk[1]))
    order = [jk[0] * n_machines + jk[1] for jk in ops]
    return order, int(solver.ObjectiveValue())


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--instances", required=True,
                    help="directory, glob, or a single instance file")
    ap.add_argument("--out", required=True, help="output seed directory")
    ap.add_argument("--scalarizations", default="lo,mid,up",
                    help="comma list of lo/mid/up, lambdas in [0,1], or 'sample'")
    ap.add_argument("--seeds-per", type=int, default=4,
                    help="number of 'sample' draws (deterministic modes give 1)")
    ap.add_argument("--time-limit", type=float, default=60.0,
                    help="seconds per CP-SAT solve")
    ap.add_argument("--workers", type=int, default=8)
    ap.add_argument("--base-seed", type=int, default=0)
    ap.add_argument("--dry-run", action="store_true",
                    help="parse instances and print a summary; do not solve")
    args = ap.parse_args()

    if os.path.isdir(args.instances):
        files = sorted(glob.glob(os.path.join(args.instances, "*.txt")))
    else:
        files = sorted(glob.glob(args.instances)) or [args.instances]
    files = [f for f in files if os.path.isfile(f)]
    if not files:
        sys.exit("no instance files matched: %s" % args.instances)
    if not args.dry_run:
        os.makedirs(args.out, exist_ok=True)
    modes = [m.strip() for m in args.scalarizations.split(",") if m.strip()]

    for path in files:
        stem = os.path.splitext(os.path.basename(path))[0]
        nj, nm, mac, lo, up = parse_instance(path)

        if args.dry_run:
            print("%-30s jobs=%2d machines=%2d ops=%4d | job0 mach=%s | "
                  "job0 dur[0:3]=%s | id(job1,op0)=%d"
                  % (stem, nj, nm, nj * nm, mac[0],
                     [(lo[0][k], up[0][k]) for k in range(min(3, nm))], nm))
            continue

        seen, lines, run = set(), [], 0
        for mode in modes:
            # deterministic modes (lo/mid/up/lambda) give one instance; only
            # 'sample' varies per draw, so only it repeats seeds_per times.
            reps = args.seeds_per if mode == "sample" else 1
            for s in range(reps):
                rng = random.Random(args.base_seed + 1000 * s) \
                    if mode == "sample" else None
                dur = crisp_durations(lo, up, mode, rng)
                res = solve_order(nj, nm, mac, dur, args.time_limit,
                                  args.workers, args.base_seed + s)
                if res is None:
                    print("  %s [%s seed %d]: no feasible solution" %
                          (stem, mode, s))
                    continue
                order, cm = res
                key = tuple(order)
                if key in seen:
                    continue
                seen.add(key)
                run += 1
                lines.append("%d;%s;(%d, %d)" %
                             (run, " ".join(map(str, order)), cm, cm))
        out_path = os.path.join(args.out, stem + "_Sols.csv")
        with open(out_path, "w") as fh:
            fh.write("Run;Solution;Objective Value\n")
            fh.write("\n".join(lines) + ("\n" if lines else ""))
        print("%-30s %d distinct seeds -> %s"
              % (stem, len(lines), os.path.basename(out_path)))


if __name__ == "__main__":
    main()
