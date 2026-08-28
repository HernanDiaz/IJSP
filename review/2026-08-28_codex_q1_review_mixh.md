## Recommendation

**Major revision, bordering on reject-and-resubmit.** The experimental corpus is unusually large and several core results are reproducible, but the manuscript is not internally consistent with its final eight-arm design. More seriously, MIXH—the arm supporting the headline “separable and additive mechanisms” claim—was designed after inspecting the original outcomes, its factorial interpretation is confounded by the V2 checkpoint blocks, and several analyses still exclude it. The time-to-target calibration and diversity inference also do not support the strength of the claims made.

Because the workspace was read-only, I did not rerun R scripts that overwrite released outputs. I checked their logic, cross-checked their emitted CSVs against the manuscript, counted raw rows, and independently recomputed deterministic arithmetic. I explicitly identify results for which the supplied pipeline cannot produce the claimed final-design analysis.

## Numerical discrepancies

| Item | Manuscript | Recomputed/released result | Source |
|---|---:|---:|---|
| Factorial design | \(61\times7\times4\times30=58{,}560\) | Seven arms give **51,240**; 58,560 requires **eight** arms. Each solver CSV has 14,640 rows, including 1,830 MIXH rows; MIXH contributes 7,320 runs. | [main.tex:546](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:546>), [results_ga.csv](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2/results_ga.csv:1>) |
| Declared design | “Seven arms” | Final data have A0 plus seven seeded arms, i.e. **eight**. MIXH is absent from the design description. | [main.tex:471](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:471>), [run_phase2c.sh:38](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/run_phase2c.sh:38>) |
| GA–V2 Holm \(p\) | 0.012 | **0.015892** under the seven-seeded-arm family; it should round to 0.016. | [main.tex:826](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:826>), [efectos_ic.csv:3](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/efectos_ic.csv:3>) |
| Global multiplicity family | 24 contrasts | **28 contrasts** \(4\times7\). The released global Holm calculation still gives 15 significant contrasts, but the manuscript describes the wrong family. | [main.tex:867](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:867>), [stats_robusto.R:59](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/stats_robusto.R:59>) |
| Friedman ranking | Caption says eight arms | Table and script contain only **seven total arms**, omitting MIXH. Every displayed rank row sums to 28, as seven ranks must; eight ranks would sum to 36. | [main.tex:1363](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1363>), [generator_ranking.R:8](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/generator_ranking.R:8>) |
| Best seeded strategy | MIX best rank on all four and best mean on three | The current mean-RPD table itself shows **MIXH best on GA, fEABCLS and TSN2**, and MIX best only on ABCE3. The eight-arm Friedman rankings are not supplied or reproducible using the released ranking script. | [main.tex:791](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:791>), [main.tex:740](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:740>) |
| Anytime analysis | Presented as the final study; abstract reports best-arm 10% gains | Figure and table scripts omit MIXH and retain A0 plus the six old seeded arms. The reported 1.3–5.2 percentage-point figures reproduce for MIX, but **not as an all-eight-arm analysis**. | [main.tex:63](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:63>), [fig_anytime_all.R:10](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_anytime_all.R:10>) |
| Anytime figure caption | “control and the two best seeded arms” | Plotting script draws **all seven old arms**, not two, and no MIXH. | [main.tex:973](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:973>), [fig_anytime_all.R:41](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_anytime_all.R:41>) |
| TSN2–V2 time-to-target \(p\) | 0.62 | Current calibrated Holm \(p=\)**0.472416**. | [main.tex:1177](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1177>), [ttt_paired.csv:24](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.csv:24>) |
| TSN2–MIX time-to-target \(p\) | Prose: 0.003 | Current calibrated Holm \(p=\)**0.000976**, correctly rounded to 0.001 in the table. | [main.tex:1230](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1230>), [ttt_paired.csv:28](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.csv:28>) |
| GA seeded-arm count | “Five of six seeded arms” | There are now **seven** seeded arms; five are slower, MIX is nonsignificant, and MIXH is faster by the calibrated sign test. | [main.tex:1233](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1233>), [ttt_paired.csv:2](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.csv:2>) |
| Learned-policy cost | Six to ten times rule cost per solution | From the supplied figures, the calculable range is **2.72–6.90×**: at \(50\times20\), 3.98 s is 3.25× GP and 2.72× GT. | [main.tex:374](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:374>), [gen_cost.R:5](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/gen_cost.R:5>) |
| One-run seed cost / 10% budget | Four to 51 times | Across all supplied solver/class cells using GP, the range is **1.03–51.07×**. The minimum is \(50.05/48.6=1.03\) for TSN2 at \(20\times20\). | [main.tex:379](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:379>), [gen_cost.R:17](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/gen_cost.R:17>) |
| Amortized cost | 0.1–7 times the 10% slice | **Reproduced:** approximately 0.141–6.972×; below one in 15 of 24 cells. | [gen_cost.R:25](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/gen_cost.R:25>) |
| Pool-quality universe | 71 instances | Released script enumerates **61 instances**, and `pool_quality.csv` has 305 rows \(61\times5\). The manuscript also says no pools were made for excluded groups. | [main.tex:323](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:323>), [pool_quality.sh:12](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/pool_quality.sh:12>) |
| Independent verification | Every solver, arm and class; 980 solutions | \(4\times7\times7\times5=980\): this is the **old seven-arm** design. MIXH is explicitly absent from both verifier drivers. Full equivalent coverage would require 1,120 solutions. | [main.tex:1525](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1525>), [verify_broad_indep.sh:11](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/verify_broad_indep.sh:11>) |
| Midpoint/routing audit | 57,450 operations presented as the study instances | The 61 evaluated instances contain **34,350 operations**. The 57,450 figure belongs to an 82-instance external collection, according to the project note; only 61 recovered crisp matrices are released. | [main.tex:556](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:556>), [LOWER_BOUNDS.md:42](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/LOWER_BOUNDS.md:42>) |

