#!/usr/bin/env python3
"""Emit per-instance anchor Cmax bounds (best of the LexME arm's 30 runs,
per component) for the ladder script: '<stem> <c_lo> <c_hi>' lines."""
import os, sys
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from analyze_lexme import load_sols_orders
from conflict_check import parse_instance
from validate_npe import decode_npe
import random
from conflict_check import POWER_SEED

HERE = os.path.dirname(os.path.abspath(__file__))
LEX = load_sols_orders(os.path.join(HERE, "results", "LexME"))
for stem in sys.argv[1:]:
    n, m, mach, dur = parse_instance(stem)
    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]
    best = None
    for o in LEX.get(stem, []):
        if len(o) != n * m:
            continue
        cmax, _ = decode_npe(o, n, m, mach, dur, pp)
        if best is None or (cmax[1], cmax[0]) < (best[1], best[0]):
            best = cmax
    print(f"{stem} {best[0]:.0f} {best[1]:.0f}")
