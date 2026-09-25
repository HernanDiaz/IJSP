#!/usr/bin/env python3
"""One schedule per CP-SAT hint: for every hint of I-045, I-046 and I-053 on an
instance, its lowest certificate (the floor CP-SAT reached from that hint).
Prints '<certificate> 1 <makespan>' sorted by makespan. Usage: floors.py <inst>"""
import glob, os, re, sys
inst = sys.argv[1]
E = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
best = {}
for it in ("I-045", "I-046", "I-053"):
    for path in glob.glob(os.path.join(E, "iter", it, "found_hint*_found_%s_*_Certificate.csv" % inst)):
        m = re.search(r"found_hint(\d+)_found_%s_(\d+)_Certificate" % inst, os.path.basename(path))
        key, ms = (it, int(m.group(1))), int(m.group(2))
        if key not in best or ms < best[key][0]:
            best[key] = (ms, os.path.relpath(path, E))
for ms, path in sorted(best.values()):
    print("%s 1 %d" % (path, ms))
