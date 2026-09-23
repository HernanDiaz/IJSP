#!/usr/bin/env python3
"""Any record or match in I-014's 1260 confirmation runs?

A record is strictly below the best known solution; equalling it is a match.
Every makespan is recomputed from its schedule.
"""
import csv
import glob
import os
import sys

EXPERIMENT = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans                                  # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
BKS = {}
for row in csv.DictReader(open(os.path.join(EXPERIMENT, "taillard_bounds.csv"))):
    BKS[row["instance"]] = int(row["bks"])

INSTANCES = sorted({line.split("\t")[2].strip()
                    for line in open(os.path.join(EXPERIMENT, "iter", "I-014",
                                                  "jobs_w1.tsv"))
                    if line.strip() and not line.startswith("#")})

total = 0
print("%-6s %6s %6s %6s %s" % ("inst", "best", "BKS", "gap", "verdict"))
for instance in INSTANCES:
    best = None
    for cell in ("control", "restart"):
        for directory in sorted(glob.glob(os.path.join(
                EXPERIMENT, "results", "I-014_w*_p*_%s" % cell))):
            for v in makespans(directory, ORLIB, instance):
                total += 1
                if best is None or v < best:
                    best = v
    bks = BKS[instance]
    verdict = ("RECORD" if best < bks else
               "match" if best == bks else "")
    print("%-6s %6d %6d %+5.2f%% %s"
          % (instance, best, bks, 100.0 * (best - bks) / bks, verdict))
print("\n%d runs recomputed from their schedules" % total)
