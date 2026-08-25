## Recommendation

**Major revision.** The core experiment is unusually broad, the principal instance-level quality analysis is substantially sound, and several important implementation claims survive source inspection. However, the survival analysis ignores the paired instance structure, the convergence classification is based on incomplete population snapshots rather than the returned best solutions, budget calibration reuses evaluation instances despite stating otherwise, and the manuscript contains several material internal/code discrepancies. These defects affect the strength of the time-to-target, convergence-conditioned, and budget-dependent conclusions, but they appear repairable without rerunning the entire experiment.

## Summary of the contribution

The manuscript studies whether replacing part or all of the initial population of four IJSP metaheuristics with learned or heuristic schedules improves performance. It compares an unseeded control, four individual seed sources, a half-seeded learned arm, and a three-source mixture across 61 instances and 30 runs per cell. Its most interesting result is not that “better seeds help,” but that the effect depends on the solver and budget: MIX helps ABCE3, is neutral for fEABC-LS, is approximately neutral at the final GA budget despite harming all single-source arms, and gives a small non-significant aggregate advantage on TS-N2. The manuscript further argues that MIX’s composition, rather than its mean initial quality, is associated with its advantage.

## Major issues

### 1. The log-rank analysis does not respect the paired instance design

**Claim.** Time to the control’s final quality is analysed “with the instance as unit,” using right censoring, Kaplan–Meier medians, and log-rank tests against A0 (manuscript lines 1043–1057 and 1100–1108).

**Evidence.** The script correctly creates one observation per instance and arm and censors failures at 1.0 (`final/ttt_survival.R:21–30`). However, it then applies ordinary

`survdiff(Surv(t, ok) ~ arm)`

to the two arms (`final/ttt_survival.R:48–50`). This treats the A0 and seeded samples as independent groups. They are paired by instance, share an instance-specific target, and the A0 target is constructed from that same instance’s A0 endpoint (`final/ttt_survival.R:21,27`). No instance strata, clustered variance, paired permutation, or within-instance resampling is used. The proportional-hazards/log-rank suitability is also not assessed, even though the manuscript explicitly describes crossing anytime behaviour for the GA.

**Why this matters.** The Kaplan–Meier descriptive medians and reach rates remain informative, but the reported log-rank p-values are not valid tests for the experimental design. In particular, statements such as fEABC-LS being significant at \(p=0.010\), or all GA arms being significantly slower, should not rest on these p-values. The common target construction also makes A0 a special dependent reference group rather than an ordinary survival sample.

**Fix.** Retain reach rates and censored descriptive summaries, but replace the inferential comparison with an instance-paired procedure. Reasonable options include a paired permutation test on a pre-specified restricted mean time-to-target, with failures assigned the budget limit; an instance bootstrap of the paired RMST/reach difference; or a suitable marginal survival model with clustered variance and a clear estimand. If a proportional-hazards model or log-rank test is retained, demonstrate that its assumptions are reasonable and account for pairing. Correct all time-to-target claims accordingly.

**Threat level:** threatens the inferential time-to-target conclusions, not the final-quality conclusions.

---

### 2. The convergence rule is applied to a proxy that the manuscript admits can miss the solver’s retained best solution

**Claim.** A control cell is converged when its objective improves by less than 0.1% during the last tenth of the budget; 106/244 cells are classified as anytime-only (manuscript lines 533–540).

**Evidence.** `convergence.R` uses the last resampled `bestcmax` at or before \(0.9T\) and the final resampled point (`final/convergence.R:30–34`). Those curves are built from the best member of the current population, not necessarily the best solution retained by the algorithm (`final/resample_anytime.sh:49–56`). The manuscript itself reports that the curve endpoint differs from final returned quality by a median 2.20% for ABCE3 (manuscript around lines 951–958), far larger than the 0.1% convergence threshold.

The script therefore tests whether the sampled population snapshot stabilises, not whether the solver’s best-so-far objective stabilises. Averaging 30 traces before applying the rule can also hide continuing progress in a subset of runs.

