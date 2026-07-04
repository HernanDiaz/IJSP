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

## Analysis (TODO once outputs exist)

Aggregate the raw CSVs into the same `config,neigh,comp,instance,run,lower,upper,
midpoint` schema as the baseline, then paired Wilcoxon N2Plus-vs-N2 and
N2Minus-vs-N2 per instance and overall (mirror `cor_tabu_2026/scripts/`).
