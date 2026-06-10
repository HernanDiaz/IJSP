# IPR-TS — Interval-aware Path Relinking Tabu Search for the IJSP

**Status:** design draft (pre-implementation) · branch `experiment/Fable/IJSP` · 2026-06-10

## 1. Goal and hypothesis

Build an elite-pool tabu search with path relinking (PR) for the Interval Job Shop
Scheduling Problem, in the spirit of i-TSAB (Nowicki & Smutnicki, 2005) for the
deterministic JSP, with the relinking guided by **extreme critical arcs** — the
structural concept introduced in the COR 2026 paper.

**Bar to beat:** the irace-tuned TS-N2/LEX2 inside ABCPSO (current in-house and
published-literature SOTA; see `experiments/cor_tabu_2026/`). Hypothesis: replacing
the generic ABCPSO population by an elite pool + structured PR recombination
yields equal quality on small instances and significant improvements (plus lower
variance) on large Taillard classes (tai50×20, tai100×20), where single-trajectory
TS stalls within the 900 s budget.

**Novelty claim for the paper:** first PR for interval JSP; PR steps and distance
defined on the disjunctive-graph orientation and restricted to extreme critical
arcs, inheriting the feasibility/connectivity guarantees of H(σ) proved in the
COR paper. Secondary contribution: bandit-adaptive neighbourhood choice (N2/N8)
resolving the paper's N2≈N8 tie without per-class tuning.

## 2. Algorithm specification

```
IPRTS(params):
  # --- initialisation ---
  pool ← ∅
  while |pool| < λ:
      x ← creation.createIndividual()          # CreationIJSP_Manager mix
      x ← TS(x)                                # LS_Tabu + tuned neighbourhood
      insert x into pool if d(x, e) ≥ dmin ∀e∈pool (else retry, give up after r tries)
  best ← best(pool)

  # --- main loop ---
  while not stop():                            # timelimit / noimprovement (pool best)
      (a, g) ← selectPair(pool)                # random pair, d(a,g) ≥ dmin; guide g = better one
      c  ← PathRelink(a → g)                   # best intermediate found on the walk
      c′ ← TS(c)
      updatePool(c′)                           # quality + diversity rule (below)
      if avgPairwiseDistance(pool) < θ·D_total: restartWorstHalf()
  return best
```

### 2.1 Distance d(σ, τ)

Number of disjunctive pairs ordered differently in the two schedules
(machine-wise Kendall-τ): for each machine, map σ's task sequence through τ's
position index and count inversions (O(n log n) per machine, merge-sort count).
`D_total = Σ_m n_m(n_m−1)/2`. `dmin` and the restart threshold θ are expressed
as fractions of `D_total`. Orientation data comes straight from
`ScheduleIJSP::taskInfo[t].mp/.ms` (public, see `ScheduleIJSP.h`).

### 2.2 PR walk (the core new operator)

The walk *drives an existing `Neighbourhood` object* — same mechanism LS_Tabu
uses (`setInitialSolution → findNewNeighbours → getEstimation →
evaluateNeighbour → acceptNeighbour`), with one extra filter:

```
PathRelink(a → g):
  σ ← a;  bestInter ← a
  precompute pos_g[task] (position of each task in g's machine sequence)
  repeat:
      C ← extreme-critical candidate arcs of σ        # nb->findNewNeighbours
      D ← { (x,y) ∈ C : pos_g[y] < pos_g[x] }          # disagree with guide
      if D = ∅: break
      rank D by heads&tails estimation                 # nb->getEstimation
      pick best-estimated arc, exact-evaluate, accept  # nb->acceptNeighbour
      if f(σ) better than f(bestInter): bestInter ← σ
  return bestInter
```

Key points:

- **Candidate generator = `ijsp.makespan.n1`** (all extreme critical arcs) by
  default: PR must not lose disagreeing arcs to N2's block-boundary filter.
  Configurable (`pr.neighbourhood`) so the N1-vs-N2-filtered ablation is one
  setup-file change.
- **Feasibility for free:** every step reverses an extreme critical arc, so by
  the H(σ) feasibility theorem (COR paper §"Feasibility") no acyclicity check is
  needed. Debug builds additionally call `verifyScheduling()` after each accept.
- **Termination:** D shrinks towards ∅ as σ aligns with g on its extreme
  critical arcs. The walk does NOT necessarily reach g (it stops when no
  extreme-critical disagreement remains) — that is fine: the goal is the best
  intermediate, and stopping there saves time. Hard cap: `pr.max-steps`
  (default: d(a,g) steps).
- **Cost:** one walk ≈ one short TS run with a pre-filtered candidate list;
  estimation discipline identical to the tuned TS (`localsearch.filter = yes`
  semantics reused).

