#!/usr/bin/env python3
"""
front_distance.py — Exp. 2 analysis: how close does each arm get to the
exact (midpoint-proxy) epsilon-constraint fronts?

For every probed instance:
- Builds the LOWER ENVELOPE of the front points in results/fronts.csv
  (time-limited FEASIBLE points can be non-monotone; the envelope is the
  publishable curve). Probe values are x2-scaled -> divided by 2 here.
- Evaluates each arm's 30 per-run best solutions (published N2, LexME,
  LexME+RS heuristic) as (Cmax_mid, NPE_mid) points.
- Reports, per instance and arm, the mean gap to the front:
  gap% = (NPE / front(Cmax) - 1) * 100, where front(Cmax) is the envelope
  value at the largest probed cap <= Cmax.

Caveats (stated in the paper): fronts are exact for the MIDPOINT scenario
(crisp proxy), and mid(C-,C+) >= crisp-mid Cmax by convexity of the longest
path, so the lookup is well defined; gaps are indicative, not certificates.

Also writes pgfplots-ready .dat files under results/fronts_dat/:
<instance>_front.dat (envelope) and <instance>_arms.dat (mean arm points).

Run:  python3 front_distance.py         (stdlib only)
"""
import csv, glob, os, random, re, statistics as st, sys
from collections import defaultdict

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from conflict_check import parse_instance, POWER_SEED
from validate_npe import decode_npe
from rightshift import heuristic_rs_npe

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.normpath(os.path.join(HERE, "..", ".."))
FRONTS = os.path.join(HERE, "results", "fronts.csv")
DATDIR = os.path.join(HERE, "results", "fronts_dat")
ARMS = [
    ("N2-pub", "/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp4/N2_tuned"
     if os.name != "nt" else r"C:\Users\diazhernan\CLionProjects\IJSP\experiments\results\exp4\N2_tuned",
     False),
    ("LexME", os.path.join(HERE, "results", "LexME"), False),
    ("LexME+RS", os.path.join(HERE, "results", "LexME"), True),
]
STEM_RE = re.compile(r"^(.+)_(\d{14})_Sols\.csv$")


def load_front(path):
    """{instance: [(cmax, npe) envelope, cmax-sorted]}"""
    pts = defaultdict(list)
    with open(path, newline="") as fh:
        for row in csv.DictReader(fh):
            npe = float(row["npe_x2"])
            if npe < 0:
                continue
            pts[row["instance"]].append(
                (float(row["cmax_cap_x2"]) / 2, npe / 2))
    env = {}
    for inst, ps in pts.items():
        ps.sort()
        out, best = [], float("inf")
        for c, e in ps:
            if e < best:
                best = e
                out.append((c, best))
        env[inst] = out
    return env


def front_at(env, c):
    val = None
    for cc, ee in env:
        if cc <= c:
            val = ee
        else:
            break
    return val


def latest_sols(directory, stem):
    latest = None
    for p in glob.glob(os.path.join(directory, stem + "_*_Sols.csv")):
        m = STEM_RE.match(os.path.basename(p))
        if m and (latest is None or m.group(2) > latest[0]):
            latest = (m.group(2), p)
    if latest is None:
        return []
    orders = []
    with open(latest[1], encoding="utf-8", errors="replace") as fh:
        for line in fh:
            parts = line.strip().split(";")
            if len(parts) >= 2 and parts[0].isdigit():
                orders.append([int(x) for x in parts[1].split()])
    return orders


def arm_points(stem, directory, with_rs):
    n, m, mach, dur = parse_instance(stem)
    rng = random.Random(POWER_SEED)
    pp = [rng.randint(2, 8) for _ in range(m)]
    pts = []
    for o in latest_sols(directory, stem):
        if len(o) != n * m:
            continue
        cmax, npe = decode_npe(o, n, m, mach, dur, pp)
        if with_rs:
            lo = heuristic_rs_npe(o, n, m, mach, dur, pp, 0)[0]
            hi = heuristic_rs_npe(o, n, m, mach, dur, pp, 1)[0]
            npe = (lo, hi)
        pts.append(((cmax[0] + cmax[1]) / 2, (npe[0] + npe[1]) / 2))
    return pts


def main():
    env = load_front(FRONTS)
    os.makedirs(DATDIR, exist_ok=True)
    print("=" * 86)
    print("Distance to the exact (midpoint-proxy) front — mean over 30 runs per arm")
    print("gap% = (NPE / front(Cmax) - 1) * 100   |   lower is better; 0 = on the front")
    print("=" * 86)
    print(f"{'Instance':22} " + "".join(f"{name:>14}" for name, _, _ in ARMS))
    for inst in sorted(env):
        with open(os.path.join(DATDIR, inst + "_front.dat"), "w") as fh:
            fh.write("cmax npe\n")
            for c, e in env[inst]:
                fh.write(f"{c} {e}\n")
        row = f"{inst:22} "
        arm_means = []
        for name, d, rs in ARMS:
            pts = arm_points(inst, d, rs)
            gaps = []
            for c, e in pts:
                f = front_at(env[inst], c)
                if f and f > 0:
                    gaps.append((e / f - 1) * 100)
            if gaps:
                row += f"{st.mean(gaps):>13.1f}%"
                arm_means.append((name, st.mean(p[0] for p in pts),
                                  st.mean(p[1] for p in pts)))
            else:
                row += f"{'--':>14}"
        print(row)
        with open(os.path.join(DATDIR, inst + "_arms.dat"), "w") as fh:
            fh.write("arm cmax npe\n")
            for name, c, e in arm_means:
                fh.write(f"{name} {c:.1f} {e:.1f}\n")
    print("\n.dat files for pgfplots written to results/fronts_dat/")


if __name__ == "__main__":
    main()
