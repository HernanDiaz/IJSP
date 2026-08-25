# Referee report

## Recommendation — major revision

The principal final-quality results are reproducible, and the study has real value. However, Table 8 implements a different target from the one stated in the manuscript; correcting it materially changes reach rates, restricted mean times, signs, and several conclusions. In addition, “converged quality” is inferred from convergence of the control alone, the fEABC-LS implementation does not match its tabulated semantics, the lower-bound mapping is only partially verified, and the independent verifier is weaker than claimed.

These are repairable, but some require reanalysis and possibly rerunning fEABC-LS. I would not recommend rejection because the central ABCE3 final-quality result, the neutral fEABC-LS endpoint result, and the budget-dependent descriptive patterns survive my independent recomputation.

## Summary of the contribution

The manuscript studies whether learned and heuristic solution pools improve four IJSP metaheuristics under fixed CPU budgets. It compares an unseeded control, four single-generator pools, a mixed pool, and a half-seeded contrast over 61 instances and 30 runs per cell. Its strongest contribution is methodological: it exposes how stopping rules, budget selection, and self-referential targets can change the apparent value of seeding, and reports full anytime traces rather than only endpoints.

The headline empirical result is heterogeneous: MIX substantially improves ABCE3, is neutral for fEABC-LS, is statistically indistinguishable from the GA control at the endpoint, and gives a small, large-instance-specific TS-N2 improvement that disappears on the selected control-converged subset.

## Numerical discrepancies

### Figures successfully reproduced

Using `results_*.csv`, `anytime_*.csv`, `conv_runs_*.csv`, and `ta_lb.csv` directly:

- Table 4 is reproduced exactly to its displayed precision. Control-converged counts are GA 12, ABCE3 46, fEABC-LS 36, TS-N2 39; anytime-only counts are respectively 49, 15, 25, and 22. All 56 displayed RPD means match. Sources: [convergence.R:45](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:45>), [convergencia.csv](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergencia.csv>), [Table 4:567](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:567>).

- Every displayed value in Table 5 matches the current repaired `anytime_*.csv` files to two decimals. At 10% budget, the MIX reductions in excess over the LB are 10.9% GA, 16.7% ABCE3, 40.0% fEABC-LS, and 36.8% TS-N2. Sources: [fig_anytime_all.R:31](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_anytime_all.R:31>), [Table 5:941](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:941>).

- All 24 per-instance mean effects in the effect-size table match `efectos_ic.csv`; the reported bootstrap intervals also match that file. In particular, ABCE3/MIX is −1.14014 pp, CI [−1.36112, −0.92994]. Source: [efectos_ic.csv](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/efectos_ic.csv>), [stats_robusto.R:36](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/stats_robusto.R:36>), [main.tex:801](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:801>).

- The experiment size is correct: \(61\times7\times4\times30=51{,}240\), and each `results_*.csv` has 12,810 observations.

- The abstract/introduction/conclusion figures of 35 ABCE3/MIX wins and zero losses, the −1.14 pp effect, its CI, and the 11%–40% short-budget reductions are numerically supported.

### Table 8: paper values reproduce the code, but the code implements the wrong target

The manuscript states “control final quality \(+0.5\%\)” [main.tex:1091](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1091>). The repaired script instead converts values to RPD and tests

\[
\mathrm{RPD}_{s}\le 1.005\,\mathrm{RPD}_{A0},
\]

at [ttt_paired.R:92](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:92>). The stated makespan target requires

\[
C_s\le1.005C_{A0}
\iff
\mathrm{RPD}_{s}\le1.005\,\mathrm{RPD}_{A0}+0.5.
\]

The older script correctly applies the multiplier to raw `bestcmax` at [ttt_phase2.R:45](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_phase2.R:45>).

Recomputing from the current anytime CSVs with the stated raw-makespan target gives:

