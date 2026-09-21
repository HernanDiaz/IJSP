#!/usr/bin/env python3
"""I-001: the pre-declared analysis of the filter or the full run.

Usage: analyze.py filter|full

Every makespan is recomputed from the schedule (verify_certificate.py), never
read from the solver. Primary endpoint: mix against control on the
per-instance mean of the 30 runs, Wilcoxon signed-rank paired by instance,
two-sided. Secondary, reported and not deciding: every seeded cell against
control, the expected best-of-7 over blocks of consecutive runs (what a
record attempt sees), and the step-0 best per cell (did the seeds go in).
"""
import csv
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon, sign_test          # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["control", "v2rand", "v2top", "v2maxmin", "mix"]
BLOCK = 7


def step0_best(directory, instance):
    """Best makespan at generation 0 of each run, from the solver's trace."""
    path = [p for p in glob.glob(os.path.join(directory, instance + "_[0-9]*.csv"))
            if not p.endswith("_Certificate.csv") and not p.endswith("_Sols.csv")]
    if not path:
        return None
    header, inside = None, False
    with open(path[0], newline="") as handle:
        for row in csv.reader(handle, delimiter=";"):
            if row and row[0].strip() == "Step":
                header, inside = row, True
                continue
            if inside and row and row[0].strip() == "0":
                n_runs = (len(header) - 8) // 8
                vals = []
                for r in range(1, n_runs + 1):
                    cell = row[8 * r + 4].strip().replace(",", ".") if 8 * r + 4 < len(row) else ""
                    if cell:
                        vals.append(float(cell))
                return statistics.mean(vals) if vals else None
    return None


def best_of_blocks(values, k):
    blocks = [values[i:i + k] for i in range(0, len(values) - k + 1, k)]
    return statistics.mean(min(b) for b in blocks) if blocks else float("nan")


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which not in ("filter", "full"):
        raise SystemExit(__doc__)
    jobs = [l.split("\t") for l in open(os.path.join(HERE, "jobs_%s.tsv" % which))
            if l.strip() and not l.startswith("#")]
    instances = sorted({j[2].strip() for j in jobs})
    dirs = {c: os.path.join(EXPERIMENT, "results", "I-001_%s_%s" % (which, c)) for c in CELLS}

    data = {}     # (cell, instance) -> [makespans]
    for c in CELLS:
        for inst in instances:
            data[(c, inst)] = makespans(dirs[c], ORLIB, inst)

    print("=== I-001 %s: per-instance mean of %d runs (best in brackets) ===" % (which, len(data[("control", instances[0])])))
    print("%-6s" % "inst" + "".join("%18s" % c for c in CELLS))
    for inst in instances:
        print("%-6s" % inst + "".join("%11.1f [%4d]" % (statistics.mean(data[(c, inst)]), min(data[(c, inst)])) for c in CELLS))

    print("\n=== step-0 best (mean over runs): did the seeds go in? ===")
    for inst in instances:
        print("%-6s" % inst + "".join("%18s" % ("%.0f" % step0_best(dirs[c], inst) if step0_best(dirs[c], inst) else "-") for c in CELLS))

    print("\n=== against control, paired by instance (n = %d) ===" % len(instances))
    print("%-9s %8s %8s %10s %8s   %s" % ("cell", "mean d", "better", "W", "p", "best-of-%d d" % BLOCK))
    for c in CELLS[1:]:
        d = [statistics.mean(data[(c, i)]) - statistics.mean(data[("control", i)]) for i in instances]
        db = [best_of_blocks(data[(c, i)], BLOCK) - best_of_blocks(data[("control", i)], BLOCK) for i in instances]
        W, p = wilcoxon(d)
        better = sum(1 for x in d if x < 0)
        tag = "  <-- PRIMARY" if c == "mix" else ""
        print("%-9s %+8.2f %5d/%-2d %10s %8.3f   %+8.2f%s"
              % (c, statistics.mean(d), better, len(d), W, p, statistics.mean(db), tag))

    if which == "filter":
        d = statistics.mean(statistics.mean(data[("mix", i)]) - statistics.mean(data[("control", i)]) for i in instances)
        print("\nfilter rule: mean(mix - control) over the filter instances = %+.2f; discard if > +2.0 -> %s"
              % (d, "DISCARD" if d > 2.0 else "PASS, go to the full run"))
    else:
        d = [statistics.mean(data[("mix", i)]) - statistics.mean(data[("control", i)]) for i in instances]
        W, p = wilcoxon(d)
        better = sum(1 for x in d if x < 0)
        verdict = "ACCEPT" if (p <= 0.05 and better * 2 > len(d)) else "REJECT"
        print("\ndecision (primary endpoint, mix vs control): W = %s, p = %.3f, mix better on %d of %d -> %s"
              % (W, p, better, len(d), verdict))


if __name__ == "__main__":
    main()
