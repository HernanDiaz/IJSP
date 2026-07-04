# N2 worst-case neighbourhood experiment

## Hypothesis

The published tabu search ranks interval makespans with **LEX2** (minimise the
upper bound `b` first, tie-break on the lower bound `a`). Yet its **N2**
neighbourhood generates arc reversals from the critical paths of **both**
extreme graphs — G⁻ (lower bound, `comp=1`) and G⁺ (upper bound, `comp=2`)
(`NeighbourhoodIJSP_N2.cpp`, the `for comp=1..2` loop).

**Claim:** because LEX2 optimises the worst case, it should suffice to explore
only the critical path of the **upper** bound (G⁺). This aligns move generation
with the objective and roughly halves the candidate arcs per iteration, so the
tabu search does more useful iterations in the same 900 s budget — matching or
beating N2 on makespan.

## Configurations

| Config  | Neighbourhood        | Critical path used            | Role      |
|---------|----------------------|-------------------------------|-----------|
| N2 (baseline) | `ijsp.makespan.n2`      | G⁻ **and** G⁺ (comp 1 & 2) | published |
| **N2Plus**    | `ijsp.makespan.n2plus`  | G⁺ only (comp 2, worst case) | test    |
| **N2Minus**   | `ijsp.makespan.n2minus` | G⁻ only (comp 1, best case)  | control |

N2Plus is the tested idea. N2Minus is the mirror control: under LEX2 it is
expected to underperform, which isolates the effect to *the worst case*
specifically, not to "using a single component".

All three share the identical irace-tuned N2 hyperparameters (`setup/` files are
byte-for-byte copies of `cor_tabu_2026/setup/setup_N2_tuned.txt` with only the
`localsearch.neighbourhood` line changed). Hyperparameters are **not re-tuned**,
so the comparison isolates the neighbourhood.

Connectivity of N2Plus/N2Minus is guaranteed by Nowicki–Smutnicki (1996)
applied to G⁺ / G⁻ as a deterministic JSP (see docstring in
`NeighbourhoodIJSP_N2Plus.h`).

## Baseline for comparison

The N2 results are already in the paper — do **not** re-run them. They live at:
`experiments/cor_tabu_2026/statistical_results/runs_data.csv`
(rows with `neigh == n2`: 82 instances × 30 runs, families abz/ft/la/tai15–tai100).

## How to run (WSL)

```bash
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP && make -j4   # build once
bash experiments/n2_worstcase_2026/run_n2_worstcase.sh <tier> [max_parallel]
```

Tiers (staged by size to control compute): `smoke` (ft10 only) → `classical`
(abz/ft/la) → `tai_small` (tai15/tai20) → `full` (all 82, includes tai50/tai100
= hours each). Re-running skips pairs that already have a final CSV.

Raw per-run CSVs land in `results/N2Plus/` and `results/N2Minus/`.

## Analysis

Run `python3 analyze.py` (pure stdlib): dedups raw CSVs by latest timestamp,
computes RE(%) per size class, paired Wilcoxon vs the published N2, and the
runtime comparison. Aggregated verdict snapshot: `RESULTS_SUMMARY.txt`.

## Outcome (2026-07-04, full 82 instances) — hypothesis CONFIRMED

- **N2Plus ≡ N2 in quality**: grand mean RE 3.48 vs 3.47, Wilcoxon p = 0.58,
  indistinguishable in all 8 size groups — at equal-or-lower runtime
  (−22.6% on 50×20; −18.4% vs N2Minus overall, faster on 71/82, p < 0.0001).
- **N2Minus (control) significantly worse AND slower**: p < 0.05 in the 6
  non-trivial groups (grand mean RE 3.82), +18% slower than N2Plus.

## Note — theoretical justification (why this does not contradict the paper)

The paper's connectivity theorem (Thm 2) requires arcs from BOTH extreme
critical paths, but that requirement buys *ranking-agnosticism*: it is proved
for any admissible ranking R. LEX2 is not generic — it is lexicographic
min–max — and specialising the theorem to it collapses the requirement:

1. **Primary objective.** The key-lemma argument applied to G⁺ alone shows
   N2Plus is connectivity-complete for C⁺: any C⁺ deficit is repairable via a
   G⁺-critical boundary arc. By Prop. 2 applied to G⁺, arcs critical only in
   G⁻ can never reduce C⁺ — they can at most improve the tie-break C⁻.
2. **Tie-break.** The only gap is a C⁻-only improving arc on a G⁻ path that
   diverges from G⁺ while C⁺ is already optimal. Phase A's structural finding
   ("LEX2 concentrates criticality in G⁺") makes this rare, hence the
   empirical equivalence. Exact limit: if interval widths are relatively
   uniform (p⁺ = c·p⁻), the G⁻/G⁺ critical structures coincide and
   N2Plus ≡ N2 ≡ N2Minus with full LEX2-completeness. General sufficient
   condition: criticality containment H⁻(σ) ⊆ H⁺(σ).
3. **The control measures the deviation.** N2Minus is complete only for the
   secondary criterion, blind to C⁺ — its per-group quality gap is an
   empirical metric of G⁻/G⁺ critical-structure divergence.

Takeaway: *the neighbourhood must align with the ranking operator*. "Both
paths" is the price of ranking-agnosticism; once LEX2 is fixed, G⁺ suffices
(symmetrically, LEX1 → G⁻). Candidate follow-up contribution: proposition
"under lexicographic min–max ranking, the G⁺ sub-neighbourhood preserves
connectivity for the primary objective, and full LEX2-completeness holds
under criticality containment", with N2Minus as the experimental falsifier.
