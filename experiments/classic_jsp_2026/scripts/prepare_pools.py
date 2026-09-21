#!/usr/bin/env python3
"""Build the seed pools an iteration uses from the read-only seed bank.

The bank (RESEARCH_IDEAS.md, "Banco de semillas") holds, per interval
instance, 1024 job permutations per generator, one per line as
"j1 j2 ... jN;[lo, up]" with jobs numbered from 1. A job permutation is a
valid solution of any instance with the same machine routing, and the
interval instances were built on the crisp Taillard routings, so the bank
serves the crisp instances unchanged; the interval annotation is only used
here to rank by quality, its midpoint being the crisp duration.

Recipes, per instance:
  v2rand    the v2 pool as it is (no selection, native order)
  v2top     the 250 best lines of v2 by E[Cmax] (interval midpoint), ascending
  v2maxmin  250 lines of v2 chosen greedily for maximum minimum positional
            Hamming distance, seeded with the best line
  mix       the bank's mix pool as it is

Usage: prepare_pools.py <bank-dir> <out-dir> <ta>...
Prints one line per file written with its md5, for the iteration record.
"""
import hashlib
import os
import random
import sys

import numpy as np

# tai -> ta, from final/ta_lb.csv of the seeding-study branch, open instances
# with a bank only
TAI = {
    "ta22": "tai20_20_02", "ta23": "tai20_20_03", "ta25": "tai20_20_05",
    "ta26": "tai20_20_06", "ta27": "tai20_20_07", "ta29": "tai20_20_09",
    "ta30": "tai20_20_10", "ta32": "tai30_15_02", "ta33": "tai30_15_03",
    "ta34": "tai30_15_04", "ta40": "tai30_15_10", "ta41": "tai30_20_01",
    "ta42": "tai30_20_02", "ta43": "tai30_20_03", "ta44": "tai30_20_04",
    "ta45": "tai30_20_05", "ta46": "tai30_20_06", "ta47": "tai30_20_07",
    "ta48": "tai30_20_08", "ta49": "tai30_20_09", "ta50": "tai30_20_10",
}
TOP = 250
random.seed(20260921)


def read_pool(path):
    """[(line, midpoint, jobs)]"""
    out = []
    with open(path, encoding="utf-8", errors="replace") as handle:
        for raw in handle:
            line = raw.rstrip("\r\n")
            if not line.strip():
                continue
            perm, _, ann = line.partition(";")
            jobs = [int(x) for x in perm.split()]
            lo, up = ann.strip().strip("[]").split(",")
            out.append((line, (float(lo) + float(up)) / 2.0, jobs))
    return out


def maxmin(entries, n):
    """Greedy max-min on positional Hamming distance, starting from the best."""
    perms = np.array([e[2] for e in entries], dtype=np.int16)
    chosen = [min(range(len(entries)), key=lambda i: entries[i][1])]
    # distance of every line to the nearest chosen one
    nearest = (perms != perms[chosen[0]]).sum(axis=1)
    nearest[chosen[0]] = -1
    while len(chosen) < n:
        pick = int(nearest.argmax())
        chosen.append(pick)
        d = (perms != perms[pick]).sum(axis=1)
        nearest = np.minimum(nearest, d)
        nearest[pick] = -1
    return [entries[i] for i in chosen]


def write(path, entries):
    with open(path, "w", newline="\n") as handle:
        for e in entries:
            handle.write(e[0] + "\n")
    h = hashlib.md5(open(path, "rb").read()).hexdigest()
    print("%s  %5d lines  md5 %s" % (os.path.relpath(path), len(entries), h))


def main():
    if len(sys.argv) < 4:
        raise SystemExit(__doc__)
    bank, out, instances = sys.argv[1], sys.argv[2], sys.argv[3:]
    os.makedirs(out, exist_ok=True)
    for ta in instances:
        tai = TAI[ta]
        v2 = read_pool(os.path.join(bank, "int__%s_v2_repo_pool.csv" % tai))
        mix = read_pool(os.path.join(bank, "int__%s_mix_repo_pool.csv" % tai))
        n = len(v2[0][2])
        for e in v2 + mix:
            if len(e[2]) != n:
                raise SystemExit("%s: a line with %d jobs, expected %d" % (ta, len(e[2]), n))
        write(os.path.join(out, "%s_v2rand.csv" % ta), v2)
        write(os.path.join(out, "%s_v2top.csv" % ta), sorted(v2, key=lambda e: e[1])[:TOP])
        write(os.path.join(out, "%s_v2maxmin.csv" % ta), maxmin(v2, TOP))
        write(os.path.join(out, "%s_mix.csv" % ta), mix)


if __name__ == "__main__":
    main()
