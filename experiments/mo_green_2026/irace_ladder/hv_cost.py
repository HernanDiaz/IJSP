#!/usr/bin/env python3
"""
hv_cost.py <merged_front.csv> <cmax_ref> <npe_ref>
Prints -HV (irace minimises) of the front's non-dominated midpoints w.r.t.
the fixed reference point. Empty/absent front -> 0.0 (worst).
"""
import sys


def nondominated(pts):
    return sorted(set(p for p in pts if not any(
        q[0] <= p[0] and q[1] <= p[1] and q != p for q in pts)))


def hv(pts, ref):
    h, prev = 0.0, ref[1]
    for c, e in sorted(pts):
        if c >= ref[0] or e >= prev:
            continue
        h += (ref[0] - c) * (prev - e)
        prev = e
    return h


def main():
    path, cref, eref = sys.argv[1], float(sys.argv[2]), float(sys.argv[3])
    pts = []
    try:
        with open(path) as fh:
            for line in fh:
                p = line.strip().split(";")
                if len(p) >= 4 and p[0].replace(".", "").replace("-", "").isdigit():
                    cl, ch, nl, nh = (float(p[0]), float(p[1]),
                                      float(p[2]), float(p[3]))
                    pts.append(((cl + ch) / 2, (nl + nh) / 2))
    except FileNotFoundError:
        pass
    if not pts:
        print("0.0"); return
    print(f"{-hv(nondominated(pts), (cref, eref)):.1f}")


if __name__ == "__main__":
    main()
