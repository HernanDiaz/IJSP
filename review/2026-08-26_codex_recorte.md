## Editorial verdict

The paper is not intrinsically 47 pages’ worth of content. It contains a strong 35–38-page paper trapped inside an over-complete Results narrative and a severe float backlog.

The compiled structure matters:

- Results begins on page 13; Discussion on page 32; Conclusions on page 36.
- Tables 5–12 are deferred to pages 41–47, after the references.
- Thus seven pages are float-only tail pages. Several cuts below will translate directly into page savings.

A conservative edit can remove approximately **9–12 pages**, reaching **35–38 pages**, without rerunning experiments or weakening the protected evidence. The largest gains come from eliminating parallel presentations of the same evidence, not from thinning the study.

Supplementary-material savings below assume Elsevier excludes a separate supplement from the manuscript page count. If the supplement is appended to the same counted PDF, relocation improves readability but saves no total pages.

## Prioritised cutting plan

Estimates overlap slightly where prose and tables are merged.

| Priority | Proposal | Location | Saving | What is lost |
|---|---|---|---:|---|
| 1 | **Delete §7.10 and Table 11 from the main paper.** The five pilot statements are already stated in §5.4, and every verdict is already established in §§7.2–7.9. Put the verdict table in the supplement if pre-specification traceability is desired. | [main.tex:409](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:409>), [main.tex:1371](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1371>) | **1.2–1.7 pp** | Only the convenient “pilot prediction versus final verdict” recap. No evidence or contribution is lost. |
| 2 | **Remove Table 7 from the main paper; retain Figure 2 and four 10%-budget comparisons in prose.** Table 7 numerically reproduces Figure 2 and its 100% column reproduces Table 4. Full values can go to the supplement/data release. | [main.tex:950](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:950>), [main.tex:971](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:971>) | **1.0–1.3 pp** table alone; **1.7–2.3 pp** with prose compression | Exact values at 25%, 50%, and 75% cease to be visible in the article. The anytime profiles and key 10% result remain. |
| 3 | **Collapse §§7.2–7.4 into “Final quality and convergence” and eliminate duplicate tables.** Delete the four “all (61)” rows of Table 2 because they are identical to Table 4; preferably remove Table 2 entirely and retain the convergence counts in prose plus compact Table 3. Then delete Table 4 by adding each solver’s A0 RPD once to Table 5 or the accompanying text. | [main.tex:572](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:572>), [main.tex:589](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:589>), [main.tex:622](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:622>), [main.tex:714](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:714>), [main.tex:792](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:792>) | **1.5–2.2 pp** | A standalone absolute-RPD matrix and the weaker control-only convergence table. The stronger both-plateau comparison and the instance-unit inference remain. |
| 4 | **Halve §7.7 while keeping its substantive result.** Move the log-rank/censoring defence and split-half-null details to supplementary methods. Keep: target definition, reach rate, primary paired sign test, and interval for ΔRMTT. In Table 8 delete the four A0 rows and the absolute RMTT column; retain **Reach**, Earlier/later, corrected \(p\), and ΔRMTT [CI]. Shorten the 18-line caption. | [main.tex:1109](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1109>), [main.tex:1121](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1121>), [main.tex:1174](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1174>) | **1.3–1.9 pp** | Detailed explanation of why several alternative survival analyses were rejected. Do **not** drop Reach: it is necessary because failures to attain the target cannot be hidden. |
| 5 | **Move Table 6 to the supplement.** Keep Figure 1 and retain in the text only the decisive numbers: ten \(50\times20\) instances, \(-1.06\%\), and 94.8% of the TSN2 aggregate improvement. | [main.tex:886](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:886>), [main.tex:919](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:919>) | **0.8–1.0 pp** | The seven-row arithmetic decomposition. The claim and its visual evidence remain. |
| 6 | **Compress §7.1 to one paragraph plus the lower-bound inequality.** Retain that interval midpoints recover the crisp durations, the crisp LB is valid, there were no violations, and six bounds were attained. Move hashes, the 57,450-operation audit, 19-versus-41 mapping discussion, recovered matrices, and six instance names to the supplement/data description. | [main.tex:521](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:521>) | **0.8–1.2 pp** | Audit detail, not validity. The proof of the reference and reproducibility route remain. |
| 7 | **Merge §§7.6 and 7.7 as “Anytime performance.”** Retain Figure 2 and compact Table 8. Reduce the four paragraphs on generator ordering and GA crossover to one paragraph each. The methodological lesson already belongs to the pilot. | [main.tex:950](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:950>), [main.tex:1033](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1033>), [main.tex:1051](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1051>), [main.tex:1101](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1101>) | **1.0–1.5 pp**, beyond Tables 7–8 savings | Secondary trajectory commentary; the full curves remain. |
| 8 | **Remove duplicated Results interpretations.** After Table 5, give one paragraph covering the four solvers. Do not separately narrate the same ABCE3, GA and TSN2 conclusions in §§7.2, 7.3 and 7.4. | [main.tex:660](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:660>), [main.tex:739](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:739>), [main.tex:854](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:854>) | **0.8–1.2 pp** | Repetition and rhetorical emphasis only. |
| 9 | **Remove the Introduction’s miniature pilot and second results abstract.** Replace lines 102–115 with a two-sentence preview referring to §5. Delete or reduce lines 144–152 to one roadmap sentence. Also avoid repeating the solver history already in Related Work. | [main.tex:78](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:78>), [main.tex:102](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:102>), [main.tex:144](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:144>) | **0.6–0.9 pp** | Detailed early preview. The contribution list and full pilot remain. |
| 10 | **Shorten “threats addressed” and move it into Methods/Limitations.** Keep the independent verifier, its systematic 980-solution sample and 979/980 result. Keep a concise disclosure of convergence-rule sensitivity and the TSN2 LEX2 issue. Move the calibration grid and repeated blocking/stopping statements to supplementary methods. | [main.tex:1418](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1418>) | **0.7–1.0 pp** | Detailed sensitivity values and repeated design assurances. The required verification and convergence caveats remain. |
| 11 | **Compress Discussion and Conclusions to synthesis rather than a third Results account.** In §8.1 remove the numeric solver ordering already reported. Move the three qualifications at lines 1586–1599 into §8.4, where the mechanism limitations are currently repeated. Reduce Conclusions to about 300 words. | [main.tex:1483](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1483>), [main.tex:1586](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1586>), [main.tex:1615](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1615>), [main.tex:1659](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1659>) | **1.0–1.5 pp** | Reiteration, not interpretation. Preserve the bounded causal language and generalisation limits. |
| 12 | **Compress implementation defences.** Table 1’s code-level local-search footnote should become a two-sentence disclosure, with file/line details in the supplement. Retain the replication gap but reduce the following defence to “all conclusions are within-implementation, within-solver contrasts.” Shorten pool-overlap arithmetic and RNG-pairing explanation similarly. | [main.tex:238](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:238>), [main.tex:260](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:260>), [main.tex:274](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:274>), [main.tex:339](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:339>), [main.tex:492](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:492>) | **0.5–0.8 pp** | Forensic implementation detail. Replication transparency remains. |

