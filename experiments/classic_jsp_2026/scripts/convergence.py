#!/usr/bin/env python3
"""Show how much of the time budget a run actually uses.

Every setup here stops on wall-clock time, so a run costs its full budget
whether or not the search is still finding anything. That makes the budget a
free parameter with a real price: time spent after the last improvement buys
nothing, and the same CPU spent on extra runs buys statistical power, which is
what separates configurations.

For each result directory this reports when the best value last improved, as a
fraction of the run's total time, and then -- for a range of candidate budgets --
what fraction of runs were still improving past that point. A budget is too long
when almost no run improves in its last half, and too short when a sizeable
fraction is still improving at the end.

Read from the solver's own evolution table rather than the certificates, which
record only the final schedule.
"""
import argparse
import glob
import os
import statistics


def last_improvements(path):
    """[(time of last improvement, total time)] for each run in one result CSV."""
    lines = open(path).read().splitlines()
    try:
        start = next(i for i, l in enumerate(lines)
                     if l.startswith("Step;Runtime;Best"))
    except StopIteration:
        return []

    header = lines[start].split(";")
    # Six shared columns, then one six-column block per run.
    runs = (len(header) - 6) // 6 if len(header) > 6 else 0

    rows = []
    for line in lines[start + 1:]:
        if not line or not line[0].isdigit():
            break
        rows.append(line.split(";"))
    if not rows:
        return []

    result = []
    for run in range(runs):
        base = 6 + run * 6
        best = None
        improved_at = total = 0.0
        for row in rows:
            if len(row) <= base + 2:
                continue
            try:
                moment, value = float(row[base + 1]), float(row[base + 2])
            except ValueError:
                continue
            total = max(total, moment)
            if best is None or value < best:
                best, improved_at = value, moment
        if best is not None and total > 0:
            result.append((improved_at, total))
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True, nargs="+",
                        help="one or more result directories")
    parser.add_argument("--cuts", default="25,40,50,60,75,90",
                        help="candidate budgets, as percentages")
    args = parser.parse_args()

    cuts = [int(c) for c in args.cuts.split(",")]

    for directory in args.results:
        fractions, budget = [], 0.0
        for path in sorted(glob.glob(os.path.join(directory, "*.csv"))):
            if "Certificate" in path or "Sols" in path:
                continue
            for improved_at, total in last_improvements(path):
                fractions.append(100.0 * improved_at / total)
                budget = max(budget, total)

        name = os.path.basename(os.path.normpath(directory))
        if not fractions:
            print("%s: no evolution data" % name)
            continue

        print("\n%s -- %d runs, budget about %.0f s" % (name, len(fractions),
                                                        budget))
        print("  last improvement at %.0f%% of the budget on average "
              "(median %.0f%%, max %.0f%%)"
              % (statistics.mean(fractions), statistics.median(fractions),
                 max(fractions)))
        print("  %-10s %-10s %s" % ("budget", "still", "CPU saved"))
        print("  %-10s %-10s %s" % ("", "improving", ""))
        for cut in cuts:
            still = sum(1 for f in fractions if f > cut)
            print("  %-10s %-10s %d%%"
                  % ("%d%% (%.0fs)" % (cut, budget * cut / 100.0),
                     "%.1f%%" % (100.0 * still / len(fractions)), 100 - cut))


if __name__ == "__main__":
    main()