The main final-RPD table agrees with the current `efectos_ic.csv`, as do its effect estimates and intervals apart from the GA–V2 adjusted \(p\). The representative convergence-intersection table agrees with `convergencia_interseccion.csv`. Most time-to-target table entries agree with `ttt_paired.csv`. The seed-quality and Hamming descriptive values agree with their scripts, although the inference drawn from the latter is invalid for the reason below.

## Major issues threatening the conclusions

### 1. MIXH is an outcome-adaptive, undisclosed extension

**Claim.** The manuscript presents a pre-specified design and elevates MIXH to the abstract-level conclusion that composition and fraction are separable and additive.

**Evidence.** The original study ended on 23 August; MIXH was launched on 26 August and completed on 28 August ([run.log:7](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2/run.log:7>), [mixh_run.log:15](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/mixh_run.log:15>)). More importantly, the MIXH script explicitly motivates the arm using the already-observed GA effects—V2H better than V2 and MIX not harmful—and states the expected interpretation if MIXH improves ([run_mixh.sh:4](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/run_mixh.sh:4>)). The manuscript source dated 24 August still described the completed seven-arm experiment ([main.tex:4](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:4>)).

**Why it matters.** Holm correction over seven seeded arms handles simultaneous testing within the final table; it does not undo adaptive hypothesis and arm selection based on the first-stage outcome. MIXH is a useful follow-up experiment, but its evidence is exploratory, not confirmation of a pre-specified mechanism.

**Fix.** Disclose the chronology and label MIXH and Section 7.9 exploratory. Either tone the abstract down to “suggests that the two factors may combine approximately additively” or perform an independently pre-specified replication.

### 2. The final eight-arm analysis pipeline is incomplete

**Claim.** Rankings, anytime behaviour, seed-quality evidence and solution verification are described as covering the final experiment.

