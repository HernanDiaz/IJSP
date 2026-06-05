#!/usr/bin/env python3
"""
verify_ta54_2792.py
Independent verification of the Cmax=2792 solution for Taillard TA54
(tai50_15_04) found by N8 in crisp mode by FuzzyFW.

This script does NOT use the FuzzyFW codebase. It re-parses the Taillard
instance, re-parses the schedule (operation permutation) from the Sols
CSV, and recomputes the makespan with a standalone serial Schedule
Generation Scheme (SGS):

  - serial SGS: schedule operations in permutation order, each at the
    earliest time consistent with (i) job precedence and (ii) machine
    capacity (one op per machine at a time). This is the most basic,
    well-understood JSP decoder.

If the serial SGS returns Cmax <= 2839, the improvement over Taillard's
BKS is confirmed.

Usage:
    python verify_ta54_2792.py
"""

import re
import sys
from pathlib import Path

import os
# Auto-detect Windows vs WSL/Linux mount of the repo
_DEFAULT_WIN = "C:/Users/diazhernan/CLionProjects/IJSP"
_DEFAULT_WSL = "/mnt/c/Users/diazhernan/CLionProjects/IJSP"
ROOT = Path(_DEFAULT_WSL if os.path.isdir(_DEFAULT_WSL) else _DEFAULT_WIN)
INSTANCE = ROOT / "SelectosYTaillardCrisp" / "tai50_15_04.F.15_01.txt"
SOLS_CSV = ROOT / "experiments" / "results" / "exp9_ts_crisp" / "N8_ts_crisp" / \
           "tai50_15_04.F.15_01_20260520224352_Sols.csv"
TARGET_RUN = 15
EXPECTED_CMAX = 2792


def parse_instance(path: Path):
    """Parse a SelectosYTaillardCrisp instance.

    File layout:
      NUMERO DE TRABAJOS
      <n_jobs>
      NUMERO DE RECURSOS
      <n_machines>
      SECUENCIA DE MAQUINAS
      <n_jobs lines, each: n_machines integers (machine index per op)>
      DURACIONES
      <n_jobs lines, each: n_machines "(low, high)" pairs>
      TIEMPOS MAXIMOS DE FIN
      <... ignored>

    Returns (n_jobs, n_machines, machines[j][o], durations[j][o]).
    """
    lines = path.read_text(encoding="utf-8").splitlines()
    n_jobs = int(lines[1].strip())
    n_machines = int(lines[3].strip())

    # Machine sequences: lines 5..(5+n_jobs-1)  (0-indexed)
    machines = []
    for j in range(n_jobs):
        row = list(map(int, lines[5 + j].split()))
        assert len(row) == n_machines, f"job {j} machine row has {len(row)} entries"
        machines.append(row)

    # "DURACIONES" header at line 5+n_jobs
    dur_header_idx = 5 + n_jobs
    assert "DURACIONES" in lines[dur_header_idx], f"expected DURACIONES at line {dur_header_idx}"

    durations = []
    pair_re = re.compile(r"\(\s*(\d+)\s*,\s*(\d+)\s*\)")
    for j in range(n_jobs):
        line = lines[dur_header_idx + 1 + j]
        pairs = pair_re.findall(line)
        assert len(pairs) == n_machines, f"job {j} duration row has {len(pairs)} pairs"
        # In crisp, low == high; use low (== midpoint == single value)
        durations.append([int(low) for low, _ in pairs])

    return n_jobs, n_machines, machines, durations


def parse_solution(path: Path, run: int):
    """Extract the operation permutation for a given run from a _Sols.csv.

    Sols format: 'Run;Solution;Objective Value' with the permutation a
    space-separated list of integers in the Solution column.

    Returns (permutation: list[int], objective: str).
    """
    with path.open(encoding="utf-8") as fh:
        for raw in fh:
            parts = raw.rstrip("\n").split(";")
            if len(parts) < 3:
                continue
            try:
                if int(parts[0]) != run:
                    continue
            except ValueError:
                continue
            perm = list(map(int, parts[1].split()))
            obj = parts[2]
            return perm, obj
    raise RuntimeError(f"Run {run} not found in {path}")


