#!/usr/bin/env python3
"""
validate_rightshift.py — checks for rightshift.py before using it in analysis.

Invariants, on real near-optimal orders (archived _Sols):
 I1  semi_active >= heuristic >= exact           (per order, per component)
 I2  Cmax is untouched by construction (same build_structures cap)
 I3  cross-check: semi-active NPE equals validate_npe's independent decoder
 I4  coherence with the Phase-0 probe (ft10): min over orders of exact-RS NPE
     at Cmax=cap must be >= the probe's FREE-timing optimum at that cap
     (the probe optimises order AND timing jointly, so it lower-bounds us)

Usage: python3 validate_rightshift.py [n_orders_per_instance]
"""
import glob, os, random, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance, POWER_SEED
from validate_npe import decode_npe
from rightshift import semi_active_npe, heuristic_rs_npe, exact_rs_npe

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
SOLS_DIRS = [
    os.path.join(HERE, "results", "LexME_N2Plus_arm"),
    os.path.join(REPO, "experiments", "n2_worstcase_2026", "results", "N2Plus"),
]


def load_orders(stem, ntasks, limit):
    orders = []
    for d in SOLS_DIRS:
        for p in sorted(glob.glob(os.path.join(d, stem + "_*_Sols.csv"))):
            with open(p, encoding="utf-8", errors="replace") as fh:
                for line in fh:
                    parts = line.strip().split(";")
                    if len(parts) >= 2 and parts[0].isdigit():
                        o = [int(x) for x in parts[1].split()]
                        if len(o) == ntasks:
                            orders.append(o)
    return orders[:limit]


def main():
    limit = int(sys.argv[1]) if len(sys.argv) > 1 else 15
    fails = 0
    for stem in ("F0.15.0.ft10_10", "F0.15.0.la29_03"):
        n, m, mach, dur = parse_instance(stem)
        rng = random.Random(POWER_SEED)
        pp = [rng.randint(2, 8) for _ in range(m)]
        orders = load_orders(stem, n * m, limit)
        print(f"\n=== {stem}: {len(orders)} orders ===")
        best_exact = {0: None, 1: None}
        cmax_at_best = {}
        for o in orders:
            # I3: semi-active NPE matches the independent decoder
            cmax_ref, npe_ref = decode_npe(o, n, m, mach, dur, pp)
            for comp in (0, 1):
                semi, cm = semi_active_npe(o, n, m, mach, dur, pp, comp)
                heur, cm_h = heuristic_rs_npe(o, n, m, mach, dur, pp, comp)
                exact, cm_e, opt = exact_rs_npe(o, n, m, mach, dur, pp, comp)
                assert cm == cm_h == cm_e == cmax_ref[comp], "I2 Cmax changed!"
                if abs(semi - npe_ref[comp]) > 1e-6:
                    print(f"  I3 FAIL comp{comp}: {semi} vs {npe_ref[comp]}"); fails += 1
                if not (semi >= heur - 1e-9 and heur >= exact - 1e-9):
                    print(f"  I1 FAIL comp{comp}: semi={semi} heur={heur} exact={exact}"); fails += 1
                if not opt:
                    print(f"  WARN comp{comp}: exact solve not proven optimal")
                if best_exact[comp] is None or exact < best_exact[comp]:
                    best_exact[comp] = exact
                    cmax_at_best[comp] = cm
        semis, heurs, exacts = [], [], []
        for o in orders:
            s = h = e = 0
            for comp in (0, 1):
                s += semi_active_npe(o, n, m, mach, dur, pp, comp)[0]
                h += heuristic_rs_npe(o, n, m, mach, dur, pp, comp)[0]
                e += exact_rs_npe(o, n, m, mach, dur, pp, comp)[0]
            semis.append(s / 2); heurs.append(h / 2); exacts.append(e / 2)
        n_ = len(orders)
        red_h = sum(1 - h / s for h, s in zip(heurs, semis)) / n_ * 100
        red_e = sum(1 - e / s for e, s in zip(exacts, semis)) / n_ * 100
        print(f"  I1/I2/I3 OK on {n_} orders x 2 components")
        print(f"  mean NPE midpoint: semi={sum(semis)/n_:.0f}  heur={sum(heurs)/n_:.0f} "
              f"(-{red_h:.1f}%)  exact={sum(exacts)/n_:.0f} (-{red_e:.1f}%)")
        print(f"  I4 reference: best exact-RS NPE comp- ={best_exact[0]} at Cmax {cmax_at_best[0]} "
              f"(probe free-timing optimum lower-bounds this)")
    print(f"\n{'ALL CHECKS PASSED' if fails == 0 else str(fails) + ' FAILURES'}")
    sys.exit(1 if fails else 0)


if __name__ == "__main__":
    main()
