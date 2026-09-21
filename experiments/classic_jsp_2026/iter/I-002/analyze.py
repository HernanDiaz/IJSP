#!/usr/bin/env python3
"""I-002: the pre-declared analysis of the filter or of the waves so far.

Usage: analyze.py filter
       analyze.py wave <N>          analyse waves 1..N together

Every makespan is recomputed from the schedule (verify_certificate.py), never
read from the solver.

Filter, on the four instances fixed in RESEARCH_IDEAS.md: the rule can only
discard. Discard if the mean of (n8 - control) over the four exceeds +2.0
makespan units. Nothing else the filter says decides anything.

Confirmation: n8 against control on the per-instance mean of the runs
accumulated so far, Wilcoxon signed-rank paired over the 21 open instances,
two-sided. Because the batch is looked at after each of six waves, the
boundary is Pocock's constant one for six equally spaced looks, p <= 0.0142 at
every look including the last, not 0.05. Declared only if the boundary is met
and n8 holds the lower rank. Secondary and not deciding: the expected best of
each cell over the accumulated runs, which is what a record attempt sees.
"""
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon                      # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["control", "n8"]
POCOCK = 0.0142
FILTER_RULE = 2.0
WAVES = 6


def cells_of(tags, instances):
    """Accumulate the makespans of every tag, per cell and instance."""
    data = {}
    for cell in CELLS:
        for instance in instances:
            values = []
            for tag in tags:
                directory = os.path.join(EXPERIMENT, "results",
                                         "I-002_%s_%s" % (tag, cell))
                if os.path.isdir(directory):
                    values += makespans(directory, ORLIB, instance)
            data[(cell, instance)] = values
    return data


def instances_of(tag):
    path = os.path.join(HERE, "jobs_%s.tsv" % tag)
    return sorted({line.split("\t")[2].strip()
                   for line in open(path)
                   if line.strip() and not line.startswith("#")})


def report(data, instances, label):
    print("=== I-002 %s: per-instance mean (best in brackets) ===" % label)
    print("%-6s" % "inst" + "".join("%18s" % c for c in CELLS) + "%12s" % "n8 - ctrl")
    differences = []
    for instance in instances:
        row = ""
        for cell in CELLS:
            v = data[(cell, instance)]
            row += "%11.1f [%4d]" % (statistics.mean(v), min(v)) if v else "%18s" % "-"
        d = (statistics.mean(data[("n8", instance)])
             - statistics.mean(data[("control", instance)]))
        differences.append(d)
        print("%-6s" % instance + row + "%+12.2f" % d)
    runs = len(data[("control", instances[0])])
    print("\nruns per cell and instance: %d" % runs)
    return differences


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which == "filter":
        instances = instances_of("filter")
        data = cells_of(["filter"], instances)
        d = report(data, instances, "filter")
        mean = statistics.mean(d)
        print("\nfilter rule: mean(n8 - control) = %+.2f; discard if > %+.1f -> %s"
              % (mean, FILTER_RULE,
                 "DISCARD" if mean > FILTER_RULE else "PASS, go to the waves"))
        print("the filter only discards: it cannot accept anything.")
    elif which == "wave":
        try:
            last = int(sys.argv[2])
        except (IndexError, ValueError):
            raise SystemExit(__doc__)
        tags = ["w%d" % w for w in range(1, last + 1)]
        instances = instances_of("w1")
        data = cells_of(tags, instances)
        d = report(data, instances, "waves 1..%d" % last)
        W, p = wilcoxon(d)
        better = sum(1 for x in d if x < 0)
        declared = p <= POCOCK and better * 2 > len(d)
        print("\nlook %d of %d, boundary p <= %.4f (Pocock, six looks)" % (last, WAVES, POCOCK))
        print("n8 vs control: mean %+.2f, better on %d of %d, W = %s, p = %.4f"
              % (statistics.mean(d), better, len(d), W, p))
        if declared:
            print("-> DECLARE: n8 accepted at wave %d" % last)
        elif last >= WAVES:
            print("-> REJECT: the sixth wave did not reach the boundary; no seventh look.")
        else:
            print("-> continue: run wave %d" % (last + 1))
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
