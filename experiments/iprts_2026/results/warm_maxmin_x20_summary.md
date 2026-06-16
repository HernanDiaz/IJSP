# IPRTS warm + max-min diversity seeding — x20 large instances

Date: 2026-06-16. Setup `setup_IPRTS_warm_maxmin.txt` (irace cfg 187 + warm
start from the N2+N8 tuned bank `seeds/N2N8_tuned`, **max-min diversity** seed
selection, 10 % random mix, unbounded PR). 10 runs x 900 s, parallel. Results
`results/warm_maxmin_x20/`. Target: the 20 high-machine-count instances
(tai30_20, tai50_20) — the only classes where the cold irace-tuned IPRTS
(Phase 2) *loses* to the baseline. Baseline: irace-tuned TS-N2 (paper Phase B/C,
30 runs). RE % vs the paper's lower bounds. "PR" below = this warm+maxmin IPRTS.

## Per-instance (warm+maxmin vs TS-N2)

| instance | LB | PR avg (RE%) | PR best (RE%) | N2 avg (RE%) | N2 best (RE%) | Δbest |
|---|---|---|---|---|---|---|
| tai30_20_01 | 1906 | 2111.25 (10.77) | 2089.0 (9.60) | 2136.97 (12.12) | 2090.5 (9.68) | −1.5 |
| tai30_20_02 | 1884 | 2018.70 (7.15) | 2008.0 (6.58) | 2041.52 (8.36) | 2008.0 (6.58) | tie |
| tai30_20_03 | 1809 | 1928.60 (6.61) | 1911.5 (5.67) | 1966.82 (8.72) | 1911.5 (5.67) | tie |
| tai30_20_04 | 1948 | 2058.20 (5.66) | 2040.0 (4.72) | 2081.57 (6.86) | 2040.0 (4.72) | tie |
| tai30_20_05 | 1997 | 2056.95 (3.00) | 2051.0 (2.70) | 2080.32 (4.17) | 2051.0 (2.70) | tie |
| tai30_20_06 | 1957 | 2091.85 (6.89) | 2073.0 (5.93) | 2127.03 (8.69) | 2091.5 (6.87) | **−18.5** |
| tai30_20_07 | 1807 | 1988.85 (10.06) | 1986.0 (9.91) | 2015.37 (11.53) | 1986.0 (9.91) | tie |
| tai30_20_08 | 1912 | 2038.20 (6.60) | 2029.5 (6.15) | 2062.52 (7.87) | 2029.5 (6.15) | tie |
| tai30_20_09 | 1931 | 2030.90 (5.17) | 2023.5 (4.79) | 2056.40 (6.49) | 2027.5 (5.00) | −4.0 |
| tai30_20_10 | 1833 | 2014.60 (9.91) | 2002.0 (9.22) | 2042.50 (11.43) | 2002.0 (9.22) | tie |
| tai50_20_01 | 2868 | 2890.60 (0.79) | 2881.5 (0.47) | 2920.55 (1.83) | 2881.5 (0.47) | tie |
| tai50_20_02 | 2869 | 2981.75 (3.93) | 2964.0 (3.31) | 3012.97 (5.02) | 2974.0 (3.66) | **−10.0** |
| tai50_20_03 | 2755 | 2769.50 (0.53) | 2766.5 (0.42) | 2801.68 (1.69) | 2767.5 (0.45) | −1.0 |
| tai50_20_04 | 2702 | 2720.50 (0.68) | 2705.5 (0.13) | 2742.45 (1.50) | 2718.0 (0.59) | **−12.5** |
| tai50_20_05 | 2725 | 2757.60 (1.20) | 2754.5 (1.08) | 2782.78 (2.12) | 2757.5 (1.19) | −3.0 |
| tai50_20_06 | 2845 | 2876.15 (1.09) | 2871.5 (0.93) | 2903.43 (2.05) | 2879.0 (1.20) | −7.5 |
| tai50_20_07 | 2825 | 2866.30 (1.46) | 2855.5 (1.08) | 2890.08 (2.30) | 2855.5 (1.08) | tie |
| tai50_20_08 | 2784 | 2797.50 (0.48) | 2791.5 (0.27) | 2814.58 (1.10) | 2792.0 (0.29) | −0.5 |
| tai50_20_09 | 3071 | 3071.00 (0.00) | 3071.0 (0.00) | 3074.65 (0.12) | 3071.0 (0.00) | tie |
| tai50_20_10 | 2995 | 3049.55 (1.82) | 3042.5 (1.59) | 3070.90 (2.53) | 3042.5 (1.59) | tie |
| **MEAN RE** | | **4.19** | **3.73** | **5.33** | **3.85** | |

