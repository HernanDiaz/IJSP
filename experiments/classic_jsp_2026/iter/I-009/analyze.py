#!/usr/bin/env python3
"""I-009: the pre-declared analysis of the filter or of the waves so far.

Usage: analyze.py filter
       analyze.py wave <N>          analyse waves 1..N together

Every makespan is recomputed from the schedule (verify_certificate.py), never
read from the solver.

Filter, on the four instances fixed in RESEARCH_IDEAS.md: the rule can only
discard. Discard if the mean of (deep - control) over the four exceeds +2.0
makespan units. Nothing else the filter says decides anything, and its
magnitude is not a forecast of the final one -- I-001 read -2.75 on the filter
and -0.90 on the 21, and I-002 read -1.44 and then +1.52.

Confirmation: kick against control on the per-instance mean of the runs
accumulated so far, Wilcoxon signed-rank paired over the 21 open instances,
two-sided, at Pocock's constant boundary for six equally spaced looks,
p <= 0.0142 at every look including the last.

The boundary is SYMMETRIC, which is the gap I-002 exposed: crossing it with
`kick` on the lower rank accepts the idea, and crossing it with `control` on
the lower rank rejects it there and then. Without that, a batch heading the
wrong way runs all six waves to say what the second already said. Not
crossing by the sixth wave is also a rejection: there is no seventh look.
"""
import os
import statistics
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
EXPERIMENT = os.path.abspath(os.path.join(HERE, "..", ".."))
sys.path.insert(0, os.path.join(EXPERIMENT, "scripts"))
from compare import makespans, wilcoxon                      # noqa: E402

ORLIB = os.path.join(EXPERIMENT, "reference", "taillard_orlib.txt")
CELLS = ["control", "deep"]
POCOCK = 0.0142
FILTER_RULE = 2.0
WAVES = 6
FILTER_CHUNKS = 6


def cells_of(tags, instances):
    data = {}
    for cell in CELLS:
        for instance in instances:
            values = []
            for tag in tags:
                directory = os.path.join(EXPERIMENT, "results",
                                         "I-009_%s_%s" % (tag, cell))
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
    print("=== I-009 %s: per-instance mean (best in brackets) ===" % label)
    print("%-6s" % "inst" + "".join("%18s" % c for c in CELLS) + "%13s" % "deep - ctrl")
    differences = []
    for instance in instances:
        row = ""
        for cell in CELLS:
            v = data[(cell, instance)]
            row += "%11.1f [%4d]" % (statistics.mean(v), min(v)) if v else "%18s" % "-"
        d = (statistics.mean(data[("deep", instance)])
             - statistics.mean(data[("control", instance)]))
        differences.append(d)
        print("%-6s" % instance + row + "%+13.2f" % d)
    print("\nruns per cell and instance: %d" % len(data[("control", instances[0])]))
    return differences


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else ""
    if which == "filter":
        tags = ["fc%d" % c for c in range(1, FILTER_CHUNKS + 1)]
        instances = instances_of("fc1")
        data = cells_of(tags, instances)
        d = report(data, instances, "filter")
        mean = statistics.mean(d)
        print("\nfilter rule: mean(deep - control) = %+.2f; discard if > %+.1f -> %s"
              % (mean, FILTER_RULE,
                 "DISCARD" if mean > FILTER_RULE else "PASS, go to the waves"))
        print("the filter only discards, and its magnitude is not a forecast.")
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
        ft_lower = better * 2 > len(d)
        print("\nlook %d of %d, symmetric boundary p <= %.4f (Pocock, six looks)"
              % (last, WAVES, POCOCK))
        print("deep vs control: mean %+.2f, deep better on %d of %d, W = %s, p = %.4f"
              % (statistics.mean(d), better, len(d), W, p))
        if p <= POCOCK and ft_lower:
            print("-> ACCEPT: deep crosses the boundary on the lower rank at wave %d" % last)
        elif p <= POCOCK and not ft_lower:
            print("-> REJECT NOW: control crosses the boundary on the lower rank at "
                  "wave %d; the remaining waves are not run." % last)
        elif last >= WAVES:
            print("-> REJECT: the sixth wave did not cross the boundary; no seventh look.")
        else:
            print("-> continue: run wave %d" % (last + 1))
    else:
        raise SystemExit(__doc__)


if __name__ == "__main__":
    main()