**Evidence.** Ranking scripts omit MIXH ([generator_ranking.R:8](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/generator_ranking.R:8>)); both anytime scripts omit it ([fig_anytime_paper.R:12](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_anytime_paper.R:12>)); the initial-quality script omits it ([seed_quality.sh:13](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/seed_quality.sh:13>)); and setup, solution and independent-verifier audits retain the old arm list ([audit_setups.sh:7](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/audit_setups.sh:7>), [verify_broad.sh:9](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/verify_broad.sh:9>)). In contrast, the primary effect, convergence and time-to-target scripts do include MIXH.

**Why it matters.** The omitted analyses are precisely those used to recommend a pool, quantify short-budget performance, rule out seed quality, and establish implementation correctness. The recommendation “default to a mixed half population” is therefore not supported by the displayed anytime or seed-quality analyses.

**Fix.** Recompute every arm-dependent figure, table, ranking, post-hoc family and audit with eight arms. Add MIXH to the initial-population-quality analysis and re-run at least the independent verifier for its 140 systematic cases.

### 3. The evidence is compatible with additivity but does not establish it

**Claim.** “Two mechanisms are separable and additive.”

**Evidence.** For GA, the four mean effects relative to A0 are V2 \(+0.528\), V2H \(+0.141\), MIX \(+0.055\), and MIXH \(-0.276\) points. Thus:

- Fraction effect within V2: \(0.141-0.528=-0.387\).
- Fraction effect within MIX: \(-0.276-0.055=-0.331\).
- Difference-in-differences interaction: \(+0.056\) points.

That is descriptively close to an additive model. But [mixh_vs_mix.R:15](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/mixh_vs_mix.R:15>) runs only three simple MIXH contrasts; it neither estimates nor tests the interaction. The statement in the script that improvement over MIX means the effects “sum” is not logically sufficient ([mixh_vs_mix.R:6](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/mixh_vs_mix.R:6>)).

**Why it matters.** A combination outperforming each component is compatible with additivity, synergy, a threshold effect, or two measurements of one underlying process. Calling the factors “mechanisms” further exceeds what an initialization-arm experiment identifies.

**Fix.** Analyze the complete \(2\times2\) contrast at the per-instance level and report the interaction estimate and interval. Use “factors” or “design levers” unless a mechanism-specific intervention is performed. Correct or explicitly scope the 12 unadjusted direct MIXH contrasts.

### 4. V2H versus V2 is confounded by checkpoint composition

**Claim.** V2H versus V2 isolates seeded fraction, with V2 blocking merely inflating run-to-run variance.

**Evidence.** The implementation advances by `run × seedCount` ([CreationIJSP.cpp:1441](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/CreationIJSP.cpp:1441>)). Applying that rule to blocks of 342/341/341 gives:

- V2: 2,726/2,387/2,387 seeds across checkpoints = **36.35/31.83/31.83%**.
- V2H: 1,368/1,359/1,023 = **36.48/36.24/27.28%**.

Thus changing 250 to 125 seeds also reduces checkpoint 3 by 4.55 percentage points and increases checkpoint 2 by 4.41 points. Twenty V2H windows fall wholly within one checkpoint, as the manuscript states. MIX and MIXH additionally use only the first 342 V2 pool entries—checkpoint 1—for their V2 component ([build_mix_pools.sh:23](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/build_mix_pools.sh:23>)).

**Why it matters.** Since the checkpoints differ in quality, V2H–V2 is not a pure fraction contrast, and MIX–V2 or MIXH–V2H is not a pure generator-composition contrast. This is more than increased variance: it can shift the mean contrast and directly compromises the proposed factorial interpretation.

**Fix.** Interleave checkpoints before windowing, regenerate balanced V2/V2H runs, or estimate checkpoint-specific effects from labeled seeds. At minimum, report both mixtures and explicitly call the contrasts confounded. The current limitation at [main.tex:1722](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1722>) does not draw this consequence.

The block-quality claim itself is only partly verifiable: `pool_blocks.R` assumes the three labels from position and tests four instances ([pool_blocks.R:11](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/pool_blocks.R:11>)); no output file or checkpoint provenance metadata is released.

### 5. The time-to-target null simulation does not validate the test actually used

**Claim.** Split-half calibration shows that using solver-specific \(p_0\) values removes the endogenous-target tilt.

**Evidence.**

- The simulation splits 30 control runs into 15+15 ([ttt_null.R:14](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_null.R:14>)), whereas the real comparison uses two 30-run averaged curves.
- Reported type-I rates are calculated with `binom.test` at its default \(p=0.5\) ([ttt_null.R:80](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_null.R:80>)).
- The actual analysis instead substitutes an estimated pooled \(p_0\) ([ttt_paired.R:85](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:85>), [ttt_paired.R:124](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:124>)).
- \(p_0\) pools all instance signs over 400 replicates, ignoring instance-specific probabilities, tie rates and uncertainty in the estimate ([ttt_null.R:76](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_null.R:76>)).

**Why it matters.** The reported 3.3–6.8% rejection rates show that the ordinary \(p=0.5\) test is not grossly miscalibrated under the 15+15 simulation. They do not demonstrate the type-I error of the plug-in \(p_0\) binomial test used in the paper. A heterogeneous sum of signs is generally Poisson-binomial, not Binomial\((n,p_0)\). This matters visibly for GA–MIX.

**Fix.** Use the resampling distribution of the total sign statistic directly, preferably with 30-run-equivalent bootstrap groups and many more replicates. Account for ties and \(p_0\) estimation, or use a symmetrized/external target. Until then, present the calibrated sign results as a sensitivity analysis, not a validated primary test.

Also, the target is the final **sampled trace endpoint**, not necessarily the returned final quality ([ttt_paired.R:94](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:94>)); the caption should say so.

### 6. Seed-cost conclusions are stronger than the evidence

**Claim.** The short-budget conclusion “holds” if a pool is reused and “does not hold” for a one-off solve.

**Evidence.** Only GP and GT pool times are direct. V2 is extrapolated from three supplied rollout timings despite the comment saying four classes, and MOR is untimed ([gen_cost.R:2](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/gen_cost.R:2>)). The amortized cost remains as high as seven times the 10% search slice. No end-to-end quality-versus-time curve includes generation cost, and the recommended MIXH pool cost is not calculated.

**Why it matters.** Economic amortization over 30 solves is not the same as latency: the first solve still waits for pool generation. A cost smaller than the 10% slice in “most cells” does not prove that every reported 10%-budget advantage survives. Conversely, cost exceeding the slice does not prove that a one-off solve is inferior without comparing total time to matched quality.

**Fix.** Report end-to-end curves with generation time prepended, separately for latency and amortized throughput. Cost MIX and MIXH explicitly, specify hardware, and state that reuse must be on the same instance because pools are instance-specific. Replace the categorical conclusion with a conditional one.

### 7. The diversity significance test is pseudoreplicated

**Claim.** The seeded GA is significantly more diverse, \(p<10^{-4}\), supporting “consolidation failure” rather than premature convergence.

**Evidence.** The experiment has four instances and ten runs, but extraction produces one run-averaged “Average Evolution” curve per instance/arm ([extract_diversity.sh:5](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/extract_diversity.sh:5>)). The test merges all matched time points and treats them as paired observations ([diversity_struct.R:23](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/diversity_struct.R:23>), [diversity_struct.R:26](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/diversity_struct.R:26>)). Adjacent points from the same four trajectories are not independent experimental units.

**Why it matters.** The \(p<10^{-4}\) value is invalid. The descriptive divergence is interesting, but four instances cannot sustain the inferential strength used in the abstract. Moreover, MIXH—the arm that benefits GA—is absent, so these data explain V2/MIX behaviour rather than the successful combination.

