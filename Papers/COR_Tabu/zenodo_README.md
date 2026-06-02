# Experimental Dataset — Neighbourhood Structures and Ranking Operators for the Interval Job Shop Scheduling Problem

This dataset accompanies the paper:

> Hernán Díaz Rodríguez. *Neighbourhood Structures and Ranking Operators for the
> Interval Job Shop Scheduling Problem*. Computers & Operations Research, 2026.
> DOI: \[paper DOI\].

It contains the raw experimental data, irace tuning records, instance files, and
analysis scripts that underlie every figure and table in the article.

---

## 1. Dataset overview

The experimental study compares five neighbourhood variants
($N_1$, $N_2$, $N_3$, $N_{\text{ext}}$, $N_8$) and four interval-ranking operators
(EV, LEX1, LEX2, YX) for the Interval Job Shop Scheduling Problem (IJSP), on 82
benchmark instances ranging from $10\times10$ to $50\times20$ operations. Three
experimental phases are reported:

- **Phase A** — common-hyperparameter hill climbing for the $5 \times 4$ operator
  comparison (Section 8.2 of the paper).
- **Phase B** — per-neighbourhood irace-tuned tabu search for the head-to-head
  neighbourhood comparison (Section 8.3).
- **Phase C** — comparison of the best Phase B configuration against three
  published IJSP solvers (Section 8.4).

Each (instance, configuration) pair is executed for 30 independent runs.

---

## 2. Directory layout

```
.
├── README.md                          (this file)
├── LICENSE                            (CC-BY 4.0)
│
├── instances/                         Instance files
│   ├── classical/                       12 OR-Library instances (ABZ7-9, FT10,
│   │                                    FT20, LA21/24/25/27/29/38/40)
│   └── taillard_82/                     70 Taillard 15x15 to 50x20 instances
│                                        (TA1-TA70)
│
├── setup/                             Algorithm setup files (parameters)
│   ├── phaseA/                          Phase A common-HC setups
│   │   └── setup_<nb>_<op>.txt          (20 files: 5 nb x 4 op)
│   └── phaseB/                          Phase B irace-tuned TS setups
│       └── setup_N<x>_tuned.txt         (5 files: one per neighbourhood)
│
├── results/                           Raw per-run results (one CSV per run)
│   ├── phaseA_HC/                       Hill-climbing results
│   │   └── <nb>_<op>/                   20 directories (5 nb x 4 op)
│   │       └── <instance>_<timestamp>.csv  (+ _Sols)
│   └── phaseB_TS/                       Tabu-search results
│       └── N<x>_tuned/                  5 directories
│           └── <instance>_<timestamp>.csv  (+ _Sols, _Robustness, _Scenarios)
│
├── irace/                             irace tuning records
│   ├── parameters.txt                   parameter space
│   ├── scenario.txt                     irace scenario configuration
│   ├── instances.txt                    20 training instances (stratified
│   │                                    random subset of 82)
│   ├── forbidden.txt                    forbidden parameter combinations
│   ├── base_setup_ts_lex2.txt           base TS+LEX2 template
│   └── results/
│       └── irace_N<x>/                  5 directories (one per neighbourhood)
│           └── irace_N<x>.log             irace progress log
│
├── aggregated_csvs/                   Pre-aggregated CSVs (sources of figures)
│   ├── phaseB/
│   │   ├── runs_data.csv                per-(neigh, instance, run) midpoint
│   │   │                                (source: Table 4, Figure 5)
│   │   ├── phaseB_group_summary.csv     per-(neigh, size class) summary
│   │   │                                (source: Table 6)
│   │   ├── per_instance_stats.csv       per-(neigh, instance) avg/best/SD/median
│   │   │                                runtime
│   │   └── convergence_data_ts.csv      per-(neigh, step) normalised best
│   │                                    midpoint (source: Figure 6)
│   └── phaseC/
│       └── (taillard per-instance comparison; see Supplementary Material)
│
└── scripts/                           Analysis scripts
    ├── python/
    │   ├── build_stats_data_exp4.py       per_instance_stats.csv builder
    │   ├── build_convergence_data_ts.py   convergence_data_ts.csv builder
    │   └── gen_tab6_v2.py                 Table 6 + supplementary table
    │                                      builder
    └── R/
        ├── combine_fig34.R                generates the Phase A operator
        │                                  comparison figure (paper Figure 3)
        ├── combine_fig5.R                 generates fig5_combined.pdf
        ├── generate_fig6_ts.R             generates fig6_convergence.pdf
        ├── generate_fig_taillard_compar.R generates fig_taillard_compar.pdf
        └── get_phaseC_runtimes.R          Phase C runtime extraction
```

---

## 3. File formats

### 3.1 Instance files (`instances/`)

Plain-text format compatible with the FuzzyFW solver. Each file describes the
job-machine matrix with **interval processing times**
$\mathbf{p}_{ij} = [0.925\,p^{*}_{ij},\, 1.075\,p^{*}_{ij}]$ derived from the
nominal Taillard or OR-Library duration $p^{*}_{ij}$.

### 3.2 Per-run results (`results/`)

Each (instance, configuration, run-batch) produces two CSVs:

