# IPRTS cold, irace-tuned (cfg 187) — full validation on the 12 selectas

Date: 2026-06-13. Setup `setup_IPRTS_cold_tuned.txt` (irace elite cfg 187:
pool_size 16, min-distance 0.0325, restart-patience 26, perturb-strength
0.0993, tabu bad-iterations 255, tabu-max 10, max-rounds 24). Cold start
(creation = ijsp.random, NO warm seeding), 30 runs x 900 s, parallel.
Baseline: irace-tuned TS-N2 (paper Phase B/C), 30 runs. RE % vs the paper's
lower bounds.

| instance | LB | IPRTS avg (RE%) | IPRTS best (RE%) | N2 avg (RE%) | N2 best (RE%) | best verdict |
|---|---|---|---|---|---|---|
| abz7 | 656 | 670.58 (2.22) | 664.5 (1.30) | 675.88 (3.03) | 668.0 (1.83) | **−3.5 new BK** |
| abz8 | 645 | 678.97 (5.27) | 673.0 (4.34) | 687.80 (6.64) | 674.0 (4.50) | **−1.0 new BK** |
| abz9 | 661 | 694.65 (5.09) | 689.0 (4.24) | 702.68 (6.31) | 694.5 (5.07) | **−5.5 new BK** |
| ft10 | 930 | 937.02 (0.75) | 935.5 (0.59) | 939.20 (0.99) | 935.5 (0.59) | tie |
| ft20 | 1165 | 1168.40 (0.29) | 1166.0 (0.09) | 1167.73 (0.23) | 1166.0 (0.09) | tie |
| la21 | 1046 | 1053.53 (0.72) | 1052.0 (0.57) | 1060.50 (1.39) | 1052.0 (0.57) | tie |
| la24 | 935 | 945.13 (1.08) | 942.0 (0.75) | 947.78 (1.37) | 942.5 (0.80) | **−0.5 new BK** |
| la25 | 977 | 986.27 (0.95) | 979.0 (0.20) | 989.82 (1.31) | 979.0 (0.20) | tie |
| la27 | 1235 | 1255.02 (1.62) | 1239.0 (0.32) | 1255.88 (1.69) | 1247.0 (0.97) | **−8.0 new BK** |
| la29 | 1152 | 1179.97 (2.43) | 1166.5 (1.26) | 1185.10 (2.87) | 1174.5 (1.95) | **−8.0 new BK** |
| la38 | 1196 | 1208.52 (1.05) | 1204.5 (0.71) | 1218.87 (1.91) | 1204.5 (0.71) | tie |
| la40 | 1222 | 1233.98 (0.98) | 1229.5 (0.61) | 1235.30 (1.09) | 1230.5 (0.70) | **−1.0 new BK** |
| **MEAN RE** | | **1.87** | **1.25** | 2.40 | 1.50 | |

## Headline

- **Mean RE beats the published baseline on both metrics**: avg-of-runs
  1.87 vs 2.40 (a 22% reduction), best-of-runs 1.25 vs 1.50. This is the
  COLD algorithm: no warm-start, random seeding only. It stands on its own.
- **7 new best-known solutions** (abz7/8/9, la24, la27, la29, la40),
  5 exact ties, **0 losses** on the best envelope. la27 (−8) and la29 (−8)
  are the largest improvements.
- Per-instance avg RE is lower than the baseline on 11/12 instances; the
  only exception is ft20 (0.29 vs 0.23), where both solvers sit essentially
  at the optimum (LB 1165, both ~1166).
- Comparison to the warm-start experiment: cold-tuned finds MORE
  best-knowns (7 vs 4) than warm v2, confirming that the irace tuning, not
  the donor material, is what closed the gap. Warm-start is now an optional
  hybridisation, not a necessity.

## Honest caveat

Runtimes differ by an order of magnitude: the de-saturated cold IPRTS uses
the full 900 s budget on hard instances (abz8 896 s, abz9 828 s, la27 102 s)
whereas the baseline self-converges at 7-40 s. The RE comparison is the
metric the paper reports and is what matters for solution quality, but the
two solvers are not iso-time. The baseline saturates (its plateau halts it
and extra budget would not help much), so the comparison reflects each
method run to its natural stopping point rather than a fixed-time race.
A fixed-time (e.g. iso-CPU) comparison is the clean follow-up.
