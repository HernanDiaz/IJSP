# Recommendation

**Major revision.**

The central empirical pattern is real: the endpoint files are complete and balanced, and most descriptive tables reproduce. However, the manuscript currently mixes six-, seven-, and eight-arm analyses; contains several false numerical statements; uses fragile or invalid inferential procedures for time-to-target, convergence, and diversity; and does not maintain a clean confirmatory/exploratory separation for MIXH. These problems affect claims in the abstract and conclusions, not merely presentation.

I independently parsed the four endpoint files: each contains 14,640 observations, covering 61 instances × 8 arms × 30 runs, for 58,560 runs total, with no missing cells. I also recomputed the endpoint RPDs, anytime table, TTT summaries, interaction estimates, convergence summaries, generator ranks, class effects, and pool-cost calculations. I did not read `review/` and did not modify files.

# Numerical discrepancies

1. **The design equation still says seven arms.**  
   Paper: “61 instances × 7 arms × 4 solvers × 30 runs (58,560)” at [main.tex:551](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:551>).  
   Correct: \(61\times8\times4\times30=58{,}560\). Seven arms would give 51,240. The raw files confirm eight arms and 58,560 runs.

2. **The claim that global multiplicity correction “changes nothing” is false.**  
   Paper: the “same 15 remain significant” at [main.tex:873](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:873>).  
   Recomputed from `efectos_ic.csv`:

   - Holm within each solver over seven seeded arms: **17** significant contrasts.
   - Holm over all 28 contrasts: **15**.
   - GA–V2 changes from adjusted \(p=0.041\) to \(0.0517\).
   - GA–MIXH changes from \(p=0.022\) to \(0.0886\).

   The current script itself identifies losses under global correction, despite its stale “24 contrastes” comment at [stats_robusto.R:59](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/stats_robusto.R:59>).

3. **The W/NS/L table is a hybrid of two multiplicity families.**  
   The original arms retain counts calculated with six seeded arms, while MIXH and the reported adjusted \(p\)-values use seven. Examples:

   - GA–V2H: paper 11/31/19; current seven-arm recomputation **11/32/18**.
   - ABCE3–MOR: paper 1/54/6; recomputed **1/57/3**.
   - fEABCLS–MIX: paper 12/38/11; recomputed **12/39/10**.
   - TSN2–V2: paper 6/47/8; recomputed **6/49/6**.
   - TSN2–MIX: paper 15/41/5; recomputed **14/43/4**.

   The comments still say six arms at [phase2_stats.R:2](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2_stats.R:2>) and [phase2_stats.R:23](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2_stats.R:23>), although the code now contains seven.

4. **The abstract’s full-budget summary is under-specified and changes with the chosen family.**  
   Paper: “0.1% to 1.0% … significant on one solver” at [main.tex:63](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:63>).  
   Using the best arm, including exploratory MIXH, the reductions are approximately GA 0.253%, ABCE3 1.018%, fEABCLS 0.090%, TSN2 0.224%. Under per-solver Holm, both GA–MIXH and ABCE3 contrasts are significant; “one solver” holds only under the global 28-test correction. For the pre-specified MIX arm, GA is slightly harmed and fEABCLS is essentially unchanged. The abstract must name the arm, correction family, and exploratory status.

   The tenth-budget values are reproducible: **1.30%, 1.82%, 5.17%, and 4.07%** for GA, ABCE3, fEABCLS, and TSN2.

5. **Two TTT \(p\)-values do not match the released analysis.**

   - TSN2–V2: paper **0.62** at [main.tex:1186](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1186>); recomputed Holm value **0.4724**.
   - TSN2–MIX: text **0.003** at [main.tex:1239](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1239>); recomputed Holm value **0.000976**.

6. **MOR does sometimes reach the target on GA and fEABCLS.**  
   Paper: “no instance at all” at [main.tex:1261](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1261>).  
   Recomputed:

   - GA–MOR: **3/61 instances, 4.9%**.
   - fEABCLS–MOR: **7/61, 11.5%**.

   Consequently, the following claim that GA’s success-only median is based on 15% of instances is also false: it is based on **4.9%**, not 15%, at [main.tex:1270](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1270>).

