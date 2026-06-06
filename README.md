# IJSP — Interval Job Shop Scheduling Problem

Code, experimental pipeline and paper sources for research on the IJSP
(scheduling under interval-valued processing times).

## Repository layout

```
.
├── experiments/
│   └── cor_tabu_2026/               *** Paper-specific package ***
│       ├── README.md                  reproduction guide
│       ├── scripts/                   analysis pipeline + utilities
│       ├── diagnostics/               bug-fix and PDF inspection helpers
│       ├── setup/                     irace-tuned configs
│       ├── irace/                     irace tuning records
│       ├── statistical_results/       aggregated CSVs + LaTeX tables
│       └── run_phaseB_tai100.sh       optional extension run script
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

## Per-paper organisation

Each paper has its own self-contained subdirectory under `experiments/`
holding everything tied to that paper: setup files, irace tuning records,
analysis scripts, aggregated results and diagnostic tooling. The scripts
in those subdirectories hard-code paths to this paper's data, so they are
kept paper-local rather than living in a shared `experiments/` root. When
starting a new paper, copy the relevant scripts from a previous paper's
`scripts/` directory and adapt their paths.

For the current COR 2026 paper, see
[`experiments/cor_tabu_2026/README.md`](experiments/cor_tabu_2026/README.md)
for the full reproduction guide.

## Zenodo records

Datasets for individual papers are mirrored on Zenodo with versioned DOIs.
For the 2026 COR paper:

- Experimental dataset: https://doi.org/10.5281/zenodo.20562888
- Supplementary tables: https://doi.org/10.5281/zenodo.20562857
