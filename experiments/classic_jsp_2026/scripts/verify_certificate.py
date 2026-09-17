#!/usr/bin/env python3
"""Independently verify a classic-JSP schedule certificate.

The certificate produced by JSPCertificateAnalyzer lists, for every task, the
job it belongs to, its position in that job, its machine, its start time and
its duration.  This checker reads the ORIGINAL OR-Library instance data rather
than the converted instance file, so a bug in the conversion cannot hide here:
the machine routes and processing times in the certificate must match the
published instance exactly.

It then verifies, without using any of the solver's code:

  1. every task of the instance appears exactly once;
  2. machine route and duration agree with the published instance;
  3. operations of a job run in job order and never overlap;
  4. operations sharing a machine never overlap;
  5. no task starts before time zero;
  6. the reported makespan equals the largest completion time.

Finally it compares the makespan against the published lower bound and
best-known solution.  A makespan below a valid lower bound would mean the
bound, the instance or this checker is wrong -- it is reported as an anomaly,
not as a success.
"""
import argparse
import csv
import re
import sys
from collections import defaultdict

INSTANCE_HEADER = re.compile(r"^\s*instance\s+(\S+)\s*$")


def load_orlib(path, wanted):
    """Return {job: [(machine, duration), ...]} for the requested instance."""
    lines = open(path).read().splitlines()
    i = 0
    while i < len(lines):
        match = INSTANCE_HEADER.match(lines[i])
        i += 1
        if not match or match.group(1) != wanted:
            continue
        while i < len(lines):
            tokens = lines[i].split()
            if len(tokens) == 2 and all(t.isdigit() for t in tokens):
                break
            i += 1
        n_jobs, n_machines = (int(t) for t in lines[i].split())
        i += 1
        routes = []
        while len(routes) < n_jobs:
            tokens = lines[i].split()
            i += 1
            if not tokens:
                continue
            values = [int(t) for t in tokens]
            routes.append([(values[2 * k], values[2 * k + 1])
                           for k in range(n_machines)])
        return routes, n_jobs, n_machines
    raise SystemExit("instance %r not found in %s" % (wanted, path))


def load_certificate(path):
    """Return {run: [row, ...]} with numeric fields already converted."""
    runs = defaultdict(list)
    with open(path) as handle:
        for row in csv.DictReader(handle, delimiter=";"):
            runs[int(row["run"])].append({
                "task": int(row["task"]),
                "job": int(row["job"]),
                "operation": int(row["operation"]),
                "machine": int(row["machine"]),
                "start": float(row["start"]),
                "duration": float(row["duration"]),
                "completion": float(row["completion"]),
            })
    return runs


def check_run(rows, routes, n_jobs, n_machines):
    """Return (makespan, [errors])."""
    errors = []
    seen = set()
    by_job = defaultdict(list)
    by_machine = defaultdict(list)

    for row in rows:
        if row["task"] in seen:
            errors.append("task %d appears more than once" % row["task"])
        seen.add(row["task"])

        job, operation = row["job"], row["operation"]
        if not (0 <= job < n_jobs) or not (0 <= operation < n_machines):
            errors.append("task %d has out-of-range job/operation %d/%d"
                          % (row["task"], job, operation))
            continue

        machine, duration = routes[job][operation]
        if row["machine"] != machine:
            errors.append("job %d op %d: machine %d, instance says %d"
                          % (job, operation, row["machine"], machine))
        if row["duration"] != duration:
            errors.append("job %d op %d: duration %g, instance says %d"
                          % (job, operation, row["duration"], duration))
        if row["completion"] != row["start"] + row["duration"]:
            errors.append("job %d op %d: completion %g != start + duration"
                          % (job, operation, row["completion"]))
        if row["start"] < 0:
            errors.append("job %d op %d starts at %g, before time zero"
                          % (job, operation, row["start"]))

        by_job[job].append(row)
        by_machine[row["machine"]].append(row)

    expected = n_jobs * n_machines
    if len(seen) != expected:
        errors.append("certificate has %d distinct tasks, expected %d"
                      % (len(seen), expected))

    # Job precedence: operation k cannot start before operation k-1 completes.
    for job, rows_of_job in by_job.items():
        rows_of_job.sort(key=lambda r: r["operation"])
        for previous, current in zip(rows_of_job, rows_of_job[1:]):
            if current["operation"] != previous["operation"] + 1:
                errors.append("job %d: operations %d and %d are not consecutive"
                              % (job, previous["operation"],
                                 current["operation"]))
            if current["start"] < previous["completion"]:
                errors.append(
                    "job %d: op %d starts at %g before op %d completes at %g"
                    % (job, current["operation"], current["start"],
                       previous["operation"], previous["completion"]))

    # Machine capacity: operations on one machine must not overlap.
    for machine, rows_of_machine in by_machine.items():
        rows_of_machine.sort(key=lambda r: r["start"])
        for previous, current in zip(rows_of_machine, rows_of_machine[1:]):
            if current["start"] < previous["completion"]:
                errors.append(
                    "machine %d: job %d op %d starts at %g while job %d op %d "
                    "runs until %g"
                    % (machine, current["job"], current["operation"],
                       current["start"], previous["job"],
                       previous["operation"], previous["completion"]))

    makespan = max((row["completion"] for row in rows), default=0)
    return makespan, errors


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--orlib", required=True,
                        help="published OR-Library instance data")
    parser.add_argument("--instance", required=True, help="e.g. ta01")
    parser.add_argument("--certificate", required=True)
    parser.add_argument("--bounds", help="taillard_bounds.csv")
    parser.add_argument("--quiet", action="store_true",
                        help="only report failures and record makespans")
    args = parser.parse_args()

    routes, n_jobs, n_machines = load_orlib(args.orlib, args.instance)
    runs = load_certificate(args.certificate)
    if not runs:
        raise SystemExit("certificate %s contains no runs" % args.certificate)

    lb = bks = None
    if args.bounds:
        for row in csv.DictReader(open(args.bounds)):
            if row["instance"] == args.instance:
                lb, bks = int(row["lb"]), int(row["bks"])

    failed = False
    best = None
    for run in sorted(runs):
        makespan, errors = check_run(runs[run], routes, n_jobs, n_machines)
        if errors:
            failed = True
            print("%s run %d: INFEASIBLE (%d problems)"
                  % (args.instance, run, len(errors)))
            for message in errors[:10]:
                print("    " + message)
            continue
        if best is None or makespan < best:
            best = makespan
        if not args.quiet:
            print("%s run %d: feasible, makespan %g"
                  % (args.instance, run, makespan))

    if best is None:
        raise SystemExit(1)

    verdict = "feasible"
    if lb is not None:
        gap = 100.0 * (best - lb) / lb
        if best < lb:
            verdict = ("ANOMALY: below the published lower bound %d -- the "
                       "bound, the instance or this checker is wrong" % lb)
            failed = True
        elif best == lb:
            verdict = "matches the lower bound %d (optimal)" % lb
        elif bks is not None and best < bks:
            verdict = ("IMPROVES the best known solution %d (lower bound %d, "
                       "gap %.2f%%)" % (bks, lb, gap))
        elif bks is not None and best == bks:
            verdict = "matches the best known solution %d (gap %.2f%%)" % (bks,
                                                                           gap)
        else:
            verdict = "gap %.2f%% to lower bound %d (best known %s)" % (
                gap, lb, bks)

    print("%s: best verified makespan %g -- %s" % (args.instance, best,
                                                   verdict))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
