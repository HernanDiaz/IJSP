#!/usr/bin/env python3
"""Print '<certificate> <run> <makespan>' of the lowest-makespan run of an
instance among the certificates matched by a glob (makespan recomputed as the
largest completion in the schedule)."""
import csv, glob, sys
from collections import defaultdict
inst, pattern = sys.argv[1], sys.argv[2]
best = None
for path in sorted(glob.glob(pattern)):
    runs = defaultdict(int)
    for r in csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";"):
        runs[r["run"]] = max(runs[r["run"]], int(float(r["start"])) + int(float(r["duration"])))
    for run, ms in runs.items():
        if best is None or ms < best[2]:
            best = (path, run, ms)
print("%s %s %d" % best)