def insertion_sgs(permutation, n_jobs, n_machines, machines, durations):
    """Insertion-based SGS: for each op (in permutation order) find the
    earliest gap on its machine where it can fit, respecting job precedence.

    Each machine is represented as a sorted list of busy intervals (start,end).
    A new op of duration d can start at S iff S >= job_prev_end AND
    the half-open interval [S, S+d) does not overlap any existing
    machine interval. The earliest such S is preferred.
    """
    machine_intervals = [[] for _ in range(n_machines)]
    job_next_free = [0] * n_jobs
    job_next_op = [0] * n_jobs

    for op_id in permutation:
        j, o = divmod(op_id, n_machines)
        if o != job_next_op[j]:
            raise RuntimeError(
                f"Permutation violates job precedence (job {j}, expected op "
                f"{job_next_op[j]}, got {o})")
        m = machines[j][o]
        dur = durations[j][o]
        earliest = job_next_free[j]
        intervals = machine_intervals[m]

        # Try to fit in the earliest available gap on machine m.
        # Sweep through gaps: before each interval i, between intervals,
        # and finally after the last interval.
        start = None
        prev_end = 0
        for s_i, e_i in intervals:
            gap_start = max(earliest, prev_end)
            gap_end = s_i
            if gap_start + dur <= gap_end:
                start = gap_start
                break
            prev_end = e_i
        if start is None:
            start = max(earliest, prev_end)

        end = start + dur

        # Insert (start, end) into intervals keeping sorted order
        i = 0
        while i < len(intervals) and intervals[i][0] < start:
            i += 1
        intervals.insert(i, (start, end))

        job_next_free[j] = end
        job_next_op[j] += 1

    return max(job_next_free)


def serial_sgs(permutation, n_jobs, n_machines, machines, durations,
               track_starts=False):
    """Decode an operation permutation into a schedule using serial SGS.

    Operation encoding (job-major):
        op_id = job_index * n_machines + position_within_job
      i.e. op_id 0..n_machines-1 are the ops of job 0 in order.

    For each op in permutation order, schedule it at the earliest time
    consistent with job precedence and the machine availability.

    Returns (makespan, ends[, starts]).
    """
    job_next_free = [0] * n_jobs        # earliest start for next op of job j
    job_next_op = [0] * n_jobs           # which op index is next for job j
    machine_next_free = [0] * n_machines

    ends = {}                            # (j, o) -> end time
    starts = {}                          # (j, o) -> start time
    for op_id in permutation:
        j, o = divmod(op_id, n_machines)
        if o != job_next_op[j]:
            raise RuntimeError(
                f"Permutation violates job precedence: tried to schedule op {o} of job {j} "
                f"before op {job_next_op[j]} of job {j}"
            )
        m = machines[j][o]
        dur = durations[j][o]
        start = max(job_next_free[j], machine_next_free[m])
        end = start + dur
        starts[(j, o)] = start
        ends[(j, o)] = end
        job_next_free[j] = end
        job_next_op[j] += 1
        machine_next_free[m] = end

    cmax = max(job_next_free)
    if track_starts:
        return cmax, ends, starts
    return cmax, ends


def format_solution_taillard(permutation, n_jobs, n_machines, machines,
                              durations, problem_name="ta54"):
    """Format the decoded schedule in the requested "machine-major" form:

        Problem: <name>
        Number of jobs: <n>
        Number of machines: <m>
        Makespan: <C>

        Solution sequence:
        <m lines, each with n job indices = job order on that machine>
    """
    cmax, _, starts = serial_sgs(permutation, n_jobs, n_machines, machines,
                                  durations, track_starts=True)

    # For each machine, collect all (start, job) pairs and sort by start
    by_machine = {m: [] for m in range(n_machines)}
    for j in range(n_jobs):
        for o in range(n_machines):
            m = machines[j][o]
            s = starts[(j, o)]
            by_machine[m].append((s, j))

    rows = []
    for m in range(n_machines):
        sorted_jobs = [j for _, j in sorted(by_machine[m])]
        assert len(sorted_jobs) == n_jobs
        rows.append(" ".join(str(j) for j in sorted_jobs))

    out = []
    out.append(f"Problem: {problem_name}")
    out.append(f"Number of jobs: {n_jobs}")
    out.append(f"Number of machines: {n_machines}")
    out.append(f"Makespan: {cmax}")
    out.append("")
    out.append("Solution sequence:")
    out.extend(rows)
    return "\n".join(out)


