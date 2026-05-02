# CLAUDE.md — IJSP Project Guide

## Overview

Research codebase implementing evolutionary algorithms for the **Interval Job Shop
Problem (IJSP)**: scheduling jobs with fuzzy/interval processing times to minimize
makespan or tardiness. Built on a generic EA framework (`FuzzyFW`) that also
supports FJSP and FVRP problems.

**Entry point:** `main.cpp` → `EvoLauncher` → algorithm → problem → results CSV

Usage:
```
./FuzzyFW <setup_file> <instance_file> [log_folder]
```
Build (Linux/WSL): `make -j4` in project directory → produces `../FuzzyFW`

---

## Namespaces

| Namespace | Role |
|-----------|------|
| `FuzzyFW` | Generic EA framework: abstract base classes for Algorithm, Individual, Fitness, Operator, Statistics |
| `IJSP` | Interval JSP domain: Problem, Schedule, SGS, Encoder, Decoder, Creation, Evaluation, Crossover, Neighbourhood |
| `PostExecution` | Post-run robustness analysis tools |

---

## Key Architectural Patterns

### ClassRegister (string → object factory)
Maps string names from config files to factory functions. Call `registerClasses()`
once at startup, then `getXxxObject("name")` to instantiate. Files:
- `AlgorithmClassRegister` — algorithm names (GA, MA, ABC, ABCT, …) → `EvolutiveAlgorithm*`
- `IJSPClassRegister` — SGS names (insertion, append, …) → `SGS_IJSP*`
- `EvaluationClassRegister`, `StatisticsClassRegister`, `LocalSearchClassRegister`

**Adding a new class:** include its `.h` in the register file, add one line in
`registerClasses()`. No other file needs changing.

### Umbrella headers (one class per file)
Multi-class `.h` files have been split into per-class files; the original `.h`
becomes a thin aggregator with only `#include` directives. `#pragma once` in
every file prevents circular-include issues.

Already split: `LocalSearch.h`, `Fitness.h`, `NeighbourhoodIJSP_Cmax.h`,
`EvaluationIJSP.h`, `CrossoverIJSP.h`, `EncoderIJSP.h`, `DecoderIJSP.h`,
`CreationIJSP.h`.

Rule: if a `.h` contains more than one class, split it.

### SharedVarsEvolutionary
Struct threaded through all algorithm components carrying shared state:
`rng`, `encoder`, `decoder`, `parameters`. **Owns** `encoder` and `decoder`
— its destructor deletes them. `EvolutiveAlgorithm::clearAll()` nulls both
before re-initialisation to prevent double-delete on a second `prepareToRun()`.

### SGS (Schedule Generation Scheme)
Converts a task-ordering genotype (integer permutation) into a concrete
`ScheduleIJSP`. Each Decoder, Creation, and Evaluation object owns one
`SGS_IJSP *sgs` and deletes it in its destructor. Created via `IJSPClassRegister`.

---

## File Map (key files)

| File | Purpose |
|------|---------|
| `heading.h` | Global includes, macros (`Infd`, `AccuracyError`), platform helpers (`makeDir`, `FSEP`), utility templates |
| `EvolutiveAlgorithm.h/.cpp` | Abstract base for all algorithms; `prepareToRun()`, `clearAll()` |
| `EvoLauncher.h/.cpp` | Orchestrates N runs, writes CSV output, calls `PostExecutionManager` |
| `AlgorithmClassRegister.h/.cpp` | Factory for algorithm objects |
| `IJSPClassRegister.h` | Factory for SGS objects |
| `SharedVarsEvolutionary.h` | Shared-state container (rng, encoder, decoder, params) |
| `ProblemIJSP.h/.cpp` | Loads IJSP instance file |
| `ScheduleIJSP.h/.cpp` | Solution representation |
| `Interval.h` | Interval arithmetic (core math type) |

---

## Verification after refactoring

```bash
bash experiments/verify_refactor.sh
```
Runs 6 instances × 5 configs (30 parallel jobs) against baseline in
`experiments/statistical_results_exp7/runs_data.csv`. Exit 0 = all match.
**Always run before committing changes to operators or their registration.**

---

## Known Technical Debt

| Issue | Location | Risk |
|-------|----------|------|
| ~~`-fpermissive` flag~~ | ~~`Makefile`~~ | **Fixed** — 14 rvalue errors patched in `CreationIJSP.cpp` |
| ~~`throw new FuzzyFWException(...)`~~ | ~~Throughout~~ | **Fixed** — all 470 throw-by-pointer converted to throw-by-value |
| ~~Raw owning pointers (SGS)~~ | ~~Decoder/Creation `sgs` fields~~ | **Fixed** — converted to `std::unique_ptr<SGS_*>`; fixes memory leaks in all Creation classes |
| Raw owning pointers (encoder/decoder) | `SharedVarsEvolutionary` fields | Cannot use `unique_ptr`: `Encoder.h` includes `SharedVarsEvolutionary.h` (circular). Needs a new `SharedVarsEvolutionary.cpp` to define the destructor out-of-line. Currently safe: `clearAll()` nulls before destructor fires. |
| `EncoderIJSP_JobOrder` copy ctor takes `const EncoderIJSP_Order&` | `EncoderIJSP_JobOrder.h` | Cross-type ctor, intent unclear |

---

## Git / Session Notes

- Commits: `git commit` with `--author="HernanDiaz_ <hernan.diaz.rodriguez@gmail.com>"`
- Baseline for experiments lives in `experiments/statistical_results_exp7/` — do not modify
- Generated results go to `experiments/statistical_results/` — ignored by `.gitignore`
- Working branch: `feature/IJSP`