7. **Two confidence intervals are misreported.**

   - ABCE3–MIX: source \([-1.3619,-0.9246]\), conventionally \([-1.36,-0.92]\); text says \([-1.36,-0.93]\) at [main.tex:871](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:871>).
   - TSN2–MIX: source \([-0.3444,-0.0396]\), conventionally \([-0.34,-0.04]\); text says \([-0.35,-0.045]\) at [main.tex:887](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:887>).

8. **The 71-instance pool-quality ladder is not reproducible from the released pool-quality data.**  
   Paper values: MOR 51.1%, GT 30.3%, GP 21.4%, V2 21.2% at [main.tex:324](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:324>).  
   `pool_quality.csv` contains 61 instances, and its generating script explicitly evaluates only the first 250 entries at [pool_quality.sh:3](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/pool_quality.sh:3>) and [pool_quality.sh:20](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/pool_quality.sh:20>). From that released file I obtain:

   - MOR **50.60%**
   - GT **29.98%**
   - GP **20.99%**
   - V2 **19.97%**

   I could not locate the 71-instance source underlying the paper values or independently verify the accompanying 0.95/0.92 diversity figures.

9. **Seed-cost ratios are overstated.**  
   Paper: learned generation is “six to ten times” the rules at [main.tex:375](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:375>). The tabulated timings imply approximately **2.72–6.90×**, depending on class and which rule is used.  
   The claim that 250 seeds cost 4–51 times the tenth-budget allowance is also not true across all solver/class combinations: using the cheaper rule generator, I obtain approximately **1.03–51.07×**. The amortized whole-pool range of roughly 0.1–7× is defensible.

10. **The independent-verifier count is stale.**  
    Paper: **980 solutions** at [main.tex:1563](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1563>). That equals \(4\times7\times7\times5\), i.e. the old seven-arm design. The current verifier script lists eight arms at [verify_broad_indep.sh:11](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/verify_broad_indep.sh:11>), which would require **1,120** solutions. The available detail file records the one known disagreement but does not contain a complete eight-arm summary.

# Major issues threatening the conclusions

1. **The statistical tables are not a coherent eight-arm analysis.**

   **Claim:** The experimental unit is the instance and all seven seeded arms receive a common within-family Holm correction ([main.tex:807](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:807>), [main.tex:1228](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1228>)).

   **Evidence:** The W/NS/L counts retain six-arm corrections, adjusted endpoint \(p\)-values use seven arms, and the global-correction statement is wrong. Moreover, \(\hat A_{12}\) is calculated from all 30×30 run comparisons within an instance and then averaged across instances ([phase2_stats.R:10](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2_stats.R:10>), [phase2_stats.R:32](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2_stats.R:32>)), not from 61 independent instance summaries.

   **Why it matters:** The table combines incompatible inferential families, and the stated “instance as unit” interpretation does not describe all reported effect sizes. Shared pool blocks also make run-level comparisons less independent than the table suggests.

   **Fix:** Regenerate the entire table in one pass from a frozen eight-arm specification. Clearly separate (i) instance-level endpoint contrasts and effect sizes from (ii) descriptive within-instance run distributions. State both per-solver and global multiplicity results, or pre-specify one primary family.

2. **The calibrated TTT sign test is not adequately calibrated.**

   **Evidence:** The null simulation uses only 400 replicates ([ttt_null.R:24](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_null.R:24>)) and splits the 30 control runs into two groups of 15 ([ttt_null.R:67](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_null.R:67>)), whereas the actual contrast compares 30-run curves. Its Type-I calculation tests against 0.5 ([ttt_null.R:80](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_null.R:80>)); it does not validate the subsequent test against the estimated pooled null probability used at [ttt_paired.R:85](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:85>) and [ttt_paired.R:124](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:124>).

   **Why it matters:** A pooled binomial model assumes the same null “earlier” probability for every instance and treats an estimated probability as fixed. Heterogeneous targets and curve variability violate this simple binomial model. Borderline findings, especially TSN2–MOR and exploratory interaction-related claims, may not survive a matched null.

   **Fix:** Use an empirical randomization/null distribution that reproduces the actual 30-versus-30 curve construction and target selection, preferably instance-specific or Poisson-binomial/hierarchical. Increase Monte Carlo replication substantially and propagate calibration uncertainty. Until then, treat TTT \(p\)-values as sensitivity evidence.

