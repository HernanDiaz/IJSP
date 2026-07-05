#!/usr/bin/env python3
"""
front_metrics.py — quality metrics for _Front.csv archives (Exp. 4).

For each given _Front.csv (algorithm archive; interval bounds per row,
midpoints used for the 2D metrics) and its instance's exact
(midpoint-proxy) front envelope from results/fronts.csv:

- HV: hypervolume of the approximation and of the exact envelope, w.r.t. a
  reference point 1.05x the nadir of the union; HV ratio (1.0 = matches
  the exact front's hypervolume).
- eps+: additive epsilon-indicator — the smallest shift e such that every
  exact-front point is weakly dominated by some approximation point
  shifted by e (in both objectives, normalised units of the union range).

Usage: python3 front_metrics.py <front_csv> <instance_stem> [more pairs...]
"""
import csv, os, sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
HERE = os.path.dirname(os.path.abspath(__file__))
FRONTS = os.path.join(HERE, "results", "fronts.csv")


def load_exact(stem):
    pts = []
    with open(FRONTS, newline="") as fh:
        for row in csv.DictReader(fh):
            if row["instance"] != stem or float(row["npe_x2"]) < 0:
                continue
            pts.append((float(row["cmax_cap_x2"]) / 2,
                        float(row["npe_x2"]) / 2))
    pts.sort()
    env, best = [], float("inf")
    for c, e in pts:
        if e < best:
            best = e
            env.append((c, e))
    return env


def load_front(path):
    pts = []
    with open(path, newline="") as fh:
        for row in csv.DictReader(fh, delimiter=";"):
            pts.append(((float(row["cmax_lo"]) + float(row["cmax_hi"])) / 2,
                        (float(row["npe_lo"]) + float(row["npe_hi"])) / 2))
    return nondominated(pts)


def nondominated(pts):
    nd = [p for p in pts if not any(
        q[0] <= p[0] and q[1] <= p[1] and q != p for q in pts)]
    return sorted(set(nd))


def hypervolume(pts, ref):
    """2D HV (minimisation), pts sorted by first objective."""
    hv, prev_e = 0.0, ref[1]
    for c, e in sorted(pts):
        if c >= ref[0] or e >= prev_e:
            continue
        hv += (ref[0] - c) * (prev_e - e)
        prev_e = e
    return hv


def eps_indicator(approx, exact, ranges):
    """Additive eps (normalised): min shift so approx weakly dominates exact."""
    eps = 0.0
    for ec, ee in exact:
        best = min(max((ac - ec) / ranges[0], (ae - ee) / ranges[1])
                   for ac, ae in approx)
        eps = max(eps, best)
    return eps


def main():
    args = sys.argv[1:]
    print(f"{'front file':45} {'n':>3} {'HV ratio':>9} {'eps+':>7}")
    for i in range(0, len(args), 2):
        path, stem = args[i], args[i + 1]
        exact = load_exact(stem)
        approx = load_front(path)
        if not exact or not approx:
            print(f"{os.path.basename(path):45}  -- (missing data)")
            continue
        union = exact + approx
        nad = (max(p[0] for p in union), max(p[1] for p in union))
        ideal = (min(p[0] for p in union), min(p[1] for p in union))
        ref = (nad[0] * 1.05, nad[1] * 1.05)
        ranges = (max(nad[0] - ideal[0], 1e-9), max(nad[1] - ideal[1], 1e-9))
        hv_a = hypervolume(approx, ref)
        hv_e = hypervolume(exact, ref)
        eps = eps_indicator(approx, exact, ranges)
        print(f"{os.path.basename(path):45} {len(approx):>3} "
              f"{hv_a / hv_e if hv_e > 0 else float('nan'):>9.3f} {eps:>7.3f}")


if __name__ == "__main__":
    main()
