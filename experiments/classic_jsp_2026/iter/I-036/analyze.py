#!/usr/bin/env python3
"""I-036: the pre-declared analysis.

Usage: analyze.py filter
       analyze.py wave <N>          analyse waves 1..N together

Every makespan is recomputed from its schedule, never read from the solver.

This is a POSITION iteration, so the endpoint is the per-instance mean.

0. MECHANISM CHECK, printed first and read first, in two halves, because the
   lesson of I-009 and I-011 is that an idea that was not exercised was not
   tested:
     a) the PR crossings per run, and the positional distance between the
        parents and from the first child to its first parent: every
        crossing in the pr cell, none in the control;
     b) the generations per run.
1. PRIMARY: the per-instance mean, paired by instance, Wilcoxon, symmetric
   Pocock boundary p <= 0.0142 at each of six looks.
2. Reported and deciding nothing: best-of-five and the outright best.

Filter rule: discard if the mean of (pr - control) over the four filter
instances exceeds +2.0.
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
CELLS = ["control", "pr"]
POCOCK = 0.0142
FILTER_RULE = 2.0
WAVES = 6
BLOCK = 5

STATS = ["log:PR crossings",
         "log:mean parent distance",
         "log:mean child distance",
         "Number of Generations"]


def directories(tags, cell):
    out = []
    for tag in tags:
        out += sorted(glob.glob(os.path.join(
            EXPERIMENT, "results", "I-036_%s_p*_%s" % (tag, cell))))
    return out


def cells_of(tags, instances):
    data = {}
    for cell in CELLS:
        for instance in instances:
            values = []
            for directory in directories(tags, cell):
                values += makespans(directory, ORLIB, instance)
            data[(cell, instance)] = values
    return data


def solver_stat(tags, cell, instance, key):
    """Read one reported statistic, averaged over the runs of a cell."""
    values = []
    if key.startswith("log:"):
        # The PR walk counters, printed to stderr at exit, in the job log
        field = key[4:]
        for directory in directories(tags, cell):
            path = os.path.join(directory, instance + ".log")
            if not os.path.exists(path):
                continue
            for line in open(path):
                if line.startswith("PR crossings "):
                    part = [p.strip() for p in line.split(",")]
                    table = {"PR crossings": part[0].split()[-1],
                             "mean parent distance": part[1].split()[-1],
                             "mean child distance": part[2].split()[-1]}
                    values.append(float(table[field]))
        return statistics.mean(values) if values else 0.0
    for directory in directories(tags, cell):
        for path in sorted(glob.glob(os.path.join(
                directory, instance + "_*.csv"))):
            if path.endswith("_Certificate.csv") or path.endswith("_Sols.csv"):
                continue
            for line in open(path):
                if line.startswith(key + ";"):
                    try:
                        values.append(float(line.split(";")[1]))
                    except (IndexError, ValueError):
                        pass
    return statistics.mean(values) if values else float("nan")


def instances_of(tag):
    path = os.path.join(HERE, "jobs_%s.tsv" % tag)
    return sorted({line.split("\t")[2].strip()
                   for line in open(path)
                   if line.strip() and not line.startswith("#")})


def best_of_blocks(values, k=BLOCK):
    blocks = [values[i:i + k] for i in range(0, len(values) - k + 1, k)]
    return statistics.mean(min(b) for b in blocks) if blocks else float("nan")


def report(data, tags, instances, label):
    print("=== 0. MECHANISM CHECK ===")
    print("%-6s %-42s %10s %10s" % ("inst", "statistic", "control", "pr"))
    shares = []
    for instance in instances:
        for key in STATS:
            c = solver_stat(tags, "control", instance, key)
            f = solver_stat(tags, "pr", instance, key)
            if key == STATS[0]:
                shares.append(f)
            print("%-6s %-42s %10.2f %10.2f" % (instance, key, c, f))
    if shares:
        mean_share = statistics.mean(s for s in shares if s == s)
        print("\nPR crossings per run: %.0f on average (the control makes none)" % mean_share)

    print("\n=== I-036 %s: per-instance mean (primary), best-of-%d (reported) ==="
          % (label, BLOCK))
    print("%-6s %11s %11s %9s   %11s %11s %9s"
          % ("inst", "mean ctrl", "mean pr", "d", "bo5 ctrl", "bo5 pr", "d"))
    mean_d, tail_d = [], []
    for instance in instances:
        c, f = data[("control", instance)], data[("pr", instance)]
        mc, mf = statistics.mean(c), statistics.mean(f)
        tc, tf = best_of_blocks(c), best_of_blocks(f)
        mean_d.append(mf - mc)
        tail_d.append(tf - tc)
        print("%-6s %11.1f %11.1f %+9.2f   %11.1f %11.1f %+9.2f"
              % (instance, mc, mf, mf - mc, tc, tf, tf - tc))
    print("\nruns per cell and instance: %d" % len(data[("control", instances[0])]))
    return mean_d, tail_d


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which == "filter":
        instances = instances_of("filter")
        mean_d, tail_d = report(cells_of(["filter"], instances),
                                ["filter"], instances, "filter")
        mean = statistics.mean(mean_d)
        print("\nfilter rule, on the MEAN: mean(pr - control) = %+.2f;" % mean)
        print("discard if > %+.1f -> %s"
              % (FILTER_RULE, "DISCARD" if mean > FILTER_RULE else "PASS, go to the waves"))
        print("best-of-%d moved %+.2f, and decides nothing here."
              % (BLOCK, statistics.mean(tail_d)))
    elif which == "wave":
        try:
            last = int(sys.argv[2])
        except (IndexError, ValueError):
            raise SystemExit(__doc__)
        tags = ["w%d" % w for w in range(1, last + 1)]
        instances = instances_of("w1")
        mean_d, tail_d = report(cells_of(tags, instances), tags, instances,
                                "waves 1..%d" % last)
        W, p = wilcoxon(mean_d)
        better = sum(1 for x in mean_d if x < 0)
        lower = better * 2 > len(mean_d)
        print("\nlook %d of %d, symmetric boundary p <= %.4f (Pocock, six looks)"
              % (last, WAVES, POCOCK))
        print("PRIMARY, per-instance mean: %+.2f, pr better on %d of %d, "
              "W = %s, p = %.4f"
              % (statistics.mean(mean_d), better, len(mean_d), W, p))
        print("reported, deciding nothing: best-of-%d moved %+.2f"
              % (BLOCK, statistics.mean(tail_d)))
        if p <= POCOCK and lower:
            print("-> ACCEPT: path relinking crosses on the lower rank at wave %d" % last)
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
