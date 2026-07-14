#!/usr/bin/env python3
"""
probe_table.py — builds the paper's conflict-probe table (tab:probe) from the
epsilon_probe CSVs. Reports, per instance, the LOWER-ENVELOPE NPE at selected
makespan allowances and the drop from the tightest solved point to the
envelope minimum. Sources: results/fronts.csv (original probes) and
results/fronts_ext.csv (review extension; la29 re-probe supersedes the
original la29 rows, which are UNKNOWN below +6%).
"""
import csv, os
HERE = os.path.dirname(os.path.abspath(__file__))

def load(path, skip=()):
    rows = {}
    if not os.path.exists(path):
        return rows
    with open(path, newline="") as fh:
        for r in csv.DictReader(fh):
            stem = r["instance"]
            if stem in skip:
                continue
            v = float(r["npe_x2"])
            if v < 0:
                continue
            rows.setdefault(stem, {})[int(r["eps_permil"])] = v / 2
    return rows

data = load(os.path.join(HERE, "results", "fronts.csv"),
            skip={"F0.15.0.la29_03"})          # superseded by the re-probe
data.update(load(os.path.join(HERE, "results", "fronts_ext.csv")))

ORDER = ["F0.15.0.ft10_10", "F0.15.0.ft20_05", "F0.15.0.la21_04",
         "F0.15.0.la24_03", "F0.15.0.la25_04", "F0.15.0.la29_03",
         "F0.15.0.abz7_06", "tai15_15_01.F.15_01", "tai15_15_02.F.15_01"]

print(f"{'instance':22} {'first(pm)':>9} {'e0':>7} {'+3%':>7} {'+5%':>7} "
      f"{'+10%':>7} {'drop%':>6}")
for stem in ORDER:
    if stem not in data:
        print(f"{stem:22}  (sin datos aun)")
        continue
    d = data[stem]
    solved = sorted(d)
    # lower envelope: env[pm] = min over solved points with eps <= pm
    env, best = {}, float("inf")
    for pm in solved:
        best = min(best, d[pm])
        env[pm] = best
    def at(target):
        elig = [pm for pm in solved if pm <= target]
        return env[max(elig)] if elig else None
    first = solved[0]
    base = env[first]
    drop = (base - min(env.values())) / base * 100 if base > 0 else 0.0
    fmt = lambda x: f"{x:7.0f}" if x is not None else "     --"
    print(f"{stem:22} {first:>9} {fmt(at(0))} {fmt(at(30))} {fmt(at(50))} "
          f"{fmt(at(100))} {drop:6.1f}")