If a still shorter target is required, move Figure 3—the absolute-time \(50\times20\) view—to the supplement, saving another **0.7–1.0 page**. It is useful, but Figures 1–2 already establish size concentration and anytime behaviour.

## Table-by-table judgement

| Table | Claim that depends on it | Recommendation |
|---|---|---|
| **1. Solver portfolio** | Establishes that four published configurations span distinct search mechanisms. | **Keep, compress.** Remove shared properties already stated in text and shorten the long implementation footnote. |
| **2. Control-converged RPD** | Shows which conclusions survive conditioning on control convergence. | **Remove from main.** Its all-instance rows duplicate Table 4; its restricted comparison is weaker than Table 3 because seeded arms need not have converged. Put full rows in supplement. |
| **3. Both-arm convergence intersections** | Carries the protected finding that GA/MIX improves where both plateau, while TSN2/MIX becomes null. | **Keep.** Drop the separate A0 and Arm columns and report only \(n\) and \(\Delta\), or retain one absolute column plus \(\Delta\). |
| **4. Final all-instance RPD** | Gives descriptive absolute performance at the budget. | **Merge into Table 5.** Its differences are already represented in Table 5, and its values recur in Table 2 and Table 7 endpoints. |
| **5. Instance-unit effects** | Main inferential evidence: mean effect, CI, corrected test; also per-instance Wilcoxon counts already folded in. | **Definitely keep.** If width must fall, move \(\hat A_{12}\) to the supplement before touching mean difference, CI or corrected \(p\). Do not recreate a separate counts table. |
| **6. TSN2 class decomposition** | Supports “94.8% of the gain comes from \(50\times20\).” | **Supplement.** Figure 1 plus three retained numbers support the main claim. |
| **7. Budget fractions** | Supplies exact values behind Figure 2 and the short-budget percentages. | **Supplement.** No unique qualitative claim depends on the full table. |
| **8. Time to target** | Carries the second protected anytime metric, reach rates, primary paired test and interval. | **Keep, shorten.** Remove A0 rows and absolute RMTT; retain Reach, Earlier/later, \(p\), and ΔRMTT [CI]. |
| **9. Initial-population quality** | Direct evidence that MIX starts worse than V2/GP but finishes better. | **Keep.** Delete “Best individual”; the text itself says the mean is the quantity characterising the pool. |
| **10. Friedman ranking** | Supports MIX being the best seeded strategy across solvers and MOR being last. | **Keep.** Delete the \(\chi^2\) row because the caption already reports \(p<10^{-15}\). |
| **11. Pilot statements** | No new empirical claim; it indexes results found elsewhere. | **Delete or supplement.** Highest-value table cut. |
| **12. Hamming diversity** | Essential evidence against premature convergence and for consolidation failure. | **Keep, compact.** “Start” is nearly identical across arms and “Drop” is computed from Start/End; keep Mean and End. |

