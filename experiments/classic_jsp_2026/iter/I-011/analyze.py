#!/usr/bin/env python3
"""I-011 [TAIL]: the pre-declared analysis of the portfolio.

Usage: analyze.py filter
       analyze.py wave <N>          analyse waves 1..N together

Every makespan is recomputed from its schedule, never read from the solver.

This is a TAIL iteration, so the endpoints are not the ones the other eleven
used:

0. MECHANISM CHECK, printed first and read first. The portfolio is supposed to
   widen the distribution, so the per-instance standard deviation must rise
   above the control's. If it does not, the mixture is not mixing and the
   hypothesis was never tested -- the lesson I-009 paid four redesigns for.
1. PRIMARY: the mean of the bests over blocks of five consecutive runs, paired
   by instance, Wilcoxon, symmetric Pocock boundary p <= 0.0142 at each of six
   looks. Best-of-five is what a record campaign consumes and costs about 0.7
   units of aggregate standard error, against 1.0 and skewed for the outright
   minimum.
2. Reported and deciding nothing: the per-instance mean, which this iteration
   is explicitly ALLOWED to worsen, and the outright best.

The filter rule applies to the primary, not to the mean: discard if the mean
of (portfolio - control) on best-of-five over the four filter instances
exceeds +2.0.
"""
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon                      # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["control", "portfolio"]
POCOCK = 0.0142
FILTER_RULE = 2.0
WAVES = 6
BLOCK = 5


def cells_of(tags, instances):
    """Every chunk has its own results directory, so a cell is the union of
    I-011_<tag>_p<NN>_<cell> over the chunks. One directory per chunk is not a
    detail: sharing one made concurrent jobs on the same instance collide on
    the solver's one-second timestamp filenames and overwrite each other."""
    import glob
    data = {}
    for cell in CELLS:
        for instance in instances:
            values = []
            for tag in tags:
                pattern = os.path.join(EXPERIMENT, "results",
                                       "I-011_%s_p*_%s" % (tag, cell))
                for directory in sorted(glob.glob(pattern)):
                    values += makespans(directory, ORLIB, instance)
            data[(cell, instance)] = values
    return data


def instances_of(tag):
    path = os.path.join(HERE, "jobs_%s.tsv" % tag)
    return sorted({line.split("\t")[2].strip()
                   for line in open(path)
                   if line.strip() and not line.startswith("#")})


def best_of_blocks(values, k=BLOCK):
    blocks = [values[i:i + k] for i in range(0, len(values) - k + 1, k)]
    return statistics.mean(min(b) for b in blocks) if blocks else float("nan")


def report(data, instances, label):
    print("=== 0. MECHANISM CHECK: does the portfolio spread wider? ===")
    print("%-6s %12s %12s %10s" % ("inst", "sd control", "sd portfolio", "ratio"))
    ratios = []
    for instance in instances:
        c = data[("control", instance)]
        p = data[("portfolio", instance)]
        if len(c) > 1 and len(p) > 1:
            sc, sp = statistics.stdev(c), statistics.stdev(p)
            ratios.append(sp / sc if sc else float("nan"))
            print("%-6s %12.2f %12.2f %10.2f" % (instance, sc, sp, ratios[-1]))
    if ratios:
        wider = sum(1 for r in ratios if r > 1.0)
        print("portfolio wider on %d of %d instances, mean ratio %.2f"
              % (wider, len(ratios), statistics.mean(ratios)))
        if statistics.mean(ratios) <= 1.0:
            print("WARNING: the mixture is not widening anything. Read the")
            print("endpoint below knowing the hypothesis was not exercised.")

    print("\n=== I-011 %s: best-of-%d (primary), mean (reported) ===" % (label, BLOCK))
    print("%-6s %11s %11s %9s   %11s %11s %9s"
          % ("inst", "bo5 ctrl", "bo5 port", "d", "mean ctrl", "mean port", "d"))
    tail_d, mean_d = [], []
    for instance in instances:
        c, p = data[("control", instance)], data[("portfolio", instance)]
        tc, tp = best_of_blocks(c), best_of_blocks(p)
        mc, mp = statistics.mean(c), statistics.mean(p)
        tail_d.append(tp - tc)
        mean_d.append(mp - mc)
        print("%-6s %11.1f %11.1f %+9.2f   %11.1f %11.1f %+9.2f"
              % (instance, tc, tp, tp - tc, mc, mp, mp - mc))
    print("\nruns per cell and instance: %d" % len(data[("control", instances[0])]))
    return tail_d, mean_d


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which == "filter":
        instances = instances_of("filter")
        data = cells_of(["filter"], instances)
        tail_d, mean_d = report(data, instances, "filter")
        mean = statistics.mean(tail_d)
        print("\nfilter rule, on the TAIL endpoint: mean(best-of-%d difference) = %+.2f;"
              % (BLOCK, mean))
        print("discard if > %+.1f -> %s"
              % (FILTER_RULE, "DISCARD" if mean > FILTER_RULE else "PASS, go to the waves"))
        print("the mean moved %+.2f, and this iteration is allowed to worsen it."
              % statistics.mean(mean_d))
    elif which == "wave":
        try:
            last = int(sys.argv[2])
        except (IndexError, ValueError):
            raise SystemExit(__doc__)
        tags = ["w%d" % w for w in range(1, last + 1)]
        instances = instances_of("w1")
        data = cells_of(tags, instances)
        tail_d, mean_d = report(data, instances, "waves 1..%d" % last)
        W, p = wilcoxon(tail_d)
        better = sum(1 for x in tail_d if x < 0)
        lower = better * 2 > len(tail_d)
        print("\nlook %d of %d, symmetric boundary p <= %.4f (Pocock, six looks)"
              % (last, WAVES, POCOCK))
        print("PRIMARY, best-of-%d: mean %+.2f, portfolio better on %d of %d, "
              "W = %s, p = %.4f"
              % (BLOCK, statistics.mean(tail_d), better, len(tail_d), W, p))
        print("reported, deciding nothing: the mean moved %+.2f"
              % statistics.mean(mean_d))
        if p <= POCOCK and lower:
            print("-> ACCEPT: the portfolio crosses on the lower rank at wave %d" % last)
        elif p <= POCOCK and not lower:
            print("-> REJECT NOW: control crosses on the lower rank at wave %d." % last)
        elif last >= WAVES:
            print("-> REJECT: the sixth wave did not cross; no seventh look.")
        else:
            print("-> continue: run wave %d" % (last + 1))
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