| Solver/arm | Paper: reach; earlier/later; RMTT; ΔRMTT | Correct target | Correct Holm sign \(p\) |
|---|---|---|---|
| GA/V2 | 38%; 20/32; .86; −.08 | 56%; 6/49; .77; **+.16** | \(<10^{-8}\), slower |
| GA/MOR | 2%; 1/48; .99; +.05 | 5%; 0/59; .99; **+.37** | \(<10^{-16}\), slower |
| GA/MIX | 46%; 24/28; .83; −.12 | 77%; 18/37; .70; **+.09** | .014, slower |
| ABCE3/V2 | 90%; 54/4; .29; −.38 | 100%; 54/2; .16; −.22 | \(<10^{-12}\), faster |
| ABCE3/MOR | 38%; 19/37; .81; +.14 | 70%; 13/40; .60; +.22 | .00054, slower |
| ABCE3/MIX | 95%; 57/3; .20; −.47 | 100%; 57/2; .12; −.26 | \(<10^{-13}\), faster |
| fEABC-LS/V2 | 56%; 32/19; .75; −.15 | 89%; 24/25; .50; **+.06** | 1.00 |
| fEABC-LS/MOR | 5%; 1/43; .99; +.09 | 11%; 0/59; .94; **+.49** | \(<10^{-16}\), slower |
| fEABC-LS/MIX | 48%; 29/20; .76; −.14 | 89%; 28/26; .50; **+.06** | 1.00 |
| TS-N2/V2 | 49%; 24/34; .83; −.03 | 90%; 35/24; .57; +.05 | .62 |
| TS-N2/MOR | 23%; 14/43; .88; +.02 | 43%; 19/41; .74; **+.22** | .031, slower |
| TS-N2/MIX | 66%; 39/19; .72; −.14 | 93%; 42/15; .46; −.06 | .0028, faster |

The corrected control RMTTs are GA .62, ABCE3 .38, fEABC-LS .44, and TS-N2 .52, rather than .94, .67, .90, and .86.

Thus:

- The strong ABCE3 acceleration remains.
- TS-N2/MIX remains an acceleration and becomes more clearly supported descriptively.
- The claimed negative RMTT differences for GA/MIX and fEABC-LS/MIX reverse sign.
- All reported Table 8 confidence intervals concern the wrongly implemented target and cannot reproduce the stated estimand; they must be regenerated.

### Smaller discrepancies

1. The manuscript later says the GA converges on 11 instances [main.tex:1375](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1375>); the script, CSV, and Table 4 give **12**.

2. It states that eight significantly slower arm–solver pairs are all attributable to MOR and GT [main.tex:1197](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1197>). MOR contributes four and GT three; the eighth is **GA/GP** under the implemented target: 16 earlier, 37 later, Holm \(p=0.0219\).

3. “Spanning an order of magnitude—11% against 40%” [main.tex:56](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:56>) is numerically false. The ratio is approximately 3.7, not ten. “Approximately fourfold” would be accurate.

## Major issues

### 1. The primary time-to-target analysis uses the wrong target

**Claim.** Table 8 measures time to the control’s final makespan within +0.5%.

**Evidence.** The code applies 1.005 to RPD rather than makespan [ttt_paired.R:92](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:92>), while the earlier implementation applies it correctly to `bestcmax` [ttt_phase2.R:45](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_phase2.R:45>).

**Why it matters.** This is not a cosmetic tolerance error. It changes the target by about 0.5 RPD points, reverses the mean-time signs for GA/MIX and fEABC-LS/MIX, and changes many reach rates and sign tests.

**Fix.** Define targets on raw `bestcmax`, regenerate Table 8, intervals, and all associated prose. State whether “final control quality” is the final trace incumbent or the returned solution; currently it is the former.

### 2. The time-to-target inferential argument remains incomplete even after fixing the target

**Claim.** The sign test is valid because setting non-reachers to \(T=1\) cannot reverse their sign, while restricted mean time is biased.

**Evidence.** The target is constructed from A0’s own observed endpoint [ttt_paired.R:83](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:83>), non-reachers are assigned 1 [ttt_paired.R:94](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:94>), and a binomial sign test is then applied [ttt_paired.R:111](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:111>).

**Why it matters.**

- I agree that an ordinary unpaired log-rank test is inappropriate: samples are paired and the target is endogenous to A0.
- However, “the censoring does not reverse signs” is insufficient to justify the exact binomial null \(P(\text{earlier})=P(\text{later})=0.5\). Even if two arms had identical stochastic processes, A0 is evaluated against a target selected from A0’s own realization, whereas the seeded arm is not. Exchangeability under the sign-test null has not been demonstrated.
- Assigning \(T=1\) is not biased for the explicitly named restricted-mean estimand \(E[\min(T,1)]\); it is exactly how that estimand is defined. It understates an unrestricted mean, which is a different estimand.
- A non-reacher is not always recorded as later: it can be tied when A0 reaches at \(T=1\). Ties are discarded.

**Fix.** Present the reach/earlier/later counts primarily as descriptive paired outcomes unless a valid randomization null is derived. Define the estimand clearly as restricted time. A simulation under identical stochastic processes, using the same endogenous-target construction, would show whether the nominal sign test controls type-I error.

### 3. “Converged quality” is not established

**Claim.** Restricting to instances whose A0 cell passes the plateau rule yields conclusions about converged quality.

**Evidence.** Only A0 traces enter the rule [convergence.R:38–45](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:45>); a cell passes when 90% of A0 runs do [convergence.R:64](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/convergence.R:64>). Seeded arms are not checked.