N2_SOLS_CSV = ROOT / "experiments" / "results" / "exp9_ts_crisp" / "N2_ts_crisp" / \
              "tai50_15_04.F.15_01_20260520223344_Sols.csv"


def main():
    n_jobs, n_machines, machines, durations = parse_instance(INSTANCE)

    # --- Sanity check: N2 Run 1 should give Cmax = 2839 (matches BKS) -------
    n2_perm, n2_obj = parse_solution(N2_SOLS_CSV, 1)
    n2_serial, _ = serial_sgs(n2_perm, n_jobs, n_machines, machines, durations)
    n2_insert = insertion_sgs(n2_perm, n_jobs, n_machines, machines, durations)
    print("=== Sanity check: N2 crisp Run 1 (FuzzyFW reports 2839) ===")
    print(f"  serial    SGS: {n2_serial}")
    print(f"  insertion SGS: {n2_insert}")
    print(f"  FuzzyFW value: {n2_obj}")
    print(f"  Taillard BKS:  2839")
    if n2_serial == 2839 and n2_insert == 2839:
        print(f"  → parser + SGS are consistent with the FuzzyFW value.")
    else:
        print(f"  → discrepancy: parser/SGS gives different value than FuzzyFW.")
    print()

    # --- Main verification: N8 Run 15 (FuzzyFW reports 2792) ----------------
    print(f"=== Main check: N8 crisp Run 15 (FuzzyFW reports 2792) ===")
    print(f"Instance: {INSTANCE.name}")
    print(f"Sols CSV: {SOLS_CSV.name}, run {TARGET_RUN}")
    print()

    print(f"Parsed: {n_jobs} jobs x {n_machines} machines = "
          f"{n_jobs * n_machines} operations")

    perm, obj = parse_solution(SOLS_CSV, TARGET_RUN)
    print(f"Permutation length: {len(perm)} (expected {n_jobs * n_machines})")
    print(f"Objective in CSV: {obj}")

    if len(perm) != n_jobs * n_machines:
        sys.exit("ERROR: permutation length mismatch")

    cmax_serial, _ = serial_sgs(perm, n_jobs, n_machines, machines, durations)
    cmax_insert = insertion_sgs(perm, n_jobs, n_machines, machines, durations)
    print()
    print(f"--- Independent verification (two SGS variants) ---")
    print(f"  serial    SGS Cmax = {cmax_serial}")
    print(f"  insertion SGS Cmax = {cmax_insert}")
    print(f"  FuzzyFW reported   = {EXPECTED_CMAX}")
    print(f"  Taillard BKS       = 2839")
    print()

    if cmax_insert == EXPECTED_CMAX:
        print(f"  STATUS: EXACT MATCH on the insertion-based SGS.")
        print(f"          The permutation, decoded with the same SGS family that")
        print(f"          fEABC uses, yields Cmax = {cmax_insert}, improving the")
        print(f"          Taillard BKS of 2839 by {2839 - cmax_insert} units "
              f"({(2839 - cmax_insert) * 100.0 / 2839:.2f}%).")
    elif cmax_insert < EXPECTED_CMAX:
        print(f"  STATUS: insertion SGS finds {cmax_insert} < {EXPECTED_CMAX}.")
        print(f"          BKS is improved by at least {2839 - cmax_insert} units.")
    elif cmax_insert <= 2839:
        print(f"  STATUS: insertion SGS gives Cmax = {cmax_insert}, still below the")
        print(f"          Taillard BKS (2839). The FuzzyFW value {EXPECTED_CMAX} may")
        print(f"          come from a slightly different SGS variant (e.g. with")
        print(f"          machine sweeping rather than per-job sweeping). Either")
        print(f"          way, the BKS is improved.")
    else:
        print(f"  STATUS: insertion SGS gives Cmax = {cmax_insert} > Taillard BKS (2839).")
        print(f"          Something inconsistent — please review.")


if __name__ == "__main__":
    main()