**Why this matters.** The paper conditions several substantive conclusions on “converged” cells, including the GA reversal and claims about where final-quality evidence is admissible. A proxy with endpoint error twenty times the threshold cannot reliably support those classifications, especially for ABCE3. A population curve can plateau while the separately retained elite improves, or fluctuate while the true best-so-far has already stabilised.

**Fix.** Reconstruct per-run incumbent-best traces including the final returned solution, then assess the rule per run and aggregate with a pre-specified cell criterion—for example, a stated proportion of runs satisfying the rule—or construct a run-averaged incumbent trace with the true endpoint appended. Provide a sensitivity analysis over reasonable thresholds and windows. Until then, replace “converged” with a more limited label such as “population-snapshot plateau,” and weaken all conclusions based on the 106/244 classification.

**Threat level:** threatens convergence-conditioned conclusions and the interpretation of budget sufficiency.

---

### 3. Budget calibration is outcome-dependent, uses evaluation instances, and is asymmetric in favour of the control’s timescale

**Claim.** Budgets are calibrated on one or two control instances per solver/size class, avoiding calibration on the 61 evaluation instances themselves (manuscript lines 432–450).

**Evidence.** The calibration instances listed in `final/phase1_budgets.sh:20–21`—for example `tai15_15_01`, `tai20_20_02`, `tai30_20_04`, `tai50_20_01`, and `tai50_20_05`—are members of the final 61-instance evaluation set. Thus the statement that the protocol “does not require calibration runs on the 61 evaluation instances themselves” is false.

Calibration is also based solely on the control’s time to within 0.1% of its own eventual pilot endpoint (`final/phase1_budgets.sh:6–9`). GA/fEABC-LS calibration uses only five runs (`final/run_phase1_calib.sh:3–4`), whereas ABCE3/TS-N2 sources come from a different phase with 30 runs. The final hard-coded caps are visible in `final/run_phase2c.sh:23–26`.

**Why this matters.** Using control data to choose the comparison horizon is not automatically unfair—indeed, it is defensible if the estimand is performance at the control’s timescale—but it is asymmetric. It can disadvantage seeded algorithms that improve on a different timescale, and using included evaluation instances creates modest budget-selection leakage. More importantly, the selected budget still fails the paper’s own plateau rule for 106/244 controls, so the calibration often did not achieve its stated purpose.

The existing caveat discusses the 900-second cap and portability, but not the reuse of evaluation instances, mixed 5/30-run calibration precision, or the estimand induced by control-based stopping.

**Fix.** Correct the factual description. Explicitly identify calibration instances and run counts. Frame the endpoint as “performance at a control-calibrated horizon,” not a neutral approximation to convergence. Add sensitivity results at multiple budget fractions or longer available horizons, preferably with calibration instances excluded from confirmatory summaries. The excellent anytime data make this repair feasible.

**Threat level:** affects interpretation and external validity more than the raw fixed-budget comparisons.

---

### 4. The lower-bound argument is mathematically defensible, but the instance-mapping audit is insufficient to establish the claimed mapping

**Claim.** Published crisp Taillard lower bounds are valid references for the interval objective, and the instance mapping was “verified rather than assumed” (manuscript lines 487–522).

**Evidence.** The mathematical inequality is sound under the implemented semantics: interval addition is component-wise (`Interval.cpp:128–130`), component-wise maximum computes the two endpoint maxima (`Interval.cpp:280–287`), and midpoint is \((a+b)/2\) (`Interval.cpp:342–345`). Consequently the crisp optimum is a valid lower bound for the midpoint of the interval makespan.

However, `final/check_ta_mapping.sh` verifies only that a recomputed trivial job/machine-load bound does not exceed the published bound. Equality identifies some instances, but inequality is merely necessary and does not distinguish among many possible Taillard instances. The script itself labels these cases only “coherent,” not exact. `ta_lb.csv` also maps the repository’s `tai20_20_01` to TA21, etc.; this is plausible given exclusion of TA11–TA20, but not proven by the weak-bound check.

