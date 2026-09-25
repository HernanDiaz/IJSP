#!/usr/bin/env python3
"""List the K lowest-makespan runs of an instance among certificates matched by
a glob, one line '<certificate> <run> <makespan>', distinct schedules only
(two runs with the same set of start times count once)."""
import csv, glob, sys
from collections import defaultdict
inst, pattern, k = sys.argv[1], sys.argv[2], int(sys.argv[3])
cands, seen = [], set()
for path in sorted(glob.glob(pattern)):
    runs = defaultdict(list)
    bad = set()
    for r in csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";"):
        try:
            runs[r["run"]].append((int(r["job"]), int(r["operation"]), int(float(r["start"])), int(float(r["duration"]))))
        except (TypeError, ValueError, KeyError):
            bad.add(r.get("run"))   # truncated or malformed row: the whole run is skipped
    for run, ops in runs.items():
        if run in bad:
            continue
        key = tuple(sorted(ops))
        if key in seen:
            continue
        seen.add(key)
        cands.append((max(s + p for (_, _, s, p) in ops), path, run))
cands.sort()
for ms, path, run in cands[:k]:
    print("%s %s %d" % (path, run, ms))