For the figures:

- **Figure 1 earns its space**: it prevents overgeneralising the TSN2 result and shows the cross-solver size interaction.
- **Figure 2 earns its space**: anytime behaviour is a central methodological contribution.
- **Figure 3 is the first figure to relocate** if necessary. Its absolute-time view is useful corroboration, but not indispensable once Figures 1–2 and the convergence classification remain.

## Recommended new Results structure

The current eleven subsections have outgrown their evidentiary roles. I would reduce them to five:

1. **Reference and verification of the performance measure** — compressed §7.1.
2. **Final quality, instance-unit inference and convergence** — merge §§7.2–7.4; retain compact Tables 3 and 5.
3. **Interaction with instance size** — retain Figure 1; move Table 6.
4. **Anytime performance and time to target** — merge §§7.6–7.7; retain Figure 2 and compact Table 8.
5. **Pool composition and strategy ranking** — merge §§7.8–7.9; retain compact Tables 9–10.

Then:

- Delete §7.10 from the article.
- Distribute the indispensable parts of §7.11 between Experimental Design and Limitations.
- Optionally place Figure 3 in the supplement.

This would make Results approximately **12–14 pages including its main floats**, rather than roughly 19 pages of narrative plus much of the seven-page deferred-float tail.

## Specific redundancy pairs

The clearest duplicates are:

