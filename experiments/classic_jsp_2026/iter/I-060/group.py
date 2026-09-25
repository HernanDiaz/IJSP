#!/usr/bin/env python3
"""The I-060 group for ta22: the floor each CP-SAT hint of I-045 and I-058
reached (lowest certificate per hint), then the ten cold CP-SAT schedules of
I-059. Prints '<certificate> 1 <makespan>' lowest first."""
import glob, os, re
E = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
best = {}
for it in ("I-045", "I-058"):
    for path in glob.glob(os.path.join(E, "iter", it, "found_hint*_found_ta22_*_Certificate.csv")):
        m = re.search(r"found_hint(\d+)_found_ta22_(\d+)_Certificate", os.path.basename(path))
        key, ms = (it, int(m.group(1))), int(m.group(2))
        if key not in best or ms < best[key][0]:
            best[key] = (ms, os.path.relpath(path, E))
rows = sorted(best.values())
for path in sorted(glob.glob(os.path.join(E, "iter", "I-059", "cold_ta22_seed*_Certificate.csv"))):
    rows.append((int(re.search(r"_(\d+)_Certificate", path).group(1)), os.path.relpath(path, E)))
rows.sort()
for ms, path in rows:
    print("%s 1 %d" % (path, ms))