**Why this matters.** A wrong mapping would change every RPD for that instance. The absence of below-bound runs is another necessary consistency check, not a mapping verification. The manuscript acknowledges part of this limitation but still uses stronger language than the audit supports.

**Fix.** Compare complete crisp processing-time matrices and machine routes against authoritative Taillard instance files, ideally by hashes or a released deterministic mapping table. Report exact identity for all 60 instances. Retain the crisp-LB justification, which is correct, while describing the bounds as valid but sometimes loose references rather than interval lower bounds specifically developed for this uncertainty model.

**Threat level:** potentially conclusion-threatening if the mapping is wrong; currently not fully verified.

---

### 5. Several headline and mechanistic statements exceed what the design establishes

**Claim.** “What predicts” the seeding effect is whether variation operators can consolidate a heterogeneous population; MIX’s superiority implicates composition rather than quality (manuscript lines 133–143 and Discussion).

**Evidence.** The manuscript later provides more appropriate caveats, acknowledging that GA and ABCE3 differ in selection, replacement, mutation, and other population dynamics, and that quality-matched diversity ablations were not performed. The GA diversity instrumentation covers only four instances and two solvers. Moreover, the claim that MIX contains no material absent from the single-generator arms is true at the pool-source level, but the single arms do not receive the same entries in the same runs: MIX takes the first interleaved 342/341/341 entries, while each single-source arm cycles through its own 1024-line pool under different block arithmetic.

**Why this matters.** The evidence supports an association between mixed-source initialisation and later performance; it does not establish consolidation ability as a predictor or pool diversity as the operative causal variable. The introduction and conclusion state the mechanism more strongly than the limitations allow.

**Fix.** Use consistently associative language throughout the abstract, introduction, and conclusion. Replace “what predicts it is…” and “composition beats quality” with a bounded finding about this tested mixture. A causal mechanism would require quality-matched pools, two-source mixtures, diversity manipulation, and solver/operator ablations.

**Threat level:** interpretation/exposition; the empirical MIX ranking itself remains intact.

---

### 6. The submitted manuscript is internally inconsistent and visibly unfinished

**Claim/evidence.**

- The abstract reports a 37–53% reduction at one tenth of budget (lines 56–57), matching the values in the anytime table and text (around lines 918–972). The introduction instead says 28–44% (lines 152–154). The former is supported; the latter appears stale.
- The source contains unresolved TODOs for the replication table (`main.tex:273`), FTOP outcome (`main.tex:429`), references, seed-generation time, and availability metadata; the DOI is still “[DOI pending]” (`main.tex:1563`).
- The manuscript says configurations are replicated “verbatim” (`main.tex:237`), yet openly reports ABCE3 being 1.5–2.5 percentage points worse than published work and supplies no replication table. “Verbatim” should be confined to parameter settings, not reproduction of the published implementation or behaviour.
- The seed-generation cost is explicitly unmeasured, although the strongest practical claim concerns only 10% of already short budgets.

**Why this matters.** These are not cosmetic in a Q1 submission. The missing replication evidence and generation costs directly limit reproducibility and practical interpretation.

**Fix.** Resolve every TODO, add the replication table, correct the contradictory percentage range, provide immutable code/data identifiers, and either measure per-instance seed-generation cost or explicitly present the 10%-budget result as excluding all seed acquisition costs.

**Threat level:** mainly manuscript readiness, with replication evidence substantively important.

## Minor issues

1. `stats_robusto.R` does use the instance as the analysis unit: it averages the 30 runs per instance/arm, forms 61 arm-minus-control differences, applies signed-rank tests, and bootstraps instances (`final/stats_robusto.R:32–45`). The corresponding ABCE3/MIX estimate, \(-1.140\) RPD points with percentile CI approximately \([-1.361,-0.930]\), agrees with `efectos_ic.csv`. This part should be retained.

