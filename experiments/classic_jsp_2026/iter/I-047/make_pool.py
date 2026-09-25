#!/usr/bin/env python3
"""Build a seed pool for CreationJSP_Seeded from CP-SAT certificates: every
distinct schedule, sorted by makespan, written as its job sequence (the jobs of
the operations in start-time order, 1-based), annotated after ';' with the
makespan. Usage: make_pool.py <instance> <out.csv> <certificate glob> [...]"""
import csv, glob, sys
inst, out = sys.argv[1], sys.argv[2]
seen, pool = set(), []
for pattern in sys.argv[3:]:
    for path in sorted(glob.glob(pattern)):
        rows = list(csv.DictReader((l for l in open(path) if not l.startswith("#")), delimiter=";"))
        ops = sorted((int(float(r["start"])), int(r["task"]), int(r["job"])) for r in rows)
        seq = tuple(j + 1 for (_, _, j) in ops)
        if seq in seen:
            continue
        seen.add(seq)
        ms = max(int(float(r["start"])) + int(float(r["duration"])) for r in rows)
        pool.append((ms, seq))
pool.sort()
with open(out, "w") as f:
    for ms, seq in pool:
        f.write(" ".join(map(str, seq)) + " ;" + str(ms) + "\n")
print("%s: %d distinct schedules, makespans %d to %d" % (inst, len(pool), pool[0][0], pool[-1][0]))
