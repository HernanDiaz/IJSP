# N8-based mutation operator — investigation summary (SHELVED)

**Conclusion (2026-06-18): the N8 mutation operator is correct and cheap, but does
NOT outperform the standard mutation operators when each is fairly (irace-)tuned.
Insertion is the best mutation for the GA. The line is abandoned.**

Branch: `experiment/crossover-mutation` (off `feature/IJSP`).

---

## Hypothesis

N2/N8 are the best critical-path neighbourhoods in the COR/ASOC paper, so use N8's
extra-block reinsertion moves as a **mutation** operator (critical-block-aware
diversification) to beat the generic random mutations (Swap/Insertion/Inversion)
in population-based algorithms.

## What was built (additive only)

- **`MutationIJSP_N8.{h,cpp}`** — IJSP-specific mutation. Per call: decode →
  identify critical blocks (O(n) BFS, mirrors N8) → apply **k random, UNCLIPPED**
  extra-block reinsertions (move a critical-block boundary task to the opposite
  end of its block, or to a machine extreme) → recompute heads (cycle-guarded) →
  re-encode. No move estimation/clipping (a mutation must be free to worsen).
  Registered `ijsp.n8` in `GeneticClassRegister`; perturbation strength parameter
  `mutation.n8.moves` (k). No algorithm code changed.
- **EvoLauncher `postexecution` guard** — ported the `postexecution = no` switch
  (3 guards around the robustness analyzer). It existed only on the abandoned
  IPRTS branch, not on `feature/IJSP`, so all the `postexecution=no` setups were
  silently running the (slow) robustness analysis.
- **`experiments/irace_ga/`** — per-operator GA tuning infrastructure (LEX2, 120 s,
  the paper's 20 training instances): base setup, parameter spaces, target-runner
  (adapted from the paper's), scenario, `run_irace_ga.sh`.

## Experiments and results

**1. Smoke (FT10, ABCPSO+TS-N2):** runs clean, reaches optimal-quality 935.5.
Operator correct, never corrupts schedules.

**2. k-sweep (tai20_20_01, ABCPSO+TS-N2, 25 s, 10 runs):** inverted-U — k=1-2 ≈
insertion; **k=3 sweet spot** (best RE 2.65 % vs insertion 3.99 %, avg also best);
k≥4 degrades (too disruptive). Iso-time. → promising at SHORT budget.

**3. Full drop-in vs published TS-N2 (82 instances, ABCPSO+TS-N2, only
mutation→N8 k=3, 30×900 s, prob 0.10):** essentially a **TIE** (global RE avg 3.67
vs 3.68, best 2.59 vs 2.55); worse on tai30_20 (best +0.31 pp). The short-budget
k=3 advantage did NOT hold at full budget.

**4. Why the tie — ABCPSO code finding:** ABCPSO applies the mutation to EVERY food
source UNCONDITIONALLY (`ArtificialBeeColonyPSO.cpp:556` — no probability gate, so
`mutation.probability` is ignored) and accepts the result ONLY if it improves
(greedy, lines 580-598; the SA branch needs `simulatedCooling`, off here). The
**greedy acceptance neutralises any diversifying mutation**, so a drop-in mutation
cannot help the SOTA host. (ABCPSO diversifies via the scout/restart phase, not the
mutation.) → motivated testing in a generational scheme where mutations persist.

**5. GA test (generational replacement → mutations persist):** switched to the pure
GA (Hernán's working config adapted to makespan). Fixed along the way: the
postexecution switch; `Tournament` selection requires `selection.tournament-size`;
the pure GA ignores the localsearch params (weak, RE ~10-12 %) and converges fast.
At a fixed prob 0.05, N8 slightly beat Swap — but that is an unfair comparison
(the optimal rate differs per operator).

**6. Per-operator irace tuning (GA, LEX2, 120 s, 20 training instances, 1000
experiments each).** Tuned each operator's GA separately over `population.size`,
`crossover` (jox/gox/ppx), `crossover.probability`, `mutation.probability`,
`selection.tournament-size` (+ `n8.moves` for N8). Best configs:

| operator | pop | cxpb | mutpb | tourn | crossover | k |
|---|---:|---:|---:|---:|---|---:|
| N8 | 384 | 0.986 | 0.244 | 2 | jox | 2 |
| Swap | 340 | 0.968 | 0.379 | 4 | gox | — |
| Insertion | 372 | 0.987 | 0.355 | 2 | gox | — |
| Inversion | 318 | 0.970 | 0.373 | 2 | gox | — |

This **validated the per-operator approach**: the optimal mutation rate differs
(N8 ~0.24 — strong, needs less; the generics ~0.35-0.38 — weak, need more); N8
prefers JOX, the generics GOX; N8's optimal k drops to 2 once everything else is
tuned.

**7. Best-vs-best validation (20 instances, 10 runs, RE % vs LB):**

| operator | RE avg | RE best | best-wins |
|---|---:|---:|---:|
| **Insertion** | **9.32** | **7.47** | **13/20** |
| Swap | 10.20 | 8.38 | 2/20 |
| Inversion | 10.59 | 8.54 | 2/20 |
| **N8** | 10.94 | 8.62 | 3/20 |

**Insertion wins clearly; N8 is the worst on average.**

## Conclusion

With each operator fairly tuned, **the N8 mutation does not beat the standard
mutations** — Insertion is best. The earlier promising signals (k=3 at 25 s;
N8>Swap at a fixed prob 0.05) were artifacts of unfair / short-budget comparisons.

**Why (a principled reason):** a good LS *neighbourhood* and a good *mutation* have
opposite goals. N8 is built for INTENSIFICATION — small, targeted, "likely-to-
improve" critical-block moves (great for the tabu search, where it already is one
of the best). A MUTATION needs DIVERSIFICATION — broad, decorrelated jumps. N8's
structural, correlated moves diversify LESS than a random insertion, so they make a
worse mutation. The premise "good neighbourhood ⇒ good mutation" was flawed; N8
belongs in the local search, not the mutation.

**Unverified alternative (not pursued):** the mutation works at the schedule level
then re-encodes to a genotype that the GA re-decodes with the insertion SGS; the
SGS (an active-schedule builder) may partially "wash out" the specific N8 move (the
known Lamarckian-fidelity issue). N8's results differ from the generics (so it is
not a no-op / not a total washout), but a partial washout was not ruled out. A
quick check — compare the mutated schedule's makespan before encode vs after
re-decode — would settle it.

## Caveats

- The GA validation is on the 20 *training* instances (fair across the 4 operators,
  but with slight overfitting optimism); revalidating on the 82 (held-out) was
  offered but not done.
- Pure GA (no LS) → weak absolute quality (RE ~9-11 %); the ranking could differ
  inside a memetic / with local search.

## Artifacts (on this branch; recoverable only if the branch is pushed before abandoning)

- Operator: `MutationIJSP_N8.{h,cpp}`, registered in `GeneticClassRegister.h`, Makefile.
- `EvoLauncher.cpp` postexecution guard.
- irace: `experiments/irace_ga/` (+ results in `results/irace_{N8,Swap,Insertion,Inversion}/`).
- Setups/runners: `experiments/setup_*`, `experiments/run_*` (smoke, k-sweep, drop-in 82,
  GA comparison, best-vs-best validation).
