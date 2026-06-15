#!/usr/bin/env python3
"""Diversity study of the warm-start seed bank.

For each instance, the merged N2+N8 _Sols.csv holds ~60 best-of-run task
orders (the warm-seed bank). This script characterises how diverse that bank
is: distinct solutions, makespan spread, and mean pairwise structural
distance (normalised Kendall-tau on the task orders).

Caveat: the task-order Kendall-tau is a PROXY for the disjunctive distance
the path relinking actually uses (which needs the decoded machine
sequences). It overestimates diversity, so low values here = genuinely
clustered.

Usage: python3 seed_diversity.py [seeds_dir] [instance_glob_prefixes...]
Default: the 20 x20 Taillard instances.
"""
import glob
import os
import re
import sys
import statistics

HERE = os.path.dirname(os.path.abspath(__file__))
SEEDS = sys.argv[1] if len(sys.argv) > 1 else os.path.join(
    HERE, "..", "seeds", "N2N8_tuned")
PREFIXES = sys.argv[2:] if len(sys.argv) > 2 else ["tai30_20", "tai50_20"]


def parse_sols(path):
    """Return list of (task_order_tuple, objective_midpoint)."""
    sols = []
    with open(path) as fh:
        for line in fh:
            parts = line.rstrip("\n").split(";")
            if len(parts) < 3:
                continue
            tasks = parts[1].split()
            if not tasks or not tasks[0].lstrip("-").isdigit():
                continue          # header or malformed line
            order = tuple(int(t) for t in tasks)
            m = re.search(r"\(\s*(\d+)\s*,\s*(\d+)\s*\)", parts[-1])
            mid = (int(m.group(1)) + int(m.group(2))) / 2.0 if m else float("nan")
            sols.append((order, mid))
    return sols


def kendall_norm(a, b):
    """Normalised Kendall-tau distance between two permutations of the same
    set: fraction of pairs in different relative order (0=identical, 1=reversed)."""
    pos = {v: i for i, v in enumerate(b)}
    seq = [pos[v] for v in a]                 # a's order, indexed by b's positions
    # count inversions in seq via merge sort
    n = len(seq)
    if n < 2:
        return 0.0
    inv = [0]

    def sort(lo, hi):
        if hi - lo < 2:
            return seq[lo:hi]
        mid = (lo + hi) // 2
        left, right = sort(lo, mid), sort(mid, hi)
        merged, i, j = [], 0, 0
        while i < len(left) and j < len(right):
            if left[i] <= right[j]:
                merged.append(left[i]); i += 1
            else:
                merged.append(right[j]); j += 1
                inv[0] += len(left) - i
        merged.extend(left[i:]); merged.extend(right[j:])
        return merged

    sort(0, n)
    return inv[0] / (n * (n - 1) / 2)


def study(path):
    sols = parse_sols(path)
    if not sols:
        return None
    orders = [s[0] for s in sols]
    objs = [s[1] for s in sols]
    n = len(orders)
    distinct = len(set(orders))
    obj_distinct = len(set(objs))
    # mean pairwise distance (sample if too many pairs)
    dists = []
    for i in range(n):
        for j in range(i + 1, n):
            dists.append(kendall_norm(orders[i], orders[j]))
    mean_d = statistics.mean(dists) if dists else 0.0
    min_d = min(dists) if dists else 0.0
    max_d = max(dists) if dists else 0.0
    return dict(n=n, distinct=distinct, obj_distinct=obj_distinct,
                obj_min=min(objs), obj_max=max(objs),
                obj_spread=max(objs) - min(objs),
                mean_d=mean_d, min_d=min_d, max_d=max_d)


def main():
    files = []
    for pref in PREFIXES:
        files += sorted(glob.glob(os.path.join(SEEDS, pref + "_*_Sols.csv")))
    if not files:
        sys.exit("No seed files found under %s for %s" % (SEEDS, PREFIXES))
    print("%-26s %4s %5s %5s %9s %7s %7s %7s" % (
        "instance", "n", "uniq", "objU", "objSpread", "meanD", "minD", "maxD"))
    agg = []
    for f in files:
        r = study(f)
        if not r:
            continue
        stem = os.path.basename(f).replace("_Sols.csv", "")
        agg.append(r)
        print("%-26s %4d %5d %5d %9.1f %7.3f %7.3f %7.3f" % (
            stem, r["n"], r["distinct"], r["obj_distinct"], r["obj_spread"],
            r["mean_d"], r["min_d"], r["max_d"]))
    if agg:
        print("-" * 78)
        print("%-26s %4s %5.1f %5.1f %9s %7.3f" % (
            "MEAN", "",
            statistics.mean(a["distinct"] for a in agg),
            statistics.mean(a["obj_distinct"] for a in agg),
            "",
            statistics.mean(a["mean_d"] for a in agg)))
        print("\nReading: meanD ~0 and few unique = clustered bank (warm seeding "
              "cannot diversify the pool). meanD high and many unique = diverse "
              "bank worth max-min seeding.")


if __name__ == "__main__":
    main()
