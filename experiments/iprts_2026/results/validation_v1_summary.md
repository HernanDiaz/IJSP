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

## Ablation (a): PR candidates over N2 instead of N1 (same budget)

Mean Δavg jumps from **+1.35 % (N1) to +2.93 % (N2)**, worse on 12/12
instances; the three 15×10 wins disappear. Empirical confirmation of the
design argument: N2's block-boundary filter removes exactly the disagreeing
interior-block arcs the walk needs, truncating the relinking. PR needs the
navigation-complete extreme-critical set (N1); the improvement-filtered set
belongs in the TS. Results: `results/validation_v1_prN2/`
(setup `setup_IPRTS_val_prN2.txt`).

## Warm-start from the paper's stored solutions (Hernán's proposal)

Pool seeded from the Phase-B tuned TS-N2 run solutions
(`seeds/N2_tuned/<instance>_Sols.csv`, loaded by the new
`creation = ijsp.solutions-file` strategy, 20 % random mix). Same 5×60 s
budget:

- **Δavg −0.67 % mean: better average-E than the baseline's 30-run average on
  12/12 instances** (from −0.06 % on la40 to −1.86 % on abz8).
- d_best: ties the baseline best on 7/12, +0.1…+0.7 % on the rest. **No
  best-known value improved yet.**
- Caveats for the strong claim ("PR improves the baseline's converged
  solutions"): each warm run starts from a sample of the baseline's own 30
  solutions, so beating its *average* partly reflects per-run selection of
  good seeds. The clean open test is beating its *best* envelope.
## Full-budget warm experiment (30 runs × 900 s, rank-biased N2+N8 seeding)

Setup `setup_IPRTS_warm_full.txt`, results `results/warm_full_v1/`. RE % vs
the paper's lower bounds (same table as the supplementary material):

- **Mean RE (avg of runs): IPRTS 1.59 % vs TS-N2 2.40 %** — the paper's
  headline metric, reduced by a third (inherits partly from warm seeding).
- **Mean RE (best of runs): 1.56 % vs 1.50 %** — par overall, but with real
  news: **3 baseline best-knowns improved** — abz9 692.5 (was 694.5), la24
  940.5 (was 942.5), la40 1229.5 (was 1230.5) — solutions the donor solver
  never produced in 30×900 s, found by PR+TS from its own material. 5 ties
  (abz8, ft10, ft20, la21, la25); 4 losses (abz7, la27, la29, la38), all
  consistent with the seed-fidelity floor (stored bests not reconstructible
  from `_Sols.csv`, see below).
- Plateau (5000 cycles) stopped every run far below the 900 s cap
  (median 18–54 s small, 80–137 s large): the warm search saturates;
  remaining margin must come from machinery (deep TS) or tuning, not budget.
- Zero-variance rows (avg = best across 30 runs, e.g. la21 at 1052 with 8
  distinct schedules all at (982, 1122)) are the expected signature of
  "warm floor reached at seeding + stagnation stop", verified per-run.

## v2: elite-perturbation restarts (Hernán's exhaustion diagnosis → fix)

Hernán's reading of the full-budget warm runs — "the search is exhausted" —
was verified (most runs: exactly 5000 sterile cycles after seeding; the
cold runs on large instances are instead budget-limited, still improving at
the 60 s cap). Cause: stagnation re-seeds can't compete with the elite
level. Fix (commit `19e23be`): `pool.restart-perturbation` — restarts
refill by perturbing surviving elites (extract+reinsert genotype moves,
re-decoded + TS), plus a `PoolDist` evolution column.

**Cold quick, v1 → v2:** mean RE avg 3.80 → 3.11, best 3.02 → 2.22; gains
concentrate exactly on the large instances (la38 best 1234 → 1206.5, abz9
721 → 707, la29 1198 → 1181).

**Warm full budget, v1 → v2 (results/warm_full_v2_perturb/):**

- Mean RE avg-of-runs **1.69 vs baseline 2.40**; best-of-runs **1.41 vs
  1.50** — both aggregate metrics now beat the tuned baseline.
- Best envelope: **4 baseline best-knowns improved** — abz9 691.0 (−3.5),
  la27 1242.5 (−4.5), la24 940.5 (−2.0), la40 1229.5 (−1.0) — 7 exact
  ties, and abz7 at +0.5 as the only miss. The v1 fidelity-floor losses
  (la27/la29/la38) were all recovered or turned into wins.
- De-saturation visible in runtimes: abz8 now ~6 min/run mean (3 h for 30
  runs) vs ~2.3 min/run in v1 — the search uses the budget again.

- **Fidelity finding (append-vs-insertion test on abz7):** the stored files
  cannot reproduce the original objectives at all. The `_Sols.csv` writer
  stores `ScheduleIJSP::toString()` = the schedule's `taskOrder` sequence,
  which does not capture the final machine sequences (`taskInfo[].mp/.ms`);
  rebuilding abz7's 668 order yields ~672.5–673 with BOTH append and
  insertion SGS (and the SGS choice is immaterial in outcome — Hernán's
  Lamarckian intuition confirmed). The recoverable warm-start envelope on
  abz7 floors at ~672.5, 4.5 points above the stored best. Exact warm-starts
  would need the runs re-dumped with a writer that stores a topological
  order of the final schedule (future, additive change).
