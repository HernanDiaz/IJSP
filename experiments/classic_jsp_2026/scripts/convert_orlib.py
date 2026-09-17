#!/usr/bin/env python3
"""Convert OR-Library / Taillard JSP instances into the FuzzyFW IJSP file format.

A classic (crisp) JSP instance is exactly an IJSP instance in which every
processing time is a degenerate interval [p, p].  With a == b every ranking
method in FuzzyFW::Interval (Component, EV, Sakawa, Jiang, Lex1, Lex2, YX)
collapses to the natural order on the reals and the component-wise maximum is
the ordinary maximum, so the interval machinery computes the classic JSP
makespan exactly.

Source: https://github.com/thomasWeise/jsspInstancesAndResults
        data-raw/instance-data/instance_data.txt   (OR-Library format)
        data-raw/instances/instances_with_bks.txt  (LB / best-known solution)
"""
import argparse
import csv
import os
import re
import sys

INSTANCE_HEADER = re.compile(r"^\s*instance\s+(\S+)\s*$")
BOUND_LINE = re.compile(r"lower bound:\s*(\d+);\s*best known solution:\s*(\d+)")


def parse_instance_data(path):
    """Yield (name, n_jobs, n_machines, rows, lb, bks) for every instance."""
    with open(path) as handle:
        lines = handle.read().splitlines()

    i = 0
    while i < len(lines):
        match = INSTANCE_HEADER.match(lines[i])
        if not match:
            i += 1
            continue

        name = match.group(1)
        lb = bks = None
        i += 1
        # Skip the separator / bound block until the "<jobs> <machines>" line.
        while i < len(lines):
            bound = BOUND_LINE.search(lines[i])
            if bound:
                lb, bks = int(bound.group(1)), int(bound.group(2))
            tokens = lines[i].split()
            if len(tokens) == 2 and all(t.isdigit() for t in tokens):
                break
            i += 1
        if i >= len(lines):
            break

        n_jobs, n_machines = (int(t) for t in lines[i].split())
        i += 1

        rows = []
        while len(rows) < n_jobs and i < len(lines):
            tokens = lines[i].split()
            i += 1
            if not tokens:
                continue
            values = [int(t) for t in tokens]
            if len(values) != 2 * n_machines:
                raise ValueError(
                    "%s: job row has %d values, expected %d"
                    % (name, len(values), 2 * n_machines))
            rows.append(values)

        if len(rows) != n_jobs:
            raise ValueError("%s: found %d job rows, expected %d"
                             % (name, len(rows), n_jobs))
        yield name, n_jobs, n_machines, rows, lb, bks


def write_ijsp(path, n_jobs, n_machines, rows):
    """Write one instance using degenerate intervals (p, p)."""
    with open(path, "w") as out:
        out.write("NUMERO DE TRABAJOS\n%d\n" % n_jobs)
        out.write("NUMERO DE RECURSOS\n%d\n" % n_machines)
        out.write("SECUENCIA DE MAQUINAS\n")
        for values in rows:
            out.write(" ".join(str(values[2 * k]) for k in range(n_machines)))
            out.write(" \n")
        out.write("DURACIONES\n")
        for values in rows:
            durations = (values[2 * k + 1] for k in range(n_machines))
            out.write(" ".join("(%d, %d)" % (p, p) for p in durations))
            out.write(" \n")


def load_bks(path):
    """Read the reference LB / best-known-solution table."""
    table = {}
    with open(path) as handle:
        for row in csv.DictReader(handle):
            table[row["inst.id"]] = (int(row["inst.opt.bound.lower"]),
                                     int(row["inst.bks"]))
    return table


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--instance-data", required=True)
    parser.add_argument("--bks", help="instances_with_bks.txt (authoritative)")
    parser.add_argument("--out-dir", required=True)
    parser.add_argument("--bounds-csv", required=True)
    parser.add_argument("--prefix", default="ta",
                        help="only convert instances whose name starts here")
    args = parser.parse_args()

    reference = load_bks(args.bks) if args.bks else {}
    os.makedirs(args.out_dir, exist_ok=True)

    written = 0
    with open(args.bounds_csv, "w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["instance", "jobs", "machines", "lb", "bks", "status"])
        for name, n_jobs, n_machines, rows, lb, bks in parse_instance_data(
                args.instance_data):
            if not name.startswith(args.prefix):
                continue
            if name in reference:
                ref_lb, ref_bks = reference[name]
                if (lb, bks) != (ref_lb, ref_bks):
                    print("note: %s bounds %s/%s -> reference %s/%s"
                          % (name, lb, bks, ref_lb, ref_bks), file=sys.stderr)
                lb, bks = ref_lb, ref_bks
            if lb is None or bks is None:
                raise ValueError("%s: missing bounds" % name)

            write_ijsp(os.path.join(args.out_dir, name + ".txt"),
                       n_jobs, n_machines, rows)
            writer.writerow([name, n_jobs, n_machines, lb, bks,
                             "closed" if lb == bks else "open"])
            written += 1

    print("wrote %d instances to %s" % (written, args.out_dir))


if __name__ == "__main__":
    main()