**Why it matters.** A control-converged instance is not an all-arms-converged experiment. The seeded arm could still be improving, particularly under the manuscript’s own GA “slow consolidation” explanation. Conditioning on A0 behavior also selects instances using one comparator’s outcome. Table 4 should therefore be described as “performance on A0-plateau instances,” not as converged-quality performance.

The threshold is also not “insensitive” [main.tex:560](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:560>): converged cells rise from 114 at 0.05% to 162 at 0.25%. My outcome sensitivity check shows the broad GA result is stable, but TS-N2’s best arm changes with the aggregation criterion: with 50% of runs required, MIX is best by 0.034 pp; with 80% or 90%, A0 is best.

Moreover, 325/1,830 TS-N2 control runs have negative calculated “improvement,” because the running-minimum midpoint trace can beat the returned LEX2-ranked solution. These automatically pass the \(<0.1\%\) rule even though the comparison does not represent progress in TS-N2’s native objective.

**Fix.** Check convergence separately for every arm and distinguish native LEX2 progress from midpoint progress. Report conclusions over intersections where both compared arms pass. Show outcome sensitivity, not merely counts, over both the plateau threshold and required-run fraction.

### 4. The fEABC-LS semantics do not match Table 1

**Claim.** fEABC-LS applies hill climbing to 75% of the population per generation [main.tex:256](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:256>).

**Evidence.** The setup indeed declares `localsearch.target = 0.75` [setup.txt:49](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2/feabcls/ft10/A0/c1/setup.txt:49>), but the algorithm invokes local search inside each employed-source iteration on a temporary two-member `currentFoodSources` population [ArtificialBeeColonyPSO.cpp:574](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/ArtificialBeeColonyPSO.cpp:574>). For a two-member population, `floor(0.75×2)−1=0` after the best member is treated [ArtificialBeeColonyPSO.cpp:727](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/ArtificialBeeColonyPSO.cpp:727>); thus only the better of the two candidates receives LS. The nominal random choice is also ignored in the general branch: `chosen` is sampled, but LS is applied to index `i` [ArtificialBeeColonyPSO.cpp:734](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/ArtificialBeeColonyPSO.cpp:734>), [ArtificialBeeColonyPSO.cpp:741](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/ArtificialBeeColonyPSO.cpp:741>).

**Why it matters.** This weakens the claim that the published configuration is replicated verbatim and affects the mechanism used to explain fEABC-LS’s early gains.

**Fix.** Establish the intended published semantics. If this is the intended original algorithm, describe it accurately. If 75% of the actual population was intended, correct the code and rerun fEABC-LS.

### 5. The lower-bound mathematics is sound, but the instance mapping is not verified as claimed

**Claim.** The Taillard mapping was “verified rather than assumed” [main.tex:524](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:524>).

**Evidence.** The script establishes exact identification for 19 instances where the trivial load bound equals the published LB; for the others it checks only `LBtriv <= LBpub` [check_ta_mapping.sh:40](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/check_ta_mapping.sh:40>), [check_ta_mapping.sh:41](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/check_ta_mapping.sh:41>). That inequality is necessary but does not identify an instance. The crisp files used by the audit are also referenced through an external personal path not present in the reviewed workspace [check_ta_mapping.sh:11](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/check_ta_mapping.sh:11>).

**Why it matters.** All RPDs and the six optimality certificates depend on correct mapping.

The theoretical use of a crisp LB is otherwise valid: componentwise addition/max and exact midpoint preservation do imply \(\operatorname{mid}(C_{\max}^{int}(x))\ge C_{\max}^{crisp}(x)\).

**Fix.** Ship the crisp instances or canonical hashes and perform an exact machine-routing and duration comparison against the canonical Taillard files for all 60 mappings.

### 6. The “independent verifier” does not verify the full claim

**Claim.** It independently reproduces the reported interval makespan exactly in 979/980 cases [main.tex:1405](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1405>).

**Evidence.**

- It compares only midpoint values, not both interval endpoints [verify_independent.cpp:214](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/verify_independent.cpp:214>). Different intervals with the same midpoint pass.
- Its insertion condition uses componentwise dominance [verify_independent.cpp:45](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/verify_independent.cpp:45>), while the solver SGS uses the configured EV or LEX2 comparison [SGS_IJSP_Insertion.cpp:85](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/SGS_IJSP_Insertion.cpp:85>), [SGS_IJSP_Insertion.cpp:107](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/SGS_IJSP_Insertion.cpp:107>).
- The serialized object is the returned phenotype’s schedule string [EvoLauncher.cpp:111](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/EvoLauncher.cpp:111>), not necessarily the chromosome. Hence Lamarckian re-encoding is not required to explain why the stored sequence contains schedule order.

