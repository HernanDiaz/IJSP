#!/usr/bin/env python3
"""How diverse is the population at the start, how diverse CAN it be, and
where does the diversity go during a run?

The solver's own diversity statistic is the average pairwise normalised
positional Hamming distance of the operation sequences (StatisticsHamming):
the share of positions at which two individuals hold a different job.

Upper bound. At one position, if a population of N individuals holds job j
with frequency p_j, the share of distinct pairs that differ there is
(1 - sum p_j^2) * N / (N - 1), maximised when the n jobs are equally frequent:
(1 - 1/n) * N / (N - 1). No population of N sequences over n jobs can average
more than that, however it is chosen.

Reads the 120 control traces of I-013's filter. Nothing is run.
"""
import glob
import os
import statistics

E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
N = 247                                    # population size of the frozen config
JOBS = {"ta23": 20, "ta29": 20, "ta30": 20, "ta45": 30}
BUDGET = {"ta23": 40, "ta29": 40, "ta30": 40, "ta45": 150}


def traces(instance):
    out = []
    for d in sorted(glob.glob(os.path.join(E, "results", "I-013_filter_p*_control"))):
        for path in sorted(glob.glob(os.path.join(d, instance + "_*.csv"))):
            if path.endswith(("_Certificate.csv", "_Sols.csv")):
                continue
            rows, seen = [], False
            for line in open(path):
                if line.startswith("Step;Runtime;"):
                    seen = True
                    continue
                if not seen:
                    continue
                f = line.strip().split(";")
                if len(f) < 7:
                    continue
                try:
                    rows.append((float(f[1]), float(f[6])))
                except ValueError:
                    pass
            if rows:
                out.append(rows)
    return out


def at(rows, t):
    v = rows[0][1]
    for runtime, h in rows:
        if runtime > t:
            break
        v = h
    return v


print("%-6s %6s %9s %8s | %s" % ("inst", "jobs", "max", "gen 0",
                                  "median Hamming at share of the budget"))
print("%-6s %6s %9s %8s | %7s %7s %7s %7s %7s"
      % ("", "", "possible", "", "5%", "10%", "25%", "50%", "100%"))
for inst, n in JOBS.items():
    runs = traces(inst)
    bound = (1.0 - 1.0 / n) * N / (N - 1)
    g0 = statistics.median(r[0][1] for r in runs)
    cols = [statistics.median(at(r, BUDGET[inst] * f) for r in runs)
            for f in (0.05, 0.10, 0.25, 0.50, 1.0)]
    print("%-6s %6d %9.4f %8.4f | %s"
          % (inst, n, bound, g0, " ".join("%7.4f" % c for c in cols)))
    print("%-6s %6s %9s %7.1f%% of the possible maximum at generation 0"
          % ("", "", "", 100.0 * g0 / bound))
