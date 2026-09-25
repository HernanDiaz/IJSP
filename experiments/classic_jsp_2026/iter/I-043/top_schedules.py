#!/usr/bin/env python3
"""List the K lowest-makespan runs of an instance among certificates matched by
a glob, one line '<certificate> <run> <makespan>', distinct schedules only
(two runs with the same set of start times count once).

Only FEASIBLE runs are listed: every row readable, operations of a job in
order without overlap, and no two operations overlapping on a machine. The
check was added after the selector picked a 1998 on ta45 from
I-002_voidfilter_n8, the cell of the old N8 bug, whose schedules overlap on a
machine (verify_certificate.py: best verified 2027). The instance-level check
(every task present, routes and durations as published) stays with
verify_certificate.py, which every schedule CP-SAT writes goes through.
"""
import csv
import glob
import sys
from collections import defaultdict


def feasible(ops):
    """ops: list of (job, op, machine, start, duration)."""
    jobs, machines = defaultdict(list), defaultdict(list)
    for (j, o, m, st, p) in ops:
        jobs[j].append((o, st, p))
        machines[m].append((st, p))
    for seq in jobs.values():
        seq.sort()
        for (_, s1, p1), (_, s2, _) in zip(seq, seq[1:]):
            if s2 < s1 + p1:
                return False
    for seq in machines.values():
        seq.sort()
        for (s1, p1), (s2, _) in zip(seq, seq[1:]):
            if s2 < s1 + p1:
                return False
    return True


def main():
    inst, pattern, k = sys.argv[1], sys.argv[2], int(sys.argv[3])
    cands, seen = [], set()
    for path in sorted(glob.glob(pattern)):
        runs, bad = defaultdict(list), set()
        for r in csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";"):
            try:
                runs[r["run"]].append((int(r["job"]), int(r["operation"]), int(r["machine"]),
                                       int(float(r["start"])), int(float(r["duration"]))))
            except (TypeError, ValueError, KeyError):
                bad.add(r.get("run"))   # truncated or malformed row: the whole run is skipped
        for run, ops in runs.items():
            if run in bad or not feasible(ops):
                continue
            key = tuple(sorted((j, o, st) for (j, o, _, st, _) in ops))
            if key in seen:
                continue
            seen.add(key)
            cands.append((max(st + p for (_, _, _, st, p) in ops), path, run))
    cands.sort()
    for ms, path, run in cands[:k]:
        print("%s %s %d" % (path, run, ms))


if __name__ == "__main__":
    main()