### 2.3 Pool management

- `λ = pool.size` individuals, kept sorted by fitness (LEX2 ranking, as tuned).
- **Insertion rule:** c′ enters iff better than the worst element AND
  d(c′, e) ≥ dmin for all e; it replaces the worst element. If c′ is better than
  some e but d(c′, e) < dmin, it replaces e iff better than e (clustering guard).
- **Restart (v1, revised):** the original distance-threshold restart
  (avg distance < θ·D_total) **can never fire** — the insertion rule keeps all
  pairwise distances ≥ dmin, so the average never falls below it. v1 uses a
  **stagnation patience** instead (`pool.restart-patience`): after that many
  consecutive cycles without any pool insertion, keep the best ⌈λ/2⌉ and
  re-seed the rest from creation + TS. This is the ABC scout-phase analogue and
  proved essential: without it the pool freezes after seeding (9 insertions in
  3 500 cycles on FT10) and the search loops over identical walks. With it,
  FT10 smoke went from ~1 025 to **940** (= paper-quality) in 10 s.

### 2.4 v2 (optional, after v1 results): adaptive N2/N8 inside TS

Two `LS_Tabu` clones configured with `ijsp.makespan.n2` and `ijsp.makespan.n8`.
Before each TS call, choose the neighbourhood by UCB1 with reward = relative
improvement per CPU second. Addresses the paper's N2≈N8 statistical tie (N8 wins
tai15×15) without per-class tuning. Strictly additive: `ls.adaptive = no`
reproduces v1.

## 3. Mapping onto the framework

### Reused as-is (no modifications — baseline stays byte-identical)

| Need | Existing piece |
|---|---|
| TS kernel | `LS_Tabu` + `localsearch.*` params (tuned values from `setup_N2_tuned.txt`) |
| Move engine + estimation | `NB_ParallelN1/N2/N8_MakespanIJSP` (`findNewNeighbours`, `getEstimation`, `acceptNeighbour`) |
| Seeds | `CreationIJSP_Manager` (SPJF/LRTF mix) via `creation` param |
| Encode/decode | `EncoderIJSP_JobOrder` / `DecoderIJSP` + `SGS_IJSP_Insertion` |
| Ranking | `evaluation.interval.comparison = LEX2` machinery |
| Runner, CSV, N runs, seeds | `EvoLauncher` (untouched) |
| Stopping/statistics conventions | mirror `GeneticAlgorithm` fields (`timelimit`, `noimprovement`, evolution spans) |

### New files

| File | Contents | Size est. |
|---|---|---|
| `PathRelinkIJSP.h/.cpp` | distance (inversion count), pos-index precompute, the walk of §2.2 driving a cloned `Neighbourhood` | ~250 loc |
| `ElitePool.h/.cpp` | vector of `Individual*` + insertion/restart rules + avg-distance | ~150 loc |
| `IPRTS.h/.cpp` | `class IPRTS : public FuzzyFW::EvolutiveAlgorithm` — prepareToRun (loads encoder/decoder/creation/LS/PR params, then base for evaluator+stats), run() loop of §2, getStatistics/getRuntime/getEvolution mirroring GA conventions | ~450 loc |
| `experiments/iprts_2026/setup/setup_IPRTS_base.txt` | N2-tuned TS params + PR params below | — |

### Edits (2 lines each)

- `AlgorithmClassRegister.h`: `#include "IPRTS.h"` + register `"IPRTS"` (alias `"PR-TS"`).
- `Makefile`: add the three `.o` targets to the object list.

### New parameters (dotted style consistent with existing ones)

| Param | Type / range | Default | irace? |
|---|---|---|---|
| `pool.size` | int 6–20 | 10 | yes |
| `pool.min-distance` | real 0.01–0.20 (fraction of D_total) | 0.05 | yes |
| `pool.restart-patience` | int 10–500 cycles (−1 = off) | 50 | yes |
| `pool.seed-retries` | int | 5 | no |
| `pr.neighbourhood` | string | `ijsp.makespan.n1` | ablation only |
| `pr.max-steps` | int, −1 = d(a,g) | −1 | no |
| `localsearch.max-rounds` | int (−1 = unlimited) | 50 (IPRTS setups) | yes |
| `ls.adaptive` (v2) | yes/no | no | phase 3 |
| `ucb.c` (v2) | real 0.25–2.0 | 1.0 | phase 3 |

### v1 implementation findings (kernel differs from the COR setups)

Smoke-testing on FT10 exposed three facts that reshape the TS kernel inside
IPRTS relative to the values tuned for ABCPSO:

