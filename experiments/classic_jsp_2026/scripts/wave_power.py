#!/usr/bin/env python3
"""The cost and the power of the wave design of RESEARCH_IDEAS.md ("Cadencia").

A confirmation is six waves of five runs per cell over the 21 open instances,
looked at after each wave with Pocock's constant boundary for six equally
spaced looks (nominal two-sided alpha 0.0142 per look, 0.05 overall). This
simulates what that buys: the type-I rate when the idea does nothing, and the
chance of declaring by each wave when it does, with the run-to-run spread
measured on the I-001 filter batch (sd about 8 makespan units). Reproduces the
table in RESEARCH_IDEAS.md.

The model is the one the test sees: each instance has its own true effect,
drawn around the assumed mean with a spread of 2 units, and a wave measures
each instance's mean difference with a standard error of sd * sqrt(2/5).
"""
import math
import random
import statistics

SD = 8.1          # per-run sd, measured on the I-001 filter batch
NINST = 21        # open instances with a seed bank
TAU = 2.0         # instance-to-instance spread of the true effect
WAVE = 5          # runs per cell per wave
LOOKS = 6
POCOCK = 0.0142   # nominal two-sided alpha per look, 6 equally spaced looks
DRAWS = 1500


def wilcoxon_p(differences):
    """Normal-approximation two-sided p of the signed-rank statistic (n = 21).

    The same approximation compare.wilcoxon() uses above n = 20.
    """
    n = len(differences)
    order = sorted(range(n), key=lambda i: abs(differences[i]))
    positive = sum(rank for rank, i in enumerate(order, start=1)
                   if differences[i] > 0)
    total = n * (n + 1) / 2.0
    statistic = min(positive, total - positive)
    mean = n * (n + 1) / 4.0
    sd = math.sqrt(n * (n + 1) * (2 * n + 1) / 24.0)
    return math.erfc(abs((statistic - mean) / sd) / math.sqrt(2))


def trial(mu, rng):
    """One confirmation run as accumulating waves.

    Returns the wave at which it declares, or 0 if it never does.
    """
    true = [rng.gauss(mu, TAU) for _ in range(NINST)]
    accumulated = [[] for _ in range(NINST)]
    error = SD * (2.0 / WAVE) ** 0.5
    for wave in range(1, LOOKS + 1):
        for i in range(NINST):
            accumulated[i].append(rng.gauss(true[i], error))
        d = [statistics.mean(a) for a in accumulated]
        if wilcoxon_p(d) <= POCOCK and sum(1 for x in d if x < 0) * 2 > NINST:
            return wave
    return 0


def main():
    print("waves of %d runs, %d looks, nominal alpha %.4f per look, %d draws"
          % (WAVE, LOOKS, POCOCK, DRAWS))
    print("%-14s %9s   %s" % ("true effect", "declares", "cumulative % by wave 1..6"))
    for label, mu in (("0 (none)", 0.0), ("-1.5", -1.5), ("-3.0", -3.0), ("-6.0", -6.0)):
        rng = random.Random(11)
        got = [trial(mu, rng) for _ in range(DRAWS)]
        cumulative = [100.0 * sum(1 for g in got if g and g <= w) / DRAWS
                      for w in range(1, LOOKS + 1)]
        print("%-14s %8.1f%%   %s"
              % (label, cumulative[-1], " ".join("%5.1f" % c for c in cumulative)))


if __name__ == "__main__":
    main()