3. **The TSN2 convergence classification is contaminated by the objective mismatch.**

   **Evidence:** The rule computes \(100(v_{90}-v_{\mathrm{final}})/v_{\mathrm{final}}\) and accepts values below the threshold ([convergence.R:48](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:48>), [convergence.R:53](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:53>)). For TSN2, 325/1,830 control runs have negative “improvement” because the trace incumbent and returned midpoint solution use different rankings. Every negative value automatically satisfies the convergence rule.

   The main intersection analysis correctly uses both-arm intersections at [convergence.R:113](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:113>), but the sensitivity loop reverts to control-converged instances at [convergence.R:130](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:130>)–[133](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:133>).

   **Why it matters:** The conclusion that the TSN2 benefit disappears once convergence is reached rests on a classifier that can label inconsistent traces as converged, and the claimed sensitivity of the intersection result was not actually calculated on intersections.

   **Fix:** Regenerate TSN2 anytime traces using the same objective/ranking as the returned endpoint. Rerun both the primary and sensitivity analyses with the both-arm intersection at every threshold. Do not retain the current TSN2 convergence conclusion as inferential evidence.

4. **MIXH is described as separate exploration but used in confirmatory selection.**

   **Evidence:** The chronology is honestly disclosed at [main.tex:475](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:475>) and [main.tex:1276](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1276>). Nevertheless, MIXH appears in earlier primary endpoint, convergence, anytime, W/NS/L, and rank tables; participates in bold “best arm” selection; determines parts of the abstract; and drives the practical recommendation at [main.tex:1708](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1708>).

   **Why it matters:** Adding MIXH after inspecting the original seven and then allowing it to redefine the primary family and best-arm summaries is not equivalent to “reported separately.” It also explains several stale six-arm counts.

   **Fix:** Freeze the original seven-arm experiment as the confirmatory analysis. Put MIXH in clearly marked exploratory columns/panels or a separate table; do not let it determine bold confirmatory winners. Rewrite the abstract and conclusions so the confirmatory MIX result and exploratory MIXH follow-up are numerically distinct.

5. **The proposed diversity mechanism is not supported by the reported test.**

   **Claim:** V2 and MIX remain “significantly more diverse” and provide evidence that composition, not seed quality, is operative ([main.tex:1626](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1626>)–[1630](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1630>), [main.tex:1383](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1383>)).

   **Evidence:** Extraction takes the already run-averaged “Average Evolution” trace ([extract_diversity.sh:5](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/extract_diversity.sh:5>)). The test then merges repeated timepoints and applies a paired Wilcoxon test over those timepoints ([diversity_struct.R:23](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/diversity_struct.R:23>)–[26](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/diversity_struct.R:26>)). Thus serially autocorrelated observations from only four instances are treated as replicates after the ten runs have already been averaged.

   **Why it matters:** The \(p<10^{-4}\) result is pseudoreplicated. The five selected seed-quality examples are also insufficient to identify composition causally. The descriptive GA differences—+24.3% for V2 and +13.7% for MIX—do reproduce, but they do not establish mechanism.

   **Fix:** Analyse run-level summaries with instance blocking, or report the four-instance experiment descriptively. Replace “direct evidence” and “operative variable” with a clearly labelled mechanistic hypothesis.

6. **The 2×2 interaction is computed correctly but interpreted too strongly.**

   **Evidence:** I reproduce the interaction estimates:

   - GA \(+0.056\), \(p=.482\)
   - ABCE3 \(-0.050\), \(p=.278\)
   - fEABCLS \(-0.084\), \(p=.008\)
   - TSN2 \(-0.074\), \(p=.046\)

   These are four post hoc interaction tests with no multiplicity correction. Holm over four gives approximately **0.032 for fEABCLS and 0.138 for TSN2**.

   **Why it matters:** The statement about interaction on “two other solvers” at [main.tex:1308](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1308>) relies on an uncorrected borderline TSN2 result. Failure to reject on GA/ABCE3 also does not demonstrate additivity.

   **Fix:** Label all four interaction tests exploratory, report corrected and uncorrected values, and restrict the positive evidence to fEABCLS. Describe GA/ABCE3 as inconclusive.