**Fix.** Retain run-level trajectories, define an instance-level summary such as area under the diversity curve or endpoint change, and bootstrap or permute by instance. Add MIXH. Keep the JOX explanation explicitly hypothetical unless crossover is experimentally varied.

The related assertion that composition, “not seed quality,” remains the operative explanation is also too categorical. MIX has the best observed initial individual by a small margin, and selection responds to tails as well as population means. A quality-matched composition/diversity ablation is required for exclusion of quality.

### 8. “Run to convergence” is not what was executed

The plateau table conditions either on control plateau or on both-arm plateau; it does not run every arm to actual convergence. Conditioning on both control and treatment plateau also selects on a treatment-affected variable and yields different instance subsets for different arms ([convergence.R:103](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:103>)). This supports a useful plateau-conditioned sensitivity analysis, not the abstract’s categorical “run to convergence” result. Use that terminology or run longer experiments with a common, externally justified horizon.

## Major issues of exposition and reproducibility

1. The 57,450-operation audit depends on external default directories outside the release ([check_midpoint.sh:9](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/check_midpoint.sh:9>)). Release the exact 82-file input set and output, or restrict the claim to the 61 supplied matrices.

2. Report the block-quality test output and checkpoint identities. At present, “three checkpoints” is provenance asserted by position, not recoverable from metadata.

3. Reconcile every occurrence of seven/eight arms, six/seven seeded arms, 24/28 contrasts, and 51,240/58,560 runs. The current mixture of versions makes it impossible to know which sections belong to which experiment.

## Minor issues

- Remove visible TODOs concerning seed times, replication table, FTOP, references and DOI ([main.tex:6](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:6>)).
- The data-availability section still says `[DOI pending]` and contains a malformed `\ref` ([main.tex:1813](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1813>)).
- `ttt_paired.R` comments and printed text still say Holm over six arms although the computation uses seven ([ttt_paired.R:44](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:44>)).
- `gen_cost.R` says rollout timings exist for four classes but supplies three ([gen_cost.R:7](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/gen_cost.R:7>)).
- “Every single-generator pool significantly harms GA” is false after global 28-contrast correction: GA–V2 has global \(p=0.0516\), although it is significant within solver.
- “Paired tests are conservative” is asserted without establishing conservatism; unrelated RNG trajectories preserve validity under the relevant null but do not guarantee conservative size.
- “Largest controlled study” requires a documented literature comparison or should be removed.
- Replace “i.i.d. pools” with a more precise description unless independence of generator outputs has actually been checked.

## What is genuinely good

- The final raw result files are complete and internally coherent: eight arms, 61 instances, 30 runs, four solvers, totaling 58,560.
- The primary analysis correctly uses the instance—not the run—as the experimental unit, and the current `stats_robusto.R` genuinely applies Holm over seven seeded arms.
- The MIXH implementation itself is sound: it uses the same mixed pool as MIX, changes only 250 to 125 seeds, and preserves global run offsets ([run_phase2c.sh:38](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/run_phase2c.sh:38>), [CreationIJSP.cpp:1437](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/CreationIJSP.cpp:1437>)).
- The interval arithmetic and decoder/evaluator convention are consistent in code, and the independent verifier genuinely reimplements componentwise arithmetic rather than calling framework classes ([Interval.cpp:280](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/Interval.cpp:280>), [verify_independent.cpp:40](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/verify_independent.cpp:40>)).
- The manuscript is unusually candid about TSN2 concentration, trace-versus-returned-quality differences, tight budgets and underdetermination of the JOX explanation.
- The fixed-budget design, complete cell reporting, convergence sensitivity analysis, and release of raw traces are substantially stronger than typical seeding studies. These strengths are worth preserving; the problem is that the newest claims outran the otherwise careful design.