**Why it matters.** The check remains useful, but it is not an exact independent replication of the solver’s interval decoder, and the single discrepancy may simply reflect the differing insertion rule.

**Fix.** Compare both endpoints; independently implement the same configurable comparison; separately test raw genotypes if the decoder itself is the object being validated.

### 7. Calibration and claimed sensitivity analyses need completion

The eight budget-calibration instances are reused in evaluation [main.tex:450](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:450>). This does not invalidate within-cell seeded-versus-control comparisons, but the budget-dependent generalizations are partly in-sample. Exclude these eight instances in a sensitivity analysis or use nested/out-of-sample calibration.

Similarly, the manuscript states that removing 20 TS-N2 tuning instances reproduces all conclusions [main.tex:414](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:414>), but I found no script, flag list, or aggregate output implementing that analysis. This claim is not verified.

### 8. “Composition beats quality” remains too strong

MIX has worse *mean* initial quality than V2 and GP, but it actually has the best initial individual in Table 9, albeit narrowly. Mean quality alone does not control tail quality, quantiles, basin coverage, or compatibility with subsequent operators. Therefore “direct evidence that the operative variable is composition, not seed quality” is stronger than the experiment supports. The limitations section later gives the more defensible formulation: an association requiring quality-matched pools to establish causality.

The abstract, Results, and Conclusions should consistently use that restrained interpretation.

## Minor issues

1. The trace-column repair is correct on real files: fields 8 and 9 are per-run `Runtime` and `Best` [raw header:56](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/phase2/ga/ft10/A0/c1/F0.15.0.ft10_10_20260809031955.csv:56>), matching [resample_anytime.sh:55](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/resample_anytime.sh:55>). All current cells contain six chunks and the aggregate solution-line counts are consistent with five runs per chunk.

2. Chunk offsets are nevertheless hard-coded as increments of five [resample_anytime.sh:76](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/resample_anytime.sh:76>). If a chunk directory exists but its trace CSV is missing, the offset is not advanced and subsequent runs are misnumbered. Derive offsets from setup metadata or the declared chunk number.

3. Early-ending runs are correctly carried forward, and output begins only once all observed runs have reported [resample_anytime.sh:101](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/resample_anytime.sh:101>). Thus run composition is stable after the first emitted grid point.

4. “Fraction of budget” is implemented as \(t/\max(t)\) separately for every instance–arm trace [fig_anytime_all.R:31](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/fig_anytime_all.R:31>) and similarly in TTT [ttt_paired.R:93](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/ttt_paired.R:93>). This is last sampled time, not the configured common budget. Use the setup budget explicitly.

5. The trace `Best` field is the midpoint of the solver’s native-ranking incumbent. For TS-N2 it is not necessarily the best midpoint in the population or among all evaluated solutions. The paper’s description should reflect this.

6. `CreationSeededSchedule` correctly implements the advertised circular block start [CreationIJSP.cpp:1442](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/CreationIJSP.cpp:1442>), correct chunk offsets are present in the setups, partial scout populations remain random, and injected permutations are re-decoded and re-evaluated. However, pool-line length and exact per-job occurrence counts are not validated before decoding.

7. The replication table is still marked TODO, ABCE3 remains 1.5–2.5 pp from the publication, the DOI is pending, and seed-generation time is missing. These are submission-blocking completeness issues.

8. The generator training/development provenance, GP tuning, and “i.i.d. pool” claims could not be verified from the provided C++ and final-analysis sources. Pool files are present, but the training/generation implementation is not.

9. The abstract says all three pilot practices “manufacture positive seeding results,” but stagnation-triggered stopping is described as penalizing the seeds. Say instead that they can manufacture or hide effects of either sign.

## What is genuinely good

- The endpoint RPD tables and instance-level effect analysis are reproducible from the released aggregates.
- The repaired trace parser now uses the correct per-run columns, applies a per-run running minimum, handles early endings by carry-forward, and prevents changing run composition.
- Treating instances—not 51,240 runs—as the main inferential unit is the right decision.
- The manuscript is unusually transparent about non-convergence, budget caps, overlapping seed blocks, TS-N2’s localized effect, and the lack of causal isolation for the mechanism claims.
- The mathematical justification for using crisp LBs under componentwise interval arithmetic is sound.
- The code confirms the essential injection protocol: circular seed blocks, correct global run offsets, random scouts, receiving-solver decoding, and no use of stored foreign objectives.
- The central substantive result—large ABCE3 benefit, fEABC-LS endpoint neutrality, and a budget- and size-dependent TS-N2 effect—survives scrutiny, although the time-to-target and convergence language requires substantial correction.