- **`<instance>_<timestamp>.csv`** — main result CSV. Contains:
    - Solver configuration in lines 1-43 (algorithm name, parameters, encoding,
      neighbourhood, local search, stopping criteria).
    - Per-run runtime breakdown (Total runtime, Creation, Selection, Crossover,
      Mutation, Replacement, …) starting at line 44.
    - Per-run solution quality (`Best solution` line: averages, best, worst,
      stddev, then 30 per-run values).
    - Optional: per-iteration evolution traces.
- **`<instance>_<timestamp>_Sols.csv`** — the best schedule of each run, as
  `<run>;<permutation>;<objective>`.

### 3.3 Aggregated CSVs (`aggregated_csvs/`)

Standard CSV (comma-separated) format. Column headers documented inside each
file. Key columns:

- `runs_data.csv`: `config, neigh, comp, instance, run, lower, upper, midpoint`
  (12,300 rows = 5 neigh x 82 inst x 30 runs).
- `phaseB_group_summary.csv`: `group, neigh, n, mean_re, med_re, med_rt, …`
  (40 rows = 5 neigh x 8 size classes).
- `per_instance_stats.csv`: `neigh, instance, group, n_runs, best_E, avg_E,
  sd_E, med_rt` (410 rows = 5 neigh x 82 instances).
- `convergence_data_ts.csv`: `neigh, step, mean_best_norm, sd_best_norm,
  n_obs` (one row per step per neighbourhood).

---

## 4. Reproducing the paper's figures and tables

Each artefact in the paper can be regenerated from this dataset:

| Artefact | Source CSV | Script |
|----------|------------|--------|
| Table 1 (Phase A midpoints) | `per_instance_stats.csv` (Phase A subset) | `scripts/python/build_stats_data_exp4.py` |
| Table 4 (Wilcoxon Phase B) | `runs_data.csv` | (in-paper R script, not included) |
| Table 6 (Phase B groups) | `phaseB_group_summary.csv` | `scripts/python/gen_tab6_v2.py` |
| Table 7 (Phase C classical) | Phase B + published comparators | manual aggregation |
| Figure 3 (Phase A operator comparison) | `per_instance_stats.csv` (Phase A subset) | `scripts/R/combine_fig34.R` |
| Figure 4 (Phase B boxplot + scatter) | `runs_data.csv`, `phaseB_group_summary.csv` | `scripts/R/combine_fig5.R` |
| Figure 5 (Phase B convergence) | `convergence_data_ts.csv` | `scripts/R/generate_fig6_ts.R` |
| Figure 6 (Phase C Taillard) | `phaseB_group_summary.csv` + supplementary data | `scripts/R/generate_fig_taillard_compar.R` |
| Supplementary tables | `per_instance_stats.csv` | `scripts/python/gen_tab6_v2.py` |

To regenerate from raw runs (instead of using the pre-aggregated CSVs), the full
pipeline is:

```
raw results/  ── build_stats_data_exp4.py ──>  per_instance_stats.csv
raw results/  ── build_convergence_data_ts.py ──>  convergence_data_ts.csv
per_instance_stats.csv  ── gen_tab6_v2.py ──>  Table 6 + supplementary
```

---

## 5. Software requirements

- **Solver:** FuzzyFW (C++14, GCC 13+). The compiled binary is **not** included
  in this dataset; contact the author for access to the solver source.
- **Analysis:**
    - Python 3.10+ with the standard library only (no extra dependencies for
      the included scripts).
    - R 4.5+ with `ggplot2`, `dplyr`, `patchwork`, `scales` for figure
      regeneration.
    - irace package for R (≥ 3.5) if you wish to re-run the tuning.

---

## 6. Hardware notes

All raw runs in `results/` were executed on an Intel Xeon E5-2680v4 workstation
(14 cores / 28 threads, 2.40 GHz, 16 GB RAM) running Windows 10 with a WSL2
Linux environment. Median runtimes in the paper should be interpreted accordingly
(see Section 8.1 of the paper for the hardware calibration of $\approx 2\times$
slower than the cluster used in the published comparators).

---

## 7. Related resources

- **Paper:** Computers & Operations Research, DOI \[paper DOI\].
- **Supplementary material PDF:** per-instance Taillard comparison tables,
  Zenodo DOI [10.5281/zenodo.20511981](https://doi.org/10.5281/zenodo.20511981).

---

## 8. License

This dataset is released under the **Creative Commons Attribution 4.0
International (CC-BY 4.0)** license. You are free to share and adapt the
material for any purpose, provided you give appropriate credit by citing the
accompanying paper:

```
@article{Diaz2026IJSP,
  author  = {Hern{\'a}n D{\'i}az Rodr{\'i}guez},
  title   = {Neighbourhood Structures and Ranking Operators for the
             Interval Job Shop Scheduling Problem},
  journal = {Computers \& Operations Research},
  year    = {2026},
  doi     = {[paper DOI]}
}
```

---

## 9. Contact

For questions about the dataset, the FuzzyFW solver, or the experimental
setup, contact Hernán Díaz Rodríguez at `diazhernan@uniovi.es`.
