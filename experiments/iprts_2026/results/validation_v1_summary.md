# IPRTS v1 — quick validation on the 12 selected classical instances

Date: 2026-06-10 · setup `setup_IPRTS_val_quick.txt` (hand-picked defaults, NOT
irace-tuned) · 5 runs × 60 s CPU per instance, parallel (1 core/instance) ·
baseline: irace-tuned TS-N2 inside ABCPSO, 30 runs, 900 s budget
(`cor_tabu_2026/statistical_results/per_instance_stats.csv`).

| instance | size | IPRTS avg_E | IPRTS best_E | N2 avg_E | N2 best_E | Δavg% | Δbest% |
|---|---|---|---|---|---|---|---|
| abz7  | 20×15 | 693.70 | 686.0 | 675.88 | 668.0 | +2.64 | +2.69 |
| abz8  | 20×15 | 705.20 | 702.0 | 687.80 | 674.0 | +2.53 | +4.15 |
| abz9  | 20×15 | 730.00 | 721.0 | 702.68 | 694.5 | +3.89 | +3.82 |
| ft10  | 10×10 | 942.60 | 935.5 | 939.20 | 935.5 | +0.36 | 0.00 |
| ft20  | 20×5  | 1168.40 | 1166.0 | 1167.73 | 1166.0 | +0.06 | 0.00 |
| la21  | 15×10 | 1059.20 | 1052.0 | 1060.50 | 1052.0 | **−0.12** | 0.00 |
| la24  | 15×10 | 946.60 | 944.0 | 947.78 | 942.5 | **−0.12** | +0.16 |
| la25  | 15×10 | 986.20 | 980.0 | 989.82 | 979.0 | **−0.37** | +0.10 |
| la27  | 20×10 | 1275.00 | 1269.0 | 1255.88 | 1247.0 | +1.52 | +1.76 |
| la29  | 20×10 | 1214.40 | 1198.0 | 1185.10 | 1174.5 | +2.47 | +2.00 |
| la38  | 15×15 | 1247.90 | 1234.0 | 1218.87 | 1204.5 | +2.38 | +2.45 |
| la40  | 15×15 | 1246.50 | 1237.5 | 1235.30 | 1230.5 | +0.91 | +0.57 |
| **mean** | | | | | | **+1.35** | **+1.48** |

## Reading

- **Clear size-dependent pattern.** Small/medium (10×10, 20×5, 15×10): parity
  to slightly BETTER than the tuned baseline (la21/24/25 negative deltas, and
  best_E ties the baseline best on ft10/ft20/la21). Large (20×15, 20×10,
  15×15): +0.9 to +3.9 % worse.
- Design gate "never >1 % worse" passes on 7/12, fails on the 5 largest.
- Caveats: 5 runs vs 30 (directional, not statistical); 60 s cap bound most
  runs on the large instances (their median runtimes hit the cap), while the
  baseline had converged by its own plateau criterion within its budget.
- Consistent hypothesis: the hand-picked pool/TS parameters (pool=10,
  dmin=0.05, patience=50, bad-iterations=100, max-rounds=50) do not scale
  with instance size. The fix list, in expected-return order: multi-start
  quality+diversity seeding (Hernán's proposal), irace (Phase 1, possibly
  size-aware), LS_TabuDeep kernel (evict-oldest instead of dead-end exit),
  richer PR (back-and-forth, sample every k-th intermediate).

## Runtime / anytime analysis (added after Hernán asked for times)

Median runtime per instance: IPRTS hit the 60 s cap on 9/12 instances
(43.8–52.3 s on ft10/ft20/la24, where the 5000-cycle plateau fired); the
baseline self-converged at 6.5–40.1 s with 900 s available.

Time-to-target (per-second evolution curves; target = baseline avg_E):

| instance | t to reach N2 avg (med) | runs reaching | IPRTS E at N2's med_rt | N2 avg |
|---|---|---|---|---|
| abz7/8/9 | never (>60 s) | 0/5 | 696.5 / 705.9 / 735.1 | 675.9 / 687.8 / 702.7 |
| ft10 | ~19 s | 3/5 | 957.9 @6.5 s | 939.2 |
| ft20 | ~24 s | 3/5 | 1183.7 @9.1 s | 1167.7 |
| la21 | ~16 s | 4/5 | 1071.9 @11.1 s | 1060.5 |
| la24 | ~32 s | 3/5 | 957.3 @10.3 s | 947.8 |
| la25 | >60 s | 2/5 | 997.0 @10.2 s | 989.8 |
| la27/29/38/40 | never (>60 s) | 0/5 | all above target | — |

**Honest conclusion:** at EQUAL time, IPRTS v1 loses everywhere; the la21/24/25
wins required 5–6× the time the baseline needed to self-converge. The flip
side: the baseline saturates (its own plateau stops it; extra budget unused)
while IPRTS keeps improving up to the cap and ends ABOVE the baseline's
converged quality on the 15×10 family. v1's weak flank is convergence SPEED,
not (only) final quality — which is exactly what better seeding, deep TS
chains and irace target.
