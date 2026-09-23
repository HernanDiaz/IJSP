#!/usr/bin/env python3
"""I-019: the pre-declared analysis of the record attempt.

Usage: analyze.py attempt

Every makespan is recomputed from its schedule (verify_certificate.py), never
read from the solver. Nothing here accepts or rejects a mechanism; the batch
is a record attempt and its endpoints are about the lower tail.

Pre-declared, in order:

1. RECORD. Any run whose verified makespan is at or below the instance's BKS.
   The proof is the schedule, so the certificate file and the run index are
   printed for anything at BKS or below, and for anything within two units of
   it so a near miss can be chased.
2. Best of N per instance and cell, and the gap of that best to the BKS.
3. The mean of the three lowest makespans per instance and cell, the
   expected-shortfall proxy the protocol asks for from I-006 on.
4. A sign test between the two cells on the best of blocks of five
   consecutive runs, which is what a record attempt actually consumes.

Reported and deciding nothing: the per-instance mean, so this batch can be
laid beside the five that came before it.
"""
import csv
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, sign_test                     # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
BOUNDS = os.path.join(EXPERIMENT, "taillard_bounds.csv")
CELLS = ["control", "best"]
BLOCK = 5
NEAR = 2


def bks_table():
    table = {}
    with open(BOUNDS, newline="") as handle:
        for row in csv.DictReader(handle):
            for key in ("best_known", "bks", "upper_bound", "ub"):
                if key in row and row[key]:
                    try:
                        table[row["instance"]] = int(float(row[key]))
                    except ValueError:
                        pass
                    break
    return table


def tags():
    seen = []
    for line in open(os.path.join(HERE, "jobs_attempt.tsv")):
        if line.strip() and not line.startswith("#"):
            tag = line.split("\t")[1].strip().split("_")[1]
            if tag not in seen:
                seen.append(tag)
    return seen


def instances():
    got = []
    for line in open(os.path.join(HERE, "jobs_attempt.tsv")):
        if line.strip() and not line.startswith("#"):
            inst = line.split("\t")[2].strip()
            if inst not in got:
                got.append(inst)
    return sorted(got)


def collect(cell, instance):
    """Every verified makespan of a cell and instance, with where it came from."""
    values = []
    for tag in tags():
        directory = os.path.join(EXPERIMENT, "results", "I-019_%s_%s" % (tag, cell))
        if not os.path.isdir(directory):
            continue
        for v in makespans(directory, ORLIB, instance):
            values.append((v, tag))
    return values


def main():
    if (sys.argv[1:2] or [""])[0] != "attempt":
        raise SystemExit(__doc__)
    bks = bks_table()
    insts = instances()
    data = {(c, i): collect(c, i) for c in CELLS for i in insts}

    print("=== 1. RECORDS and near misses (within %d of the BKS) ===" % NEAR)
    found = 0
    for inst in insts:
        target = bks.get(inst)
        for cell in CELLS:
            for value, tag in sorted(data[(cell, inst)]):
                if target is not None and value <= target + NEAR:
                    mark = "*** RECORD ***" if value <= target else "near miss"
                    print("%-6s %-9s %s  makespan %d, BKS %d, gap %+d   (chunk %s)"
                          % (inst, cell, mark, value, target, value - target, tag))
                    found += 1
    if not found:
        print("none: no run came within %d of a BKS" % NEAR)

    print("\n=== 2. best of N, and its gap to the BKS ===")
    print("%-6s %6s %s" % ("inst", "BKS", "".join("%22s" % c for c in CELLS)))
    for inst in insts:
        target = bks.get(inst)
        row = ""
        for cell in CELLS:
            v = [x for x, _ in data[(cell, inst)]]
            row += "%12d (%+4d) %4s" % (min(v), min(v) - target, "") if v and target \
                else "%22s" % "-"
        print("%-6s %6s %s" % (inst, target if target else "-", row))

    print("\n=== 3. mean of the three lowest (expected-shortfall proxy) ===")
    print("%-6s %s" % ("inst", "".join("%14s" % c for c in CELLS)))
    for inst in insts:
        row = ""
        for cell in CELLS:
            v = sorted(x for x, _ in data[(cell, inst)])
            row += "%14.1f" % statistics.mean(v[:3]) if len(v) >= 3 else "%14s" % "-"
        print("%-6s %s" % (inst, row))

    print("\n=== 4. best of blocks of %d runs, best against control ===" % BLOCK)
    wins = losses = ties = 0
    for inst in insts:
        blocks = {}
        for cell in CELLS:
            v = [x for x, _ in data[(cell, inst)]]
            blocks[cell] = [min(v[i:i + BLOCK])
                            for i in range(0, len(v) - BLOCK + 1, BLOCK)]
        pairs = list(zip(blocks["control"], blocks["best"]))
        w = sum(1 for c, f in pairs if f < c)
        l = sum(1 for c, f in pairs if f > c)
        t = sum(1 for c, f in pairs if f == c)
        wins += w; losses += l; ties += t
        print("%-6s best better on %d of %d blocks (worse %d, tied %d)"
              % (inst, w, len(pairs), l, t))
    print("pooled: best better on %d, worse on %d, tied %d -> sign test p = %.3f"
          % (wins, losses, ties, sign_test(wins, losses)))

    print("\n=== reported, deciding nothing: per-instance mean ===")
    print("%-6s %s" % ("inst", "".join("%14s" % c for c in CELLS)))
    for inst in insts:
        row = ""
        for cell in CELLS:
            v = [x for x, _ in data[(cell, inst)]]
            row += "%14.1f" % statistics.mean(v) if v else "%14s" % "-"
        print("%-6s %s" % (inst, row))
    n = len(data[(CELLS[0], insts[0])])
    print("\nruns per cell and instance: %d" % n)


if __name__ == "__main__":
    main()
