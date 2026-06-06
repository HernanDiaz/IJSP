# IJSP — Interval Job Shop Scheduling Problem

Code, experimental pipeline and paper sources for research on the IJSP
(scheduling under interval-valued processing times).

## Repository layout

```
.
├── experiments/                    Analysis pipeline (general purpose)
│   ├── build_phaseB_stats.py        Aggregate per-run TS results
│   ├── build_convergence_data_ts.py Build per-generation profile
│   ├── gen_phaseB_tables.py         Friedman + Wilcoxon + LaTeX tables
│   ├── validate_phaseA.py           SGS verifier for HC results
│   ├── rebuild_supp_taillard_tables.py / patch_supp_taillard.py
│   ├── build_submission_zip.py / rebuild_dataset_zip.py
│   └── cor_tabu_2026/               *** Paper-specific setup + outputs ***
│       ├── setup/                     irace-tuned configs
│       ├── irace/                     irace tuning records
│       ├── statistical_results/       aggregated CSVs + LaTeX tables
│       └── README.md                  reproduction guide for the paper
│
├── Papers/
│   └── COR_Tabu/                    LaTeX source of the COR 2026 paper
│       ├── main.tex / main.pdf
│       ├── highlights.txt
│       ├── cover_letter.tex / .pdf
│       ├── references.bib
│       ├── figures/                  PDF figures + R scripts to regenerate
│       ├── data/phaseB_ts/           R-side analysis data (gitignored)
│       └── supplementary/            per-instance Taillard tables
│
└── (root-level FuzzyFW C++ source lives outside this repo at /home/diazhernan/IJSP/)
```

## Papers

| Paper | Year | Venue | Code tag |
|-------|------|-------|----------|
| Neighbourhood Structures and Ranking Operators for the IJSP | 2026 | Computers & Operations Research (submitted) | `cor-tabu-2026-submission` |

## Reproducing a published paper

Check out the tag for the paper of interest and follow the README in the
relevant `experiments/<paper_subdir>/` directory:

```bash
git checkout cor-tabu-2026-submission
cat experiments/cor_tabu_2026/README.md
```

## Generic analysis scripts

The scripts at the top level of `experiments/` are general-purpose tools
intended to be reused across future papers:

- `build_phaseB_stats.py` — aggregate per-(NB, instance, run) interval data
  from the raw FuzzyFW result CSVs into `runs_data.csv` and
  `per_instance_stats.csv`. Filters to the intersection of instances common
  to all tracked neighbourhoods.
- `build_convergence_data_ts.py` — per-generation convergence profile from
  the per-instance CSVs (used to produce the convergence figure).
- `gen_phaseB_tables.py` — Friedman + pairwise Wilcoxon signed-rank tests
  with Holm--Bonferroni correction, plus group-level RE table; emits
  LaTeX-ready tables.
- `validate_phaseA.py` — independent SGS verifier for HC neighbourhoods.
- `rebuild_supp_taillard_tables.py`, `patch_supp_taillard.py` — regenerate
  the per-instance supplementary LaTeX tables.
- `build_submission_zip.py` / `rebuild_dataset_zip.py` — package the LaTeX
  source / the experimental dataset for journal submission and Zenodo.

For paper-specific setups (irace parameters, neighbourhood configs, run
scripts) see the per-paper subdirectory under `experiments/`.

## Zenodo records

Datasets for individual papers are mirrored on Zenodo with versioned DOIs.
For the 2026 COR paper:

- Experimental dataset: https://doi.org/10.5281/zenodo.20562888
- Supplementary tables: https://doi.org/10.5281/zenodo.20562857