RE % = 100·(E[Cmax] − LB)/LB, identical formula and LBs as the paper Phase B
table (`cor_tabu_2026/scripts/gen_phaseB_tables.py:303`). Δbest = PR best − N2
best (makespan); negative = PR better.

## Three-way class means (RE %), incl. the cold IPRTS this branch tried to fix

| class | cold IPRTS avg | cold best | **PR avg** | **PR best** | N2 avg | N2 best |
|---|---|---|---|---|---|---|
| tai30×20 | 9.56 | 8.26 | **7.18** | **6.53** | 8.62 | 6.65 |
| tai50×20 | 3.64 | 2.56 | **1.20** | **0.93** | 2.03 | 1.05 |

(cold = Phase 2 `results/phase2_cold_tuned/`, irace cfg 187, random seeding, 30
runs × 900 s.)

Head-to-head on the best envelope (20 instances):

| | wins | ties | losses |
|---|---|---|---|
| cold vs N2 | 0 | 1 | **19** |
| **PR (warm+maxmin) vs N2** | 9 | 11 | **0** |
| PR vs cold | **19** | 1 | 0 |
| PR vs N2 (average) | **20** | 0 | 0 |

## Headline

- **The x20 weakness is recovered.** Cold IPRTS lost these classes outright
  (19/20 on best). Warm+maxmin is **never worse than TS-N2 on best** (9 wins,
  11 ties, 0 losses) and beats it on the **average of all 20**. Against the cold
  variant it ran *at the same 900 s budget* and dominates 19-1-0.
- Class-best RE drops below the baseline on both classes (tai30 6.53 vs 6.65,
  tai50 0.93 vs 1.05) and the average RE falls by ~1 pp (4.19 vs 5.33).

## Why the average gain (−1.14 pp) >> the best gain (−0.12 pp)

The two metrics measure different things and the gap between them is the real
story:

- **Best is floored.** Warm runs seed from N2's own best-of-run solutions, so
  the best can only tie the donor (11 instances where TS could not improve the
  seed) or beat it when PR+TS escapes the N2 optimum (9 instances, small
  margins). So "≥ N2 on best" is partly structural, not a clean search win.
- **The average gain is consistency, not records.** Within-method gap
  avg − best (in RE points): **PR 0.46 vs N2 1.48**. Every PR run starts from a
  good seed and lands near the best (low variance); N2 from scratch has a tail
  of mediocre runs that drags its 30-run average up. The whole extra average
  advantage (−1.02 pp) is exactly this variance reduction
  (0.46 − 1.48 = −1.02). In absolute makespan the spread is ~12 (PR) vs ~37
  (N2) on tai30×20.

## Decision: branch closed

The best-envelope improvement is marginal (mean RE best 3.73 vs 3.85; only 9 of
20 strictly better, all small) and is bought by a **two-stage pipeline**
(TS-N2 → IPRTS refinement) that costs the donor runtime *plus* a full 900 s, and
whose headline average edge is mostly run-to-run consistency inherited from the
warm seed. Not worth pursuing further as a separate line.

The standalone story remains the **cold irace-tuned IPRTS** (cfg 187), which
beats the baseline on its own on the 12 selectas (7 new best-knowns,
`cold_tuned_v1_summary.md`); warm/max-min seeding is, at most, an optional
hybridisation that buys consistency on the hardest x20 block, not new
best-knowns. Earlier related branches similarly retired: PR-over-N2 (worse,
`validation_v1_summary.md`) and the unbounded-vs-cap-40 PR walk
(`via1_cap40`, minor).
