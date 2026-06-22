# Seeding test — IPRTS warm + multi-solver bank (×20 Taillard): negative result

**Date:** 2026-06-21 · **Branch:** `experiment/Fable/IJSP`

**Question.** Can warm-starting IPRTS from a *multi-solver* seed bank
(TS-N2/N8 + Google OR-Tools CP-SAT) beat the record-best on the large ×20
instances (tai30×20, tai50×20), where cold IPRTS loses to the tuned TS-N2
baseline?

**Answer: no.** Across five independent probes, the warm best on the ×20
instances is a deep local optimum that neither richer seeding, path relinking,
exact recombination, nor stronger neighbourhoods can improve. Root cause: the
near-optimal solutions form a wide, **quality-neutral plateau**, and IPRTS's
intensification is **capped by its local-search neighbourhood**.

## Setup
- **Instances:** 20 ×20 Taillard interval instances (`tai30_20_*`,
  `tai50_20_*`, 15 % interval width).
- **Banks:** `N2N8` (best-of-run task orders of tuned TS-N2 + TS-N8,
  ~60/instance, regenerated from COR `exp4`); OR-Tools CP-SAT seeds (crisp
  lo/mid/up scalarisations + uniform interval sampling, ≤9/instance);
  `N2N8_ortools` (combined).
- **Algorithm:** IPRTS cfg187 (irace-tuned) — max-min diversity seeding, PR
  over N1 extreme-critical arcs, TS-N2 kernel.
- **New tooling:** `scripts/ortools_seeds.py`, `seed_novelty.py`,
  `disjunctive_disagreement.py`.

## The five probes

| # | Probe | Metric | Result |
|---|-------|--------|--------|
| 1 | Cross-solver seed diversity | novelty / within-N2N8 (Kendall-τ proxy) | **1.12×** — OR-Tools seeds overlap the N2N8 cluster (rich pass identical) |
| 2 | Warm A/B (N2N8 vs +OR-Tools) | best makespan, 3 tai50×20, 3×180 s | **Δ0** on all three; run-by-run ~identical |
| 3 | Path relinking | best vs time | best **frozen at the seed value**; ~500 PR calls / 340 k PR steps / 80–96 pool insertions, **0** record improvements |
| 4 | Exact-recombination fuel | disjunctive disagreement / consensus | ~40 % arcs contested but **freeOps ≈ 99–100 %** (even top-5 ≈ 97 %) — no consensus backbone to fix |
| 5 | Stronger LS kernel | best, n2/n3/n8/next/n2plus | **all → 2881.5** (= warm seed) on tai50_20_01; n3/n8 confirmed at full 180 s |

## Diagnosis
1. **Neutral plateau.** The near-optimal ×20 solutions disagree on ~40 % of
   disjunctive arcs (even the top-5 share no consensus core — ~97 % of
   operations are touched by some contested arc) yet have ~equal makespan. The
   structural diversity is *quality-neutral*: flipping contested arcs does not
   change the objective.
2. **PR capped by the kernel.** Every PR product is re-decoded and TS-N2'd, so
   it collapses back to an N2-local-optimum. The warm seeds are already the
   best of 30 TS-N2 runs, so PR+TS merely resamples the N2-optimal manifold and
   cannot beat them. Diversity gives PR paths to walk, but it cannot relink
   *out* of the local-optimum class.
3. **The cap is the whole neighbourhood family.** A stronger LS kernel (N3, N8,
   Next, N2plus) returns the exact same warm best — the optimum is robust
   across the family, not specific to N2.

## Implication
There is a ~2–3.6 % gap to the lower bound on tai50×20 (cold-tuned results), so
room exists in principle — but it lies on the far side of the plateau,
unreachable by recombining/refining the existing near-optima. Improving
record-best would need a search able to *leave* the plateau (a much larger or
coordinated move, or a different metaheuristic), outside the warm-PR /
multi-solver-seeding family.

**Decision.** The warm / multi-solver-seeding / exact-recombination line is
closed. The standalone keeper from the IPRTS line remains the **cold-tuned
cfg187** (7 new best-knowns on the 12 selectas, `cold_tuned_v1_summary.md`).
This negative result is a clean, reusable ablation.

## Artifacts (branch `experiment/Fable/IJSP`)
- **Generators / analysis:** `scripts/ortools_seeds.py`, `seed_novelty.py`,
  `disjunctive_disagreement.py`
- **Runners:** `scripts/run_{ortools_gate,ortools_rich,seedtest_ab,kernel_screen,kernel_confirm}.sh`,
  `regen_n2n8.sh`
- **Setups:** `setup/setup_seedtest_{n2n8,combo,smoke}.txt`
- **Banks (regenerable):** `seeds/{N2_tuned,N8_tuned,N2N8_tuned,ortools_x20,ortools_x20_rich,N2N8_ortools_x20}`
- **Results:** `results/{seedtest,kernelscreen,kernelconfirm}`

**Operational note.** Large neighbourhoods (`nh`/`next`) on 1000-op instances
blow up memory and can freeze the machine; keep parallelism low and avoid them
on large instances.
