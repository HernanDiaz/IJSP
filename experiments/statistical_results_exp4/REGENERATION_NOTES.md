# Phase B — Canonical Layout (post bug-fix consolidation)

Updated 2026-06-05 after consolidating the N_3 / N_8 bug-fix campaign into the
standard `exp4/` layout.

## Source data (`experiments/results/exp4/`)

```
N1_tuned/    92 instances (incl. tai100_20)  — clean baseline
N2_tuned/    92 instances (incl. tai100_20)  — 1 residual outlier (gap 57)
N3_tuned/    82 instances                    — re-run with full bug fix
N8_tuned/    82 instances                    — re-run with bug fix
Next_tuned/  92 instances (incl. tai100_20)  — clean baseline
exp4_logs/   per-instance solver logs        — merged from the bug-fix re-runs
```

`tai100_20` was not re-run for N_3 / N_8. Analysis scripts restrict to the
**82 common instances** (intersection of all 5 folders) so the reported numbers
are an apples-to-apples comparison.

## Aggregated analysis outputs (`statistical_results_exp4/`)

| File | Rows | Description |
|---|---:|---|
| `runs_data.csv` | 12,300 | 1 row per (NB, instance, run) |
| `per_instance_stats.csv` | 410 | 1 row per (NB, instance): best_E, avg_E, sd_E, med_rt |
| `phaseB_group_summary.csv` | — | per (group × NB) mean RE + median runtime |
| `exp4_comparison_per_instance.csv` | 82 | wide format, 5 NBs side by side |
| `convergence_data_ts.csv` | 443 | per (NB, step) mean normalised best (for fig6) |
| `tab4_wilcoxon.tex` | — | Phase B Table 4 (Wilcoxon pairwise) |
| `tab_phaseb_groups.tex` | — | Phase B Table 6 (group RE × NB) |
| `wilcoxon_table.csv` | 10 | raw Wilcoxon stats |
| `friedman.txt` | — | Friedman χ²(4) = 3948.37 |
| `tab_lit_*.tex` | — | Phase C tables (unchanged) |

Mirror copy for R figure scripts: `Papers/COR_Tabu/data/phaseB_ts/`.

## Canonical pipeline

```bash
cd experiments

# 1. Build per-run + per-instance CSVs from exp4/ (filtered to 82 common)
python3 build_stats_data_exp4.py

# 2. Build per-generation convergence profile
python3 build_convergence_data_ts.py

# 3. Generate Phase B Wilcoxon + group tables in LaTeX
python3 gen_phaseB_tables.py
```

R side (figures + R-flavour outputs):
```bash
cd Papers/COR_Tabu/data/phaseB_ts && Rscript analyze_phaseB_ts.R
cd ../../figures
Rscript combine_fig5.R
Rscript generate_fig6_ts.R
Rscript generate_fig_taillard_compar.R
```

## Bug fixes in `NeighbourhoodIJSP_Cmax.cpp`

Four BFS-seeding patches addressing head/tail propagation. All commented
with `// BUGFIX` in the source.

1. eval tipo=1: missing arc->x, arc->z seeds
2. eval tipo=2: missing arc->x, arc->y seeds
3. eval tipo=0: missing arc->x seed (residual found later)
4. accept tipo=2: missing arc->x seed (tail side; cosmetic for makespan)

## Validation results

After fixes, validated all 2460 runs per neighbourhood on the 82 common
instances:

| NB | OK | Bug | Notes |
|---|---:|---:|---|
| N_1 | 2460/2460 | 0 | |
| N_2 | 2459/2460 | 1 | tai15_15_08 run 18, gap 57 on high |
| N_3 | 2459/2460 | 1 | tai20_20_07 run 7, gap 2 on high |
| N_ext | 2460/2460 | 0 | |
| N_8 | 2460/2460 | 0 | |
