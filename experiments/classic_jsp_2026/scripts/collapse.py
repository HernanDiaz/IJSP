#!/usr/bin/env python3
"""The population-collapse curve of a run, from the solver's Evolution block.

Reads the "Average Evolution" columns -- Step, Runtime, Best, Avg, and, when the
setup asked for them, "Avg Hamming" (mean pairwise Hamming distance between
genotypes, normalised to [0, 1]) and "Avg Neri (W-B)" (1 - (avg - best) /
(worst - best) on the fitness: 0 when the population is spread, 1 when it sits
on the best individual). Prints the curve sampled at a fixed set of
generations, so two runs can be read side by side.

Usage:
    collapse.py <stats.csv> [<stats.csv> ...]
"""
import csv
import os
import sys

SAMPLE = (1, 2, 3, 5, 10, 15, 20, 30, 50, 75, 100, 150, 200, 300, 400, 600, 800, 1000)


def evolution(path):
    """(header, rows) of the Average Evolution block, cells as strings."""
    header, rows, inside = None, [], False
    with open(path, newline='') as handle:
        for row in csv.reader(handle, delimiter=';'):
            if not row:
                continue
            if row[0].strip() == 'Step':
                header = [c.strip() for c in row]
                inside = True
                continue
            if inside:
                try:
                    int(row[0])
                except ValueError:
                    break
                rows.append(row)
    return header, rows


def column(header, name):
    """Index of the first column whose name starts with `name`, or None."""
    for i, h in enumerate(header):
        if h.startswith(name):
            return i
    return None


def report(path):
    header, rows = evolution(path)
    if not rows:
        print(f'{path}: no evolution block')
        return
    c_gen, c_t = 0, 1
    c_best, c_avg = column(header, 'Best'), column(header, 'Avg')
    c_ham, c_neri = column(header, 'Avg Hamming'), column(header, 'Avg Neri')
    last = int(rows[-1][0])
    print(f'== {os.path.basename(path)}   ({last} generations)')
    head = f"{'gen':>5} {'sec':>7} {'best':>7} {'avg':>9}"
    if c_ham is not None:
        head += f" {'hamming':>8}"
    if c_neri is not None:
        head += f" {'neri':>6}"
    print(head)
    wanted = set(g for g in SAMPLE if g <= last) | {last}
    for row in rows:
        gen = int(row[c_gen])
        if gen not in wanted:
            continue
        line = (f'{gen:5d} {float(row[c_t]):7.1f} {float(row[c_best]):7.0f} '
                f'{float(row[c_avg]):9.2f}')
        if c_ham is not None:
            line += f' {float(row[c_ham]):8.3f}'
        if c_neri is not None:
            line += f' {float(row[c_neri]):6.3f}'
        print(line)
    print()


for arg in sys.argv[1:]:
    report(arg)
