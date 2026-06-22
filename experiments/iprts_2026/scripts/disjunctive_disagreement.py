#!/usr/bin/env python3
"""Cross-solver disjunctive disagreement of a warm-seed bank.

Does an exact fix-and-optimize have fuel? Measures how many disjunctive arcs
(same-machine operation orderings) the bank's solutions DISAGREE on. Consensus
arcs would be FIXED; contested arcs are the exact-recombination neighbourhood.

Each stored task order is decoded to machine sequences with a crisp-midpoint
ACTIVE (gap-insertion) schedule generation -- a structural proxy for the
framework's interval insertion SGS (enough for a go/no-go on neighbourhood
size; the real LNS would use the C++ decode).

Per instance it reports, for the base bank (N2N8) and the augmented bank
(N2N8 + OR-Tools):
  arcs%    : contested disjunctive arcs / total same-machine pairs
  freeOps% : operations touched by >=1 contested arc / total ops  (~ the set
             the exact solver would re-sequence; small+targeted = good, ~100%
             diffuse = nothing to fix)
and the delta arcs% (what the extra solver class makes contested).

Usage: python3 disjunctive_disagreement.py [--base DIR] [--plus DIR]
                                            [--instdir DIR] [prefixes...]
"""
import argparse
import glob
import os
import re
from collections import defaultdict

from ortools_seeds import parse_instance, crisp_durations  # same scripts/ dir

HERE = os.path.dirname(os.path.abspath(__file__))


def parse_orders(path):
    """Return [(order, objective_midpoint)]; mid=inf when absent."""
    out = []
    with open(path) as fh:
        for line in fh:
            p = line.rstrip("\n").split(";")
            if len(p) < 2:
                continue
            toks = p[1].split()
            if not toks or not toks[0].lstrip("-").isdigit():
                continue                       # header / malformed
            mid = float("inf")
            m = re.search(r"\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)", p[-1])
            if m:
                mid = (int(m.group(1)) + int(m.group(2))) / 2.0
            out.append(([int(t) for t in toks], mid))
    return out


def decode_machine_seqs(order, nm, machines, dur):
    """Active (gap-insertion) decode of a task order -> machine sequences."""
    iv = {m: [] for m in range(nm)}            # sorted [(start, end, op)]
    end_of = {}
    for op in order:
        j, k = divmod(op, nm)
        m, d = machines[j][k], dur[j][k]
        release = end_of[op - 1] if k > 0 else 0   # job precedence (id contig.)
        prev_end, s = 0, None
        for (st, en, _o) in iv[m]:
            gs = max(release, prev_end)
            if gs + d <= st:                   # fits in the gap before this op
                s = gs
                break
            prev_end = max(prev_end, en)
        if s is None:
            s = max(release, prev_end)
        e = s + d
        iv[m].append((s, e, op))
        iv[m].sort()
        end_of[op] = e
    return {m: [o for (_s, _e, o) in iv[m]] for m in range(nm)}


def contested(orders, nm, machines, dur):
    """Return (contested_arcs, total_arcs, free_ops, total_ops)."""
    orient = defaultdict(set)
    for order in orders:
        seq = decode_machine_seqs(order, nm, machines, dur)
        for m in range(nm):
            s = seq[m]
            for i in range(len(s)):
                a = s[i]
                for jx in range(i + 1, len(s)):
                    b = s[jx]                  # a precedes b on machine m
                    lo, hi = (a, b) if a < b else (b, a)
                    orient[(m, lo, hi)].add(a < b)   # smaller id first?
    total_arcs = len(orient)
    free_ops = set()
    cont = 0
    for (m, lo, hi), signs in orient.items():
        if len(signs) > 1:
            cont += 1
            free_ops.add(lo)
            free_ops.add(hi)
    return cont, total_arcs, len(free_ops), len(machines) * nm


def measure(bankdir, stem, instdir, topk=0):
    inst = os.path.join(instdir, stem + ".txt")
    nj, nm, mac, lo, up = parse_instance(inst)
    dur = crisp_durations(lo, up, "mid")
    sols = parse_orders(os.path.join(bankdir, stem + "_Sols.csv"))
    if not sols:
        return None
    if topk and topk < len(sols):                 # recombine only the best
        sols = sorted(sols, key=lambda s: s[1])[:topk]
    orders = [s[0] for s in sols]
    c, ta, fo, to = contested(orders, nm, mac, dur)
    return dict(n=len(orders), arcs=100.0 * c / ta, free=100.0 * fo / to)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", default=os.path.join(HERE, "..", "seeds", "N2N8_tuned"))
    ap.add_argument("--plus", default=os.path.join(HERE, "..", "seeds", "N2N8_ortools_x20"))
    ap.add_argument("--instdir", default="SelectosYTaillardIntervalos")
    ap.add_argument("--topk", type=int, default=0,
                    help="restrict each bank to its best K solutions (0=all)")
    ap.add_argument("prefixes", nargs="*", default=["tai30_20", "tai50_20"])
    args = ap.parse_args()

    stems = []
    for pref in args.prefixes:
        for f in sorted(glob.glob(os.path.join(args.plus, pref + "*_Sols.csv"))):
            stems.append(os.path.basename(f)[:-len("_Sols.csv")])

    print("%-24s %4s %4s | %8s %9s | %8s %9s | %7s" % (
        "instance", "nB", "nP", "base arcs%", "freeOps%",
        "plus arcs%", "freeOps%", "d arcs%"))
    agg = defaultdict(list)
    for stem in stems:
        b = measure(args.base, stem, args.instdir, args.topk)
        p = measure(args.plus, stem, args.instdir, args.topk)
        if not b or not p:
            print("%-24s  (missing)" % stem)
            continue
        d = p["arcs"] - b["arcs"]
        agg["ba"].append(b["arcs"]); agg["bf"].append(b["free"])
        agg["pa"].append(p["arcs"]); agg["pf"].append(p["free"]); agg["d"].append(d)
        print("%-24s %4d %4d | %8.2f %9.1f | %8.2f %9.1f | %7.2f" % (
            stem.replace(".F.15_01", ""), b["n"], p["n"],
            b["arcs"], b["free"], p["arcs"], p["free"], d))
    if agg["ba"]:
        n = len(agg["ba"])
        mean = lambda k: sum(agg[k]) / n
        print("-" * 86)
        print("%-24s %4s %4s | %8.2f %9.1f | %8.2f %9.1f | %7.2f" % (
            "MEAN", "", "", mean("ba"), mean("bf"),
            mean("pa"), mean("pf"), mean("d")))
        print("\nReading: freeOps% ~100 = contention diffuse, nothing to fix "
              "(weak). Low arcs% with a targeted freeOps% = a real exact-"
              "recombination neighbourhood. d arcs% = what OR-Tools adds.")


if __name__ == "__main__":
    main()
