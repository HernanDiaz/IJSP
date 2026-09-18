#!/usr/bin/env python3
"""Generation rate and best makespan of a results directory.

The speed of this hybrid is measured in generations, not in wall clock: every
setup here stops on time, so a run always costs its full budget and the only
thing a faster solver buys is more generations inside it. The JOURNAL's claim
that the total budget is the scarce resource is the reason this is the number
that matters.

The counts are read from the solver's own statistics CSV -- the "Number of
Generations" and "Total runtime" rows, which carry one column per run -- rather
than timed from outside, so parallel runs on a loaded machine do not distort
them.

Usage:
    generations.py <results-dir> [<results-dir> ...]
"""
import csv
import glob
import os
import statistics
import sys


def _rows(path):
    with open(path, newline='') as handle:
        for row in csv.reader(handle, delimiter=';'):
            if row:
                yield row


def per_run(path, label):
    """The per-run columns of one statistics row.

    Layout is  Field;Average;Best;Worst;Std.Dev.;Run 1;Run 2;...
    so the runs start at column 5.
    """
    for row in _rows(path):
        if row[0].strip() == label:
            return [float(cell) for cell in row[5:] if cell.strip()]
    return []


def best_makespan(resdir, instance):
    """Best completion time over the runs in an instance's certificate."""
    certificates = glob.glob(os.path.join(resdir, f'{instance}_*_Certificate.csv'))
    if not certificates:
        return None
    runs = {}
    for certificate in certificates:
        with open(certificate) as handle:
            for line in handle.read().splitlines()[1:]:
                if not line:
                    continue
                fields = line.split(';')
                runs.setdefault((certificate, fields[0]), []).append(int(fields[7]))
    if not runs:
        return None
    return min(max(completions) for completions in runs.values())


def report(resdir):
    print(f'== {resdir}')
    print(f"{'inst':6} {'runs':>4} {'gens/run':>9} {'sec/run':>8} {'gen/s':>7} {'best':>6}")
    rates, generations = [], []
    for path in sorted(glob.glob(os.path.join(resdir, '*[0-9].csv'))):
        instance = os.path.basename(path).split('_')[0]
        gens = per_run(path, 'Number of Generations')
        secs = per_run(path, 'Total runtime')
        if not gens or not secs:
            continue
        rate = [g / s for g, s in zip(gens, secs)]
        rates += rate
        generations += gens
        best = best_makespan(resdir, instance)
        print(f'{instance:6} {len(gens):4d} {statistics.mean(gens):9.1f} '
              f'{statistics.mean(secs):8.2f} {statistics.mean(rate):7.3f} '
              f'{best if best is not None else "":>6}')
    if not rates:
        print('  no runs recorded yet')
        return None
    mean_rate = statistics.mean(rates)
    print(f'\nmean gen/s over all runs: {mean_rate:.3f}   (n={len(rates)} runs)')
    print(f'mean generations per run: {statistics.mean(generations):.1f}\n')
    return mean_rate


def main(argv):
    if len(argv) < 2:
        sys.exit(__doc__)
    rates = [report(d) for d in argv[1:]]
    if len(rates) == 2 and all(rates):
        print(f'speed-up of the second over the first: {rates[1] / rates[0]:.3f}x')


if __name__ == '__main__':
    main(sys.argv)
