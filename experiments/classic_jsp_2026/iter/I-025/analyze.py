#!/usr/bin/env python3
"""I-025 (B-1): k short runs against one long run, at equal CPU, on data that
already exists. Nothing is run.

Short side: I-001's full control cell, the frozen configuration of the time at
the per-class budgets L (40 s for 20x20, 100 s for 30x15, 150 s for 30x20),
30 independent runs per instance. Long side: results/prereg2_abc, the same
configuration at 300 s, 10 runs per instance.

Equal CPU: one 300 s run against the best of k = floor(300 / L) short runs
(7, 3 and 2). The 30 short runs give disjoint blocks of k, so 4, 10 and 15
equal-CPU samples; the long side gives 10. Reported per instance as a
distribution, as the external review asked: median, best and worst of the
equal-CPU samples on each side. Aggregate: the paired Wilcoxon on the
per-instance medians, and how many instances each side wins.

Caveat, declared before reading any number: the two batches ran on different
days, and this machine drifts about 9 % in throughput between batches
(memory note, and scripts/paired_compare.sh exists for that reason). A budget
in wall-clock seconds turns that drift into search effort, so this comparison
is descriptive and decides nothing.
"""
import glob
import os
import statistics
import sys

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(E, "scripts"))
from compare import makespans, wilcoxon                  # noqa: E402

ORLIB = os.path.join(E, "reference", "taillard_orlib.txt")
SHORT = os.path.join(E, "results", "I-001_full_control")
LONG = os.path.join(E, "results", "prereg2_abc")
L = {"20x20": 40, "30x15": 100, "30x20": 150}
SIZE = {}
for row in open(os.path.join(E, "taillard_bounds.csv")):
    f = row.strip().split(",")
    if f[0] != "instance":
        SIZE[f[0]] = f[1] + "x" + f[2]

instances = sorted({os.path.basename(p).split("_")[0]
                    for p in glob.glob(os.path.join(SHORT, "*_Certificate.csv"))}
                   & {os.path.basename(p).split("_")[0]
                      for p in glob.glob(os.path.join(LONG, "*_Certificate.csv"))})

print("%-6s %5s %2s | %-26s | %-26s | %s"
      % ("inst", "class", "k", "best-of-k short: med best worst (n)",
         "300 s long: med best worst (n)", "median d (short - long)"))
dmed, short_wins, long_wins = [], 0, 0
for inst in instances:
    cls = SIZE[inst]
    k = 300 // L[cls]
    s = makespans(SHORT, ORLIB, inst)
    lng = makespans(LONG, ORLIB, inst)
    blocks = [min(s[i:i + k]) for i in range(0, len(s) - k + 1, k)]
    ms, ml = statistics.median(blocks), statistics.median(lng)
    dmed.append(ms - ml)
    if ms < ml:
        short_wins += 1
    elif ml < ms:
        long_wins += 1
    print("%-6s %5s %2d | %6.1f %5d %5d (%2d)        | %6.1f %5d %5d (%2d)        | %+6.1f"
          % (inst, cls, k, ms, min(blocks), max(blocks), len(blocks),
             ml, min(lng), max(lng), len(lng), ms - ml))

W, p = wilcoxon(dmed)
print("\n%d instances: restart (best of k short) better on %d, the long run on %d, "
      "ties %d" % (len(instances), short_wins, long_wins,
                   len(instances) - short_wins - long_wins))
print("median difference, mean over instances %+.2f; Wilcoxon W = %s, p = %.4f"
      % (statistics.mean(dmed), W, p))
print("descriptive only: two batches on different days, ~9 % throughput drift")