2. The signed-rank test requires a symmetric distribution of instance-level differences, not merely a median-zero null. The manuscript asserts symmetry without checking it. Show the difference distributions and add a sign-test or paired permutation/bootstrap sensitivity analysis.

3. Holm correction is applied per solver over six arms (`stats_robusto.R:37`), while a second global 24-test Holm correction is also computed (`stats_robusto.R:59`) but not foregrounded. State clearly which family is confirmatory. Encouragingly, `efectos_ic.csv` shows the main ABCE3/MIX result survives the global correction.

4. Percentile bootstrap CIs are acceptable but not ideal at \(n=61\). BCa or bootstrap-\(t\) intervals would be a useful sensitivity check.

5. `early_budget.R` is stale and uses `refs.csv` best-known values rather than `ta_lb.csv`, contrary to the paper. The published 37–53% table appears instead to come from `fig_anytime_all.R`, which correctly uses `ta_lb.csv`. Remove or repair the stale script so the released pipeline has one authoritative implementation.

6. `early_budget.R` also uses fractions 5/10/25/50/100, whereas the manuscript table uses 10/25/50/75/100. Again, identify the authoritative script.

7. The anytime resampling correctly applies a per-run cumulative minimum (`resample_anytime.sh:49–56`) and emits means only once all runs have reported (`resample_anytime.sh:86–95`). This avoids the most obvious non-monotonicity and changing-composition errors.

8. In `fig_anytime_all.R`, when no trace point exists before a requested fraction, the code substitutes the first available value rather than `NA`. At the 10% cut this may be harmless, but it should be reported and checked explicitly.

9. `resample_anytime.sh` advances chunk run offsets by a hard-coded five (`line 70`). The current `chunk_size()` also returns five, but comments in `run_phase2c.sh` say fEABC-LS/TS-N2 use chunks of three. Remove this latent inconsistency or derive offsets from actual chunk run counts.

10. The verifier sample uses only `c1`, i.e. five runs per selected cell (`verify_broad_indep.sh:25–30`), producing 980 checks. This is systematic across solvers, arms, and classes, but it is not a check of all 51,240 outputs.

11. The lone verifier discrepancy—TS-N2/A0, `tai50_20_09`, run 4, reported 3119.0 versus recomputed 3118.5—is accurately disclosed. It should be traced before publication rather than left unexplained.

12. The paper calls the insertion SGS “active.” The implementation is clearly insertion-based, but gap feasibility uses component-wise dominance comparisons. It would help to define “active” under intervals precisely, since classical crisp active-schedule properties do not transfer automatically.

13. The manuscript alternates among “per-cell,” “per-instance,” and solver/class budgets. The implemented cap is per solver and size class, not per individual instance.

14. The CPU-occupancy caveat is sensible, but process CPU time does not make cache/memory-contention effects “constant” merely because worker count is fixed. Solver/arm memory-access patterns may differ.

## Discrepancies between paper and code

1. **Seed-block overlap.** The paper says “consecutive” \(k=250\) blocks overlap by about 22% (`main.tex:329–331`). The code sets block starts to \((r\cdot250)\bmod1024\) (`CreationIJSP.cpp:1441–1448`). Blocks \(r=0\) and \(r=1\) are 0–249 and 250–499 and do not overlap; likewise for the next two. Overlap occurs among other run pairs after wraparound. The intended statement is probably average pairwise reuse across all 30 blocks, not consecutive overlap.

2. **Calibration outside the evaluation set.** The paper says class-level calibration avoids calibration on the 61 evaluation instances (around `main.tex:449`). The calibration list in `phase1_budgets.sh` consists of final evaluation instances.

3. **Short-budget percentage.** Abstract/results: 37–53%; introduction: 28–44%. The anytime table supports 37–53%.

4. **Authoritative early-budget reference.** `early_budget.R:9–14` uses `refs.csv` and a different fraction grid. The manuscript’s table is consistent with `fig_anytime_all.R`, which uses `ta_lb.csv`. The repository contains two incompatible analyses under similar names.

