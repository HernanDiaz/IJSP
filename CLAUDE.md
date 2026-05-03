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

### Umbrella headers and per-class `.cpp` files (one class per file)
Multi-class `.h` and `.cpp` files have been split into per-class files; the
original `.h` becomes a thin aggregator with only `#include` directives. The
original `.cpp` is deleted and replaced by one `.cpp` per class. `#pragma once`
in every header prevents circular-include issues.

Already split (`.h` + `.cpp`): `LocalSearch.h`, `Fitness.h`,
`NeighbourhoodIJSP_Cmax.h` / `NeighbourhoodIJSP_Cmax.cpp` (→ 8 files),
`EvaluationIJSP.h`, `CrossoverIJSP.h` / `CrossoverIJSP.cpp` (→ 5 files),
`EncoderIJSP.h`, `DecoderIJSP.h`, `CreationIJSP.h` / `CreationIJSP.cpp`
(→ 10 files, incl. new `CreationIJSP_Base.h` and `CreationIJSP_helpers.h`),
`Selection.h` / `Selection.cpp` (→ 13 files each, incl. new `Selection_Base.h`).

Rule: if a `.h` or `.cpp` contains more than one class, split it.

### SharedVarsEvolutionary
Struct threaded through all algorithm components carrying shared state:
`rng`, `encoder`, `decoder`, `parameters`. **Owns** `encoder` and `decoder`
— its destructor deletes them. `EvolutiveAlgorithm::clearAll()` nulls both
before re-initialisation to prevent double-delete on a second `prepareToRun()`.

### SGS (Schedule Generation Scheme)
Converts a task-ordering genotype (integer permutation) into a concrete
`ScheduleIJSP`. Each Decoder and Creation object owns one
`std::unique_ptr<SGS_IJSP> sgs`. Created via `IJSPClassRegister`.

`SGS_IJSP` provides a concrete `buildSchedule()` (template method): cast
problem → reset/create schedule → loop calling `scheduleTask()` → call
`postBuild()` → return. Subclasses only override `scheduleTask()` (required)
and optionally `postBuild()`. `SGS_IJSP_Insertion` overrides `postBuild()` to
call `schedule->verifyScheduling()`; `SGS_IJSP_Append` needs no overrides.

### Creation class hierarchy (IJSP)
`CreationRandomSchedule` is the base for all IJSP creation classes. It owns
`sgsLabel`, `sgs` (the SGS), `randomRatio`, and implements `shouldUseRandom()`.
Its `setup()` loads SGS and `randomRatio`. All 9 concrete classes (SRT, LRTF,
LRTFInverse, SNTF, SCTF, LCTF, SPJF, SPJFInverse, Manager) inherit from it
and only override `createIndividual()` (and `setup()` for Manager which also
sets up its embedded SPJFSchedule and LRTFSchedule). Calling
`CreationRandomSchedule::createIndividual(svars)` from a concrete class
produces a pure-random individual using the shared SGS.

### Crossover dispatch (IJSP)
`CrossoverIJSP_Base` (inherits `FuzzyFW::Crossover`) provides a single
`apply()` that dispatches to `applyPermutation()` or `applyJobPermutation()`
based on whether `svars->encoder` is an `EncoderIJSP_Order` or
`EncoderIJSP_JobOrder`. All 4 crossover classes (JOX, GOX, GPMX, PPX) inherit
`CrossoverIJSP_Base` and implement only the two typed variants.

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
| Raw owning pointers (encoder/decoder) | `SharedVarsEvolutionary` fields | Cannot use `unique_ptr` in header: `Encoder.h` → `SharedVarsEvolutionary.h` (circular). Fix: add `SharedVarsEvolutionary.cpp` to define destructor where types are complete. Currently safe: `clearAll()` nulls before destructor fires. |
| Delete-on-incomplete-type (UB) | `SharedVarsEvolutionary.h:56-57` | GCC warns: `delete encoder/decoder` on forward-declared types skips the real destructor. Same root cause as above; resolved by the `.cpp` split. |
| ~~Raw `NeighbourIJSP_Arc*` vector~~ | ~~`NB_ParallelBase_MakespanIJSP::neighbours`~~ | **Fixed** — `std::vector<std::unique_ptr<NeighbourIJSP_Arc>>`; bumped Makefile to C++14 |
| ~~Duplicated `buildSchedule()` in SGS_IJSP_Append and Insertion~~ | ~~`SGS_IJSP_*.cpp`~~ | **Fixed** — moved to `SGS_IJSP` base as template method; `postBuild()` hook for Insertion's `verifyScheduling()` |
| Raw owning pointers (operators) | `EvolutiveAlgorithm` `evaluator`; `GeneticAlgorithm` `creation`, `selection`, `crossover`, `mutation`, `replacement`, `bestSoFar`; `SharedVarsEvolutionary` `rng` | All managed via manual `delete` in `clearAll()`/destructors. Longer-term `unique_ptr` candidates; not urgent since patterns are correct. |
| `clone()` returns raw `T*` | All operators and problem classes | 20+ `clone()` methods return a raw pointer the caller must `delete`. Inconsistent with `unique_ptr<SGS_*>` introduced for `sgs`. Consider returning `unique_ptr<T>` or `shared_ptr<T>`. |
| ~~Dead `#include "EncoderFJSP.h"` in IJSP creation~~ | ~~`CreationIJSP.h`~~ | **Fixed** — replaced with `#include "Encoder.h"` (the real dependency) |
| ~~`EncoderIJSP_JobOrder` copy ctor cross-type typo~~ | ~~`EncoderIJSP_JobOrder.h`~~ | **Fixed** — ctor now takes `const EncoderIJSP_JobOrder&` |
| ~~Duplicated `setup()` / `randomRatio` check in 8 Creation classes~~ | ~~`CreationIJSP_*.h/.cpp`~~ | **Fixed** — pulled into `CreationRandomSchedule` base; `shouldUseRandom()` added |
| ~~Duplicated encoding dispatch in 4 Crossover `apply()` methods~~ | ~~`CrossoverIJSP.cpp`~~ | **Fixed** — extracted to `CrossoverIJSP_Base::apply()` |
| `NULL` vs `nullptr` | Throughout | C++11 codebase mixes both. `nullptr` is type-safe; `NULL` is a macro that can silently convert to `int`. Low risk, cosmetic. |

---

## Git / Session Notes

- Commits: `git commit` with `--author="HernanDiaz_ <hernan.diaz.rodriguez@gmail.com>"`
- Baseline for experiments lives in `experiments/statistical_results_exp7/` — do not modify
- Generated results go to `experiments/statistical_results/` — ignored by `.gitignore`
- Working branch: `feature/IJSP`
