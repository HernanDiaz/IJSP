#!/usr/bin/env python3
"""I-031: the pre-declared analysis.

Usage: analyze.py filter
       analyze.py wave <N>

Every makespan recomputed from its schedule. A POSITION iteration.

0. MECHANISM CHECK, first: the mean parent distance and the generations per
   run of each cell. far must raise the distance and near lower it, or the
   partner choice did not change.
1. PRIMARY, per treated cell against the control: the per-instance mean,
   paired Wilcoxon over the 21, symmetric Pocock boundary at six looks,
   p <= 0.0142 with one treated cell in the waves, p <= 0.0071 each with two.
2. Reported only: best-of-five.

Filter rule, per treated cell: discard it if the mean of (cell - control) over
the four filter instances exceeds +2.0. Survivors go to wave_cells.txt.
"""
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon                  # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["control", "far", "near"]
FILTER_RULE = 2.0
WAVES = 6
BLOCK = 5
STATS = ["Mean parent distance", "Number of Generations"]


def directories(tags, cell):
    out = []
    for tag in tags:
        out += sorted(glob.glob(os.path.join(EXPERIMENT, "results", "I-031_%s_p*_%s" % (tag, cell))))
    return out


def stat(tags, cell, instance, key):
    vals = []
    for d in directories(tags, cell):
        for path in glob.glob(os.path.join(d, instance + "_*.csv")):
            if path.endswith(("_Certificate.csv", "_Sols.csv")):
                continue
            for line in open(path):
                if line.startswith(key + ";"):
                    try:
                        vals.append(float(line.split(";")[1]))
                    except ValueError:
                        pass
    return statistics.mean(vals) if vals else float("nan")


def jobs_cells(tag):
    path = os.path.join(HERE, "jobs_%s.tsv" % tag)
    found = {l.split("\t")[1].rsplit("_", 1)[1] for l in open(path) if l.strip() and not l.startswith("#")}
    insts = sorted({l.split("\t")[2].strip() for l in open(path) if l.strip() and not l.startswith("#")})
    return [c for c in CELLS if c in found], insts


def best_of_blocks(values, k=BLOCK):
    blocks = [values[i:i + k] for i in range(0, len(values) - k + 1, k)]
    return statistics.mean(min(b) for b in blocks) if blocks else float("nan")


def report(tags, instances, cells):
    print("=== 0. MECHANISM CHECK ===")
    print("%-6s %-24s " % ("inst", "statistic") + " ".join("%10s" % c for c in cells))
    for i in instances:
        for key in STATS:
            print("%-6s %-24s " % (i, key) + " ".join("%10.3f" % stat(tags, c, i, key) for c in cells))
    data = {(c, i): [] for c in cells for i in instances}
    for c in cells:
        for i in instances:
            for d in directories(tags, c):
                data[(c, i)] += makespans(d, ORLIB, i)
    diffs = {}
    for c in cells[1:]:
        print("\n-- %s - control --" % c)
        md, td = [], []
        for i in instances:
            a, b = data[("control", i)], data[(c, i)]
            md.append(statistics.mean(b) - statistics.mean(a))
            td.append(best_of_blocks(b) - best_of_blocks(a))
            print("%-6s %10.1f %10.1f %+9.2f   bo5 %+8.2f" % (i, statistics.mean(a), statistics.mean(b), md[-1], td[-1]))
        diffs[c] = (md, td)
    print("\nruns per cell and instance: %d" % len(data[("control", instances[0])]))
    return diffs


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which == "filter":
        cells, insts = jobs_cells("filter")
        diffs = report(["filter"], insts, cells)
        survivors = ["control"]
        print()
        for c in cells[1:]:
            mean = statistics.mean(diffs[c][0])
            v = "DISCARD" if mean > FILTER_RULE else "PASS"
            print("filter rule, %s: mean(%s - control) = %+.2f; discard if > %+.1f -> %s" % (c, c, mean, FILTER_RULE, v))
            if v == "PASS":
                survivors.append(c)
        with open(os.path.join(HERE, "wave_cells.txt"), "w", newline="\n") as out:
            out.write("# written by analyze.py filter\n" + "\n".join(survivors) + "\n")
        print("cells for the waves: %s" % ", ".join(survivors))
    elif which == "wave":
        last = int(sys.argv[2])
        tags = ["w%d" % w for w in range(1, last + 1)]
        cells, insts = jobs_cells("w1")
        boundary = 0.0142 if len(cells) == 2 else 0.0071
        diffs = report(tags, insts, cells)
        print("\nlook %d of %d, symmetric boundary p <= %.4f per cell" % (last, WAVES, boundary))
        for c in cells[1:]:
            md, td = diffs[c]
            W, p = wilcoxon(md)
            better = sum(1 for x in md if x < 0)
            lower = better * 2 > len(md)
            print("PRIMARY %s: mean %+.2f, better on %d of %d, W = %s, p = %.4f; bo5 %+.2f"
                  % (c, statistics.mean(md), better, len(md), W, p, statistics.mean(td)))
            if p <= boundary and lower:
                print("-> %s: ACCEPT at wave %d" % (c, last))
            elif p <= boundary:
                print("-> %s: REJECT NOW, the control crosses at wave %d" % (c, last))
            elif last >= WAVES:
                print("-> %s: REJECT, the sixth wave did not cross" % c)
            else:
                print("-> %s: continue" % c)
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