1. **`LS_Tabu` chains die early by design.** The tabu list has unlimited max
   size in the tuned setups and `apply()` exits permanently on the first
   "dead end" iteration (every candidate tabu/reverse-of-last). With the
   small block-filtered candidate lists this happens after ~15 iterations, so
   `localsearch.bad-iterations` is rarely the binding constraint. Inside
   ABCPSO that is fine (the population provides depth); an elite-pool method
   starves. v1 adds `localsearch.max-rounds`: `applyLocalSearch()` re-applies
   the LS (fresh tabu list each round) while the result keeps improving.
2. **`localsearch.filter = yes` compounds the early exits** in the elite
   context (the estimation filter cuts the candidate scan at the first tabu
   best-estimated move). IPRTS setups use `filter = no` (classic full-scan TS
   iteration) plus bounded tenure `localsearch.tabu-size.max = 8`,
   `bad-iterations = 100`.
3. **PR products are re-decoded through the insertion SGS** before TS (same
   repair/improvement path crossover offspring get in the population
   algorithms), instead of keeping the raw relink schedule.

These values are hand-picked sane defaults pending Phase 1 (irace).

## 4. Experimental plan

- **Phase 0 — smoke + first validation (validation set fixed by Hernán: the 12
  selected classical instances):** plumbing smoke on FT10 (`F0.15.0.ft10_10.txt`,
  LB 930, expect ≤940 in seconds); then **all 12 selectas** (abz7/8/9, ft10, ft20,
  la21/24/25/27/29/38/40) — first at reduced budget to iterate, then 5 runs at
  paper budget — compared against the paper's
  `statistical_results/per_instance_stats.csv`. Gate: never worse than TS-N2 mean
  by >1% on any of the 12 instances.
- **Phase 1 — irace tuning:** reuse `experiments/cor_tabu_2026/irace/` as template
  (same tuning instances and budget for fairness); tune the 3 pool params +
  `localsearch.bad-iterations` + `localsearch.tabu-size.min` jointly.
- **Phase 2 — full comparison:** 82 instances × 30 runs × 900 s vs TS-N2.
  **Baseline is NOT rerun** — the paper's `runs_data.csv` is the comparison data
  (same machine assumption must hold; otherwise rerun N2 setup only). Wilcoxon +
  Friedman, reusing the analysis scripts as templates (new copies under
  `experiments/iprts_2026/scripts/`, never editing the frozen ones).
- **Phase 3 — ablations:** (a) PR on extreme-critical vs all disagreeing arcs;
  (b) pool without diversity rule; (c) `ls.adaptive` on/off; (d) optional IG
  (destroy/repair) baseline.
- All results under `experiments/iprts_2026/results/` (gitignored raw, aggregated
  CSVs committed), mirroring the cor_tabu_2026 layout.

## 5. Implementation phases and validation gates

1. **Skeleton:** `IPRTS` with pool + TS but PR replaced by random elite restart.
   Gate: runs end-to-end on FT10, CSV well-formed, no leaks (valgrind on small run).
2. **Distance + pool rules.** Gate: unit checks — d(σ,σ)=0, symmetry,
   d ≤ D_total, pool never holds two elites with d < dmin.
3. **PR walk.** Gate: walk monotonically reduces |D|; `verifyScheduling()` clean
   over 10⁴ accepted PR steps across instances of all sizes; best-intermediate
   bookkeeping correct.
4. **Statistics/evolution wiring + setups + Phase 0.**
5. **irace + Phase 2 runs** (mostly wall-clock waiting; script first).
6. **v2 adaptive** only if v1 ties or beats TS-N2 overall.

## 6. Risks and mitigations

- **PR intermediates rarely beat TS restarts** → back-and-forth relink, relink
  only between TS local optima (already the case), larger λ; worst case the
  paper pivots to "adaptive neighbourhood TS" (v2) as the headline.
- **Walks too short on small instances** (few extreme-critical disagreements) →
  fall back to non-critical disagreeing arcs *with* acyclicity check, as ablation (a).
- **Baseline machine mismatch** for Phase 2 → rerun the single N2 setup
  (5 × 82 × 900 s ≈ manageable) to calibrate.

## 7. Decisions taken (overridable)

- PR in **disjunctive (solution) space**, not genotype space — exploits the
  paper's theory and the existing move engine; genotype-level PR kept only as a
  possible cheap ablation.
- New algorithm registered as `IPRTS`, inheriting `EvolutiveAlgorithm` directly
  (not `GeneticAlgorithm`): avoids dead GA operators and their setup checks.
- Pool stores TS local optima only (relink between optima, i-TSAB style).
- Doc and code comments in English (repo convention); experiment notes may be ES.