- **Introduction artefacts ↔ Pilot:** [lines 102–115](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:102>) rehearse §§5.1–5.3 almost point for point.
- **Introduction result teaser ↔ Abstract/Results/Conclusions:** [lines 124–152](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:124>) already state nearly the entire final interpretation.
- **Table 2 all-instance rows ↔ Table 4:** the four rows are numerically identical.
- **Table 4 ↔ Table 5:** Table 5’s mean difference plus one A0 value per solver reproduces the substantive comparison.
- **Table 4 ↔ Table 7’s 100% column:** same endpoint, differing only by trace-sampling resolution.
- **Figure 2 ↔ Table 7:** graphical and numerical versions of the same anytime profiles.
- **Figure 1 ↔ Table 6 ↔ lines 903–917:** three presentations of the TSN2 \(50\times20\) concentration.
- **Pilot §5.4 ↔ §7.10/Table 11:** exactly the same five statements, first as predictions and later as a verdict table.
- **MIX starts worse:** [lines 1278–1289](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1278>) ↔ sampling-objection paragraph [1358–1369](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1358>) ↔ Discussion [1505–1517](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1505>) ↔ Conclusions [1690–1697](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1690>).
- **GA/TSN2 convergence reversals:** [675–707](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:675>) ↔ [769–777](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:769>) ↔ [862–883](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:862>) ↔ Discussion [1496–1503](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1496>) ↔ Conclusions [1678–1688](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1678>).
- **Time-to-target definitions and defence:** Pilot [396–407](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:396>) ↔ Design [507–511](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:507>) ↔ Results [1111–1206](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1111>) ↔ threats [1420–1437](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1420>).
- **Mechanism limitations:** [1586–1599](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1586>) repeats the Mechanism limitation at [1626–1631](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1626>).

## Passages that argue rather than report

Necessary, but too long:

- The Table 1 implementation discrepancy and replication gap. Disclosure is essential; source-code forensics can move to the supplement.
- The lower-bound validity argument in §7.1. The inequality must remain; the extended identification defence need not.
- The endogenous time-target calibration. Because the null proportion is nonstandard, it must be disclosed, but the 400-replicate detail can be supplementary.
- The TSN2 LEX2/plateau issue. This is an important limitation of the convergence classification; retain it concisely.
- The consolidation account in §8.2. It is one of the paper’s valuable findings, but its three qualifications should appear once, in Limitations.

Padding or obvious supplementary material:

- The 4,900 CPU-hour/fourteen-day justification for the 900-second cap at [main.tex:480](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:480>). State the cap as a feasibility constraint and its inferential consequence.
- The exact pool-block overlap arithmetic at [main.tex:343](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:343>).
- The extended common-random-numbers defence at [main.tex:493](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:493>).
- The sampling-objection paragraph at [main.tex:1358](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1358>), which repeats §7.8.
- The success-only MOR counterexample at [main.tex:1243](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1243>).
- The methodological sermon at [main.tex:1101](<C:/Users/diazhernan/CLionProjects/RL Seeds/IJSP/final/paper/main.tex:1101>), already demonstrated by the pilot and curves.

## Abstract

At approximately 283 words, it should come below 250; I would target **235–245 words**.

The cleanest reductions are:

- Replace lines 54–58—the four percentages and solver-order reversal—with:  
  “All four solvers benefit at short budgets, but the ordering reverses by the budget limit.”
- Replace lines 60–65 with:  
  “MIX starts from a worse average population yet finishes better, implicating composition; the harmed GA remains more dispersed, contradicting premature convergence.”
- Compress the design inventory in lines 39–43 by removing “spanning the weak-to-strong axis” and shortening the generator description.

Those edits remove roughly **40–50 words** while retaining the question, pilot contribution, design scale, primary interval result, convergence qualification, anytime result and mechanism.

## What I would not cut

I would protect:

- **The pilot study itself.** It is a stated contribution and gives the paper a methodological identity. Cut its later recap, not §5.
- **The convergence classification and both-plateau comparison.** They materially change the GA and TSN2 interpretations.
- **Table 5’s instance-unit mean effects, confidence intervals and corrected tests.** This is the inferential centre of the paper.
- **The independent verifier and its 979/980 result.** It is unusually strong credibility evidence.
- **Figure 1’s size interaction.** Without it, the TSN2 claim is easy to overstate.
- **Figure 2 and time-to-target Reach data.** These carry the anytime contribution and prevent success-only reporting.
- **The mean column of Table 9.** It is the direct evidence for “composition rather than initial quality.”
- **The compacted Hamming evidence.** It overturns the standard premature-convergence explanation.
- **The ABCE3 replication gap.** A referee would react badly if this were hidden.
- **The causal limitations.** The paper is credible precisely because it labels composition and consolidation as supported associations rather than isolated causes.

The right editorial move is therefore not to thin the experimental core. It is to keep one presentation of each result, relocate audit detail, and stop retelling the same conclusions in Results, Discussion and Conclusions.