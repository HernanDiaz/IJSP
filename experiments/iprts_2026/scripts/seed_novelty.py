#!/usr/bin/env python3
"""Cross-bank novelty of the OR-Tools warm seeds vs the N2N8 bank.

The IPRTS warm-seed FLOOR came from a CLUSTERED bank (all seeds from the same
TS-N2/N8 attractor). This is the go/no-go GATE for the cross-solver idea: do
the OR-Tools (CP-SAT crisp-reduction) seeds sit in regions the N2N8 bank never
reached? If yes, they de-cluster the pool and the warm validation is worth
running; if they overlap the N2N8 cluster, OR-Tools adds nothing and we stop.

Per x20 instance, on the normalised Kendall-tau distance over task orders
(the same PROXY as seed_diversity.py -- it overestimates the true disjunctive
distance the path relinking uses, so treat absolute values as upper bounds;
the COMPARISON between banks is what matters):
  withinN2N8 : mean pairwise distance inside the N2N8 bank (cluster tightness)
  withinORT  : mean pairwise distance among the OR-Tools seeds
  novelty    : mean over OR-Tools seeds of the distance to the NEAREST N2N8
               seed   <-- the gate metric
  cross      : mean distance between every OR-Tools seed and every N2N8 seed

Reading: novelty >> withinN2N8  => OR-Tools reaches genuinely new regions (GO).
         novelty ~  withinN2N8  => seeds overlap the existing cluster (STOP).

Usage: python3 seed_novelty.py [--n2n8 DIR] [--ortools DIR] [prefixes...]
"""
import argparse
import glob
import os
import statistics

HERE = os.path.dirname(os.path.abspath(__file__))


def parse_orders(path):
    orders = []
    with open(path) as fh:
        for line in fh:
            parts = line.rstrip("\n").split(";")
            if len(parts) < 2:
                continue
            tasks = parts[1].split()
            if not tasks or not tasks[0].lstrip("-").isdigit():
                continue                      # header / malformed line
            orders.append(tuple(int(t) for t in tasks))
    return orders


def kendall_norm(a, b):
    """Normalised Kendall-tau distance between two permutations of one set
    (0 = identical, 1 = reversed); inversion count via merge sort."""
    pos = {v: i for i, v in enumerate(b)}
    seq = [pos[v] for v in a]
    n = len(seq)
    if n < 2:
        return 0.0
    inv = [0]

    def sort(lo, hi):
        if hi - lo < 2:
            return seq[lo:hi]
        mid = (lo + hi) // 2
        left, right = sort(lo, mid), sort(mid, hi)
        merged, i, j = [], 0, 0
        while i < len(left) and j < len(right):
            if left[i] <= right[j]:
                merged.append(left[i]); i += 1
            else:
                merged.append(right[j]); j += 1
                inv[0] += len(left) - i
        merged.extend(left[i:]); merged.extend(right[j:])
        return merged

    sort(0, n)
    return inv[0] / (n * (n - 1) / 2)


def mean_pairwise(orders):
    d = [kendall_norm(orders[i], orders[j])
         for i in range(len(orders)) for j in range(i + 1, len(orders))]
    return statistics.mean(d) if d else 0.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n2n8",
                    default=os.path.join(HERE, "..", "seeds", "N2N8_tuned"))
    ap.add_argument("--ortools",
                    default=os.path.join(HERE, "..", "seeds", "ortools_x20"))
    ap.add_argument("prefixes", nargs="*", default=["tai30_20", "tai50_20"])
    args = ap.parse_args()

    stems = []
    for pref in args.prefixes:
        for f in sorted(glob.glob(os.path.join(args.ortools, pref + "*_Sols.csv"))):
            stems.append(os.path.basename(f)[:-len("_Sols.csv")])
    if not stems:
        raise SystemExit("no OR-Tools seed files under %s" % args.ortools)

    print("%-26s %4s %4s %10s %9s %8s %8s" %
          ("instance", "nB", "nO", "withinN2N8", "withinORT", "novelty", "cross"))
    agg = {"withinN2N8": [], "withinORT": [], "novelty": [], "cross": []}
    for stem in stems:
        bpath = os.path.join(args.n2n8, stem + "_Sols.csv")
        opath = os.path.join(args.ortools, stem + "_Sols.csv")
        if not os.path.isfile(bpath):
            print("%-26s  (no N2N8 file)" % stem)
            continue
        B, O = parse_orders(bpath), parse_orders(opath)
        if not B or not O:
            print("%-26s  (empty bank)" % stem)
            continue
        wB = mean_pairwise(B)
        wO = mean_pairwise(O)
        nov = statistics.mean(min(kendall_norm(o, b) for b in B) for o in O)
        cross = statistics.mean(kendall_norm(o, b) for o in O for b in B)
        for k, v in (("withinN2N8", wB), ("withinORT", wO),
                     ("novelty", nov), ("cross", cross)):
            agg[k].append(v)
        print("%-26s %4d %4d %10.3f %9.3f %8.3f %8.3f" %
              (stem, len(B), len(O), wB, wO, nov, cross))

    if agg["novelty"]:
        print("-" * 74)
        m = {k: statistics.mean(v) for k, v in agg.items()}
        print("%-26s %4s %4s %10.3f %9.3f %8.3f %8.3f" %
              ("MEAN", "", "", m["withinN2N8"], m["withinORT"],
               m["novelty"], m["cross"]))
        ratio = (m["novelty"] / m["withinN2N8"]) if m["withinN2N8"] else float("inf")
        verdict = ("GO (OR-Tools reaches new regions)" if ratio >= 1.3
                   else "WEAK (seeds overlap the N2N8 cluster)")
        print("\nnovelty / withinN2N8 = %.2f  ->  %s" % (ratio, verdict))
        print("(threshold 1.3 is indicative, not a hard rule)")


if __name__ == "__main__":
    main()
