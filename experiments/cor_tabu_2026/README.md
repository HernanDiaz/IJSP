# Phase B experimental package — COR 2026 paper

This directory contains all experimental setup and results tied to the paper:

> **Neighbourhood Structures and Ranking Operators for the Interval Job Shop
> Scheduling Problem**
> Hernán Díaz Rodríguez. Submitted to *Computers & Operations Research*, 2026.

The code state corresponding to the submitted version is tagged
**`cor-tabu-2026-submission`** in this repository.

## Layout

```
cor_tabu_2026/
├── setup/                     irace-tuned config files (5 neighbourhoods)
│   ├── setup_N1_tuned.txt
│   ├── setup_N2_tuned.txt
│   ├── setup_N3_tuned.txt
│   ├── setup_N8_tuned.txt
│   └── setup_Next_tuned.txt
├── irace/                     irace tuning records (input + master log)
│   ├── parameters.txt
│   ├── instances.txt
│   ├── scenario.txt
│   ├── base_setup_ts_lex2.txt
│   ├── forbidden.txt
│   ├── run_irace.sh
│   └── results/                  per-neighbourhood irace dumps
└── statistical_results/       aggregated Phase B outputs
    ├── runs_data.csv             per (NB, instance, run) interval data
    ├── per_instance_stats.csv    per (NB, instance) summary
    ├── convergence_data_ts.csv   per-generation profile (fig 6)
    ├── wilcoxon_table.csv        Wilcoxon raw results
    ├── tab4_wilcoxon.tex         Table 4 LaTeX
    ├── tab_phaseb_groups.tex     Table 6 LaTeX
    ├── friedman.txt              Friedman test result
    └── REGENERATION_NOTES.md     pipeline overview + data sources
```

## Reproducing the paper analysis

The analysis pipeline scripts live one level up (`experiments/`). They are
generic across papers; the setup/irace/statistical_results dirs above are the
inputs and outputs specific to this paper.

### 1. Pull the raw run data from Zenodo

The full raw run logs (~118 MB) are not stored in this repo; they live on
Zenodo at https://doi.org/10.5281/zenodo.20562888. Extract the zip into
`experiments/results/exp4/` (preserving the per-neighbourhood
`N1_tuned/`, `N2_tuned/`, `N3_tuned/`, `N8_tuned/`, `Next_tuned/`
subdirectories).

### 2. Regenerate the aggregated CSVs

```bash
cd experiments
python3 build_phaseB_stats.py       # → cor_tabu_2026/statistical_results/{runs_data,per_instance_stats}.csv
python3 build_convergence_data_ts.py  # → convergence_data_ts.csv
python3 gen_phaseB_tables.py         # → tab4_wilcoxon.tex, tab_phaseb_groups.tex, friedman.txt, wilcoxon_table.csv
```

### 3. Regenerate the figures (requires R)

```bash
cd Papers/COR_Tabu/figures
Rscript combine_fig34.R                # Figure 3 (Phase A combined)
Rscript combine_fig5.R                 # Figure 4 (Phase B boxplot + tradeoff)
Rscript generate_fig6_ts.R             # Figure 5 (convergence)
Rscript generate_fig_taillard_compar.R # Figure 6 (Phase C Taillard)
```

### 4. Recompile the paper

```bash
cd Papers/COR_Tabu
latexmk -pdf main.tex
```

## Related Zenodo records

- Dataset (raw runs + scripts): https://doi.org/10.5281/zenodo.20562888
- Supplementary (per-instance Taillard tables): https://doi.org/10.5281/zenodo.20562857
