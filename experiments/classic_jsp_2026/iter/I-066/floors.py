#!/usr/bin/env python3
"""The lowest certificate per CP-SAT hint for an instance, over the given
iterations, lowest first. Usage: floors.py <inst> <iteration> [...]"""
import glob, os, re, sys
inst = sys.argv[1]
E = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
best = {}
for it in sys.argv[2:]:
    for path in glob.glob(os.path.join(E, "iter", it, "found_hint*_found_%s_*_Certificate.csv" % inst)):
        m = re.search(r"found_hint(\d+)_found_%s_(\d+)_Certificate" % inst, os.path.basename(path))
        key, ms = (it, int(m.group(1))), int(m.group(2))
        if key not in best or ms < best[key][0]:
            best[key] = (ms, os.path.relpath(path, E))
for ms, path in sorted(best.values()):
    print("%s 1 %d" % (path, ms))
