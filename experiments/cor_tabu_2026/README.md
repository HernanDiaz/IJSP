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
├── scripts/                   analysis pipeline + utilities
│   ├── build_phaseB_stats.py
│   ├── build_convergence_data_ts.py
│   ├── gen_phaseB_tables.py
│   ├── validate_phaseA.py
│   ├── rebuild_supp_taillard_tables.py
│   ├── patch_supp_taillard.py
│   ├── rebuild_dataset_zip.py
│   ├── build_submission_zip.py
│   └── check_paper_compliance.py
├── diagnostics/               bug-fix tooling and PDF inspection helpers
│   ├── scan_n8_bug.py
│   ├── verify_ta54_2792.py
│   └── figtab_map.py
├── setup/                     irace-tuned config files (5 neighbourhoods)
│   └── setup_N{1,2,3,8,Next}_tuned.txt
├── irace/                     irace tuning records (input + master log)
│   ├── parameters.txt
│   ├── instances.txt
│   ├── scenario.txt
│   ├── base_setup_ts_lex2.txt
│   ├── forbidden.txt
│   ├── run_irace.sh
│   └── results/                  per-neighbourhood irace dumps
├── statistical_results/       aggregated Phase B outputs
│   ├── runs_data.csv             per (NB, instance, run) interval data
│   ├── per_instance_stats.csv    per (NB, instance) summary
│   ├── convergence_data_ts.csv   per-generation profile (fig 6)
│   ├── wilcoxon_table.csv        Wilcoxon raw results
│   ├── tab4_wilcoxon.tex         Table 4 LaTeX
│   ├── tab_phaseb_groups.tex     Table 6 LaTeX
│   ├── friedman.txt              Friedman test result
│   └── REGENERATION_NOTES.md     pipeline overview + data sources
└── run_phaseB_tai100.sh       optional extension run script
```

## Reproducing the paper analysis

The scripts under `scripts/` use absolute paths to the project root, so they
can be invoked from any working directory.

### 1. Pull the raw run data from Zenodo

The full raw run logs (~118 MB) are not stored in this repo; they live on
Zenodo at https://doi.org/10.5281/zenodo.20562888. Extract the zip into
`experiments/results/exp4/` (preserving the per-neighbourhood
`N1_tuned/`, `N2_tuned/`, `N3_tuned/`, `N8_tuned/`, `Next_tuned/`
subdirectories).

### 2. Regenerate the aggregated CSVs

```bash
cd experiments/cor_tabu_2026/scripts
python3 build_phaseB_stats.py        # → ../statistical_results/{runs_data,per_instance_stats}.csv
python3 build_convergence_data_ts.py # → ../statistical_results/convergence_data_ts.csv
python3 gen_phaseB_tables.py         # → ../statistical_results/{tab4_wilcoxon,tab_phaseb_groups}.tex, friedman.txt, wilcoxon_table.csv
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