7. **The practical recommendation is not cost-accounted consistently.**

   **Evidence:** The manuscript recommends three-generator MIX partly because it “costs no more than” one generator ([main.tex:1704](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1704>)), while the measured learned generator is materially slower and the strongest final-budget effects are small or exploratory.

   **Why it matters:** “Same pool size” is not the same as “same generation cost.” The answer depends on whether component pools already exist, whether only 1,024 total candidates are generated, whether policy training is amortized, and how often the pool is reused. For a single solve, seed creation can dominate the early optimization budget.

   **Fix:** Give explicit deployment scenarios: pre-existing pool, newly generated mixed pool, and training included/excluded. Base the primary recommendation on the pre-specified MIX result, with MIXH as a candidate for confirmation.

# Major issues requiring better exposition or reproducibility

8. **The independent verification claim must be rerun for all eight arms.**

   The 980 count demonstrates that the archived verification predates MIXH. Rerun the current eight-arm script, archive the complete 1,120-case summary and discrepancy log, and state exactly which objective transformation the verifier checks.

9. **The time metric is described incorrectly.**

   The manuscript says both TTT quantities are medians at [main.tex:1148](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1148>), but Metric B is a mean of \(\min(T,1)\), i.e. a restricted mean time. Correct the definition and table caption.

10. **The anytime x-axis is not strictly the declared nominal budget fraction.**

    `fig_anytime_all.R` divides by each instance-arm trace’s own observed maximum time at [fig_anytime_all.R:32](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_anytime_all.R:32>). This guarantees every arm reaches its own 100%, even when observed endpoints differ. Deviations are small, but “fraction of the per-instance budget” should use the nominal common budget or a shared control horizon.

# Minor issues

- The source header still describes seven arms at [main.tex:5](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:5>).

- Several nominal audit/figure scripts retain seven-arm lists, notably [audit_paper_numbers.R:6](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/audit_paper_numbers.R:6>), [fig_tables.R:21](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_tables.R:21>), [ttt_phase2.R:21](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_phase2.R:21>), and [audit_sols.sh:17](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/audit_sols.sh:17>). `run_phase2c.sh` defaulting to seven is understandable because MIXH was launched separately, but this must be documented.

- `stats_robusto.R` says “24 contrasts” while executing 28; `phase2_stats.R` says six seeded arms while executing seven.

- `ft10` is a 10×10 instance but receives the 15×15 budget mapping. The design text says size classes range from 15×15 upward while later treating 10×10 as one of seven classes. State the mapping explicitly.

- The manuscript contains unresolved TODOs for the replication table and FTOP result at [main.tex:294](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:294>) and [main.tex:483](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:483>), plus a pending DOI at [main.tex:1853](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1853>). It is not publication-ready.

- I could not verify the policy-training cost or the claimed 71-instance pool statistics from the released analysis products. These must either be released with exact derivation scripts or removed.

# What is genuinely good

- The four endpoint files are complete, exactly balanced, and genuinely contain all 58,560 claimed runs.

- The complete final-RPD table reproduces to rounding from the raw endpoints. The entire anytime table also reproduces to rounding from the raw trace files.

- The reported 133/244 control-converged count and solver breakdown—12 GA, 46 ABCE3, 36 fEABCLS, 39 TSN2—are correct.

- The 2×2 interaction point estimates and intervals reproduce; the defect is multiplicity and interpretation, not calculation.

- Generator mean ranks and the reported mean cross-solver rank correlation of about 0.72 reproduce.

- The class-specific TSN2 result is accurately localized: the aggregate MIX advantage is overwhelmingly concentrated in the 50×20 class. The manuscript does not hide that limitation.

- Six instances reaching the lower bound and the absence of below-bound solutions both reproduce.

- The pool block-offset implementation is sound: blocks depend on run number, consecutive blocks are disjoint when possible, all 30 starts are distinct, and the reported approximately 21.9% average pairwise overlap for 250-seed blocks is correct ([CreationIJSP.cpp:1442](</C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/CreationIJSP.cpp:1442>)).

- The paper is unusually candid about MIXH chronology, objective-rank mismatch, convergence sensitivity, and blocked learned pools. That candour makes the study salvageable—but the statistical presentation now needs to follow those disclosures consistently.