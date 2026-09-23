#!/usr/bin/env python3
"""I-016: the pre-declared analysis.

Usage: analyze.py filter
       analyze.py wave <N>          analyse waves 1..N together

Every makespan is recomputed from its schedule, never read from the solver.
A POSITION iteration: the endpoint is the per-instance mean.

0. MECHANISM CHECK, first: the average makespan of generation 0 per cell, read
   from each run's trace. Both treated cells must start clearly better than
   the control, which is the only thing either of them changes. If they do
   not, the idea did not reach the run.
1. PRIMARY, per treated cell against the control: the per-instance mean,
   paired by instance, Wilcoxon, symmetric Pocock boundary at six looks,
   p <= 0.0142 when one treated cell is in the waves and p <= 0.0071 for each
   when both are, a Bonferroni split so that the two comparisons together keep
   the family's error where one would.
2. Reported and deciding nothing: best-of-five.

Filter rule, applied to each treated cell on its own: discard it if the mean
of (cell - control) over the four filter instances exceeds +2.0. The cells that
survive are written to wave_cells.txt, which both make_jobs.py and this file
read for the waves.
"""
import glob
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon                      # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["control", "circle", "uniform"]
FILTER_RULE = 2.0
WAVES = 6
BLOCK = 5


def directories(tags, cell):
    out = []
    for tag in tags:
        out += sorted(glob.glob(os.path.join(
            EXPERIMENT, "results", "I-016_%s_p*_%s" % (tag, cell))))
    return out


def gen0(tags, cell, instance):
    """Average makespan of the population at generation 0, over the runs."""
    values = []
    for directory in directories(tags, cell):
        for path in glob.glob(os.path.join(directory, instance + "_*.csv")):
            if path.endswith(("_Certificate.csv", "_Sols.csv")):
                continue
            seen = False
            for line in open(path):
                if line.startswith("Step;Runtime;"):
                    seen = True
                    continue
                if seen:
                    f = line.strip().split(";")
                    if f and f[0] == "0" and len(f) > 5:
                        values.append(float(f[5]))
                    break
    return statistics.mean(values) if values else float("nan")


def instances_of(tag):
    path = os.path.join(HERE, "jobs_%s.tsv" % tag)
    return sorted({line.split("\t")[2].strip()
                   for line in open(path)
                   if line.strip() and not line.startswith("#")})


def cells_in(tag):
    path = os.path.join(HERE, "jobs_%s.tsv" % tag)
    found = {line.split("\t")[1].rsplit("_", 1)[1]
             for line in open(path) if line.strip() and not line.startswith("#")}
    return [c for c in CELLS if c in found]


def best_of_blocks(values, k=BLOCK):
    blocks = [values[i:i + k] for i in range(0, len(values) - k + 1, k)]
    return statistics.mean(min(b) for b in blocks) if blocks else float("nan")


def report(tags, instances, cells, label):
    data = {(c, i): [] for c in cells for i in instances}
    for c in cells:
        for i in instances:
            for d in directories(tags, c):
                data[(c, i)] += makespans(d, ORLIB, i)

    print("=== 0. MECHANISM CHECK: average makespan of generation 0 ===")
    print("%-6s " % "inst" + " ".join("%10s" % c for c in cells))
    starts = {c: [] for c in cells}
    for i in instances:
        row = []
        for c in cells:
            v = gen0(tags, c, i)
            starts[c].append(v)
            row.append("%10.1f" % v)
        print("%-6s " % i + " ".join(row))
    for c in cells[1:]:
        gap = statistics.mean(a - b for a, b in zip(starts[c], starts["control"]))
        print("%s starts %+.1f against the control on average" % (c, gap))
        if gap > -20:
            print("WARNING: %s does not start clearly better; the idea did not "
                  "reach the run." % c)

    print("\n=== I-016 %s: per-instance mean (primary), best-of-%d (reported) ==="
          % (label, BLOCK))
    diffs = {}
    for c in cells[1:]:
        print("\n-- %s - control --" % c)
        print("%-6s %10s %10s %9s   %10s %10s %9s"
              % ("inst", "mean ctrl", "mean " + c[:5], "d", "bo5 ctrl", "bo5 " + c[:5], "d"))
        md, td = [], []
        for i in instances:
            a, b = data[("control", i)], data[(c, i)]
            ma, mb = statistics.mean(a), statistics.mean(b)
            ta, tb = best_of_blocks(a), best_of_blocks(b)
            md.append(mb - ma)
            td.append(tb - ta)
            print("%-6s %10.1f %10.1f %+9.2f   %10.1f %10.1f %+9.2f"
                  % (i, ma, mb, mb - ma, ta, tb, tb - ta))
        diffs[c] = (md, td)
    print("\nruns per cell and instance: %d" % len(data[("control", instances[0])]))
    return diffs


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which == "filter":
        instances = instances_of("filter")
        diffs = report(["filter"], instances, CELLS, "filter")
        survivors = ["control"]
        print()
        for c in CELLS[1:]:
            mean = statistics.mean(diffs[c][0])
            verdict = "DISCARD" if mean > FILTER_RULE else "PASS"
            print("filter rule, %s: mean(%s - control) = %+.2f; discard if > %+.1f -> %s"
                  % (c, c, mean, FILTER_RULE, verdict))
            if verdict == "PASS":
                survivors.append(c)
        with open(os.path.join(HERE, "wave_cells.txt"), "w", newline="\n") as out:
            out.write("# written by analyze.py filter\n" + "\n".join(survivors) + "\n")
        print("cells for the waves: %s" % ", ".join(survivors))
    elif which == "wave":
        try:
            last = int(sys.argv[2])
        except (IndexError, ValueError):
            raise SystemExit(__doc__)
        tags = ["w%d" % w for w in range(1, last + 1)]
        instances = instances_of("w1")
        cells = cells_in("w1")
        boundary = 0.0142 if len(cells) == 2 else 0.0071
        diffs = report(tags, instances, cells, "waves 1..%d" % last)
        print("\nlook %d of %d, symmetric boundary p <= %.4f per cell (%d treated)"
              % (last, WAVES, boundary, len(cells) - 1))
        for c in cells[1:]:
            md, td = diffs[c]
            W, p = wilcoxon(md)
            better = sum(1 for x in md if x < 0)
            lower = better * 2 > len(md)
            print("PRIMARY %s: mean %+.2f, better on %d of %d, W = %s, p = %.4f; "
                  "best-of-%d %+.2f"
                  % (c, statistics.mean(md), better, len(md), W, p, BLOCK,
                     statistics.mean(td)))
            if p <= boundary and lower:
                print("-> %s: ACCEPT, crosses on the lower rank at wave %d" % (c, last))
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