5. **Chunk-size documentation.** `run_phase2c.sh` says stronger solvers use three-run chunks, but `chunk_size()` always defaults to five (`run_phase2c.sh:31`). `resample_anytime.sh:70` assumes five.

6. **“Verbatim” solver replication.** The paper claims verbatim published configurations, but ABCE3 does not reproduce the published performance and the promised replication table is absent. The repository verifies parameter settings, not exact implementation equivalence.

7. **Lamarckian behaviour needs solver qualification.** Evaluation-level Lamarckism re-encodes decoded schedules when enabled (`EvaluationIJSP.cpp:157–163`), and memetic local search separately writes the improved schedule back when `localsearch.lamarckism=yes` (`MemeticAlgorithm.cpp:601–623`). The shipped GA and TS-N2 setups enable these flags. Thus the core claim is supported, but the paper should distinguish evaluation re-encoding from local-search Lamarckism and state which solvers actually perform local search.

8. **MIX provenance wording.** The actual MIX construction uses the first 342/341/341 entries of V2/GT/GP, interleaved (`build_mix_pools.sh:18–26`). It is sourced from the same generator pools, but it is not true that every specific solution injected by a single arm is necessarily also injected by MIX during the 30 runs.

9. **Population size and injection position.** This claim is supported: the phase-2 setups use population 250; seeded arms request 125 or 250 seeds (`run_phase2c.sh:38–40`), and `CreationSeededSchedule::createPopulation` places seeds at indices \(0,\ldots,k-1\) and random individuals thereafter (`CreationIJSP.cpp:1436–1454`).

10. **Chunked versus monolithic seed assignment.** This claim is supported: `globalRun = seedOffset + runCounter` and `blockStart = globalRun*seedCount mod L` (`CreationIJSP.cpp:1441–1442`), while the launcher supplies chunk offsets (`run_phase2c.sh:81–86`). Scouts/partial populations remain random because seeding occurs only for a full-size population (`CreationIJSP.cpp:1431–1433`).

## What is genuinely good

- The central decoder semantics match the manuscript. The insertion SGS uses component-wise maximum for interval start times (`SGS_IJSP_Insertion.cpp:55–63,96,111`); makespan evaluation takes the configured maximum over job completion intervals (`EvaluationIJSP.cpp:114–116,155–157`); midpoint ranking is exactly \((a+b)/2\) (`Interval.cpp:342–345`).

- Lamarckian write-back is real, not merely asserted: improved schedules are encoded back into chromosomes in the memetic code (`MemeticAlgorithm.cpp:618–623`), and the relevant experimental setups enable it.

- The injection implementation is simple and auditable. It discards stored pool objectives, rebuilds individuals from permutations, re-decodes them using the receiving solver’s SGS, assigns deterministic global run blocks, and leaves scout replacements random.

- The principal final-quality statistics genuinely use instances, not 51,240 runs, as replicates. The reported ABCE3/MIX effect and confidence interval agree with the shipped aggregate output.

- The anytime extraction fixes two common but serious errors: it uses per-run traces rather than changing-composition averaged traces, and it computes a cumulative minimum before averaging. The 37–53% short-budget reductions follow arithmetically from the published table and are supported by the `fig_anytime_all.R` pipeline, excluding seed-generation cost.

- The crisp lower-bound argument is correct under the implemented component-wise interval arithmetic. This is a thoughtful and defensible choice once the instance mapping is verified exactly.

- The independent verifier is genuinely independent at the source level: it reimplements parsing, insertion decoding, interval arithmetic, and makespan calculation using only the standard library. Its 979/980 agreement is strong evidence that the reported schedules and interval objective are largely coherent.

Overall, this is a potentially valuable paper with an unusually commendable audit trail. The final-quality result is stronger than the survival and convergence analyses surrounding it; the revision should make that hierarchy of evidence explicit.