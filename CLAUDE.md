# CLAUDE.md — JSP Project Guide (branch `experiment/classic-jsp-crisp`)

## Overview

Research codebase implementing evolutionary algorithms for the **classic job
shop scheduling problem** on crisp integer processing times, built on a generic
EA framework (`FuzzyFW`). This branch is the crisp refactor of the interval job
shop code on `feature/JSP`: the `Interval` type became `Crisp` (one `int`), the
interval ranking machinery and robustness analysers were removed, the fuzzy job
shop (FJSP) and fuzzy VRP (FVRP) domains were pruned, and every `JSP` name
became `JSP`. The interval and fuzzy code lives on in `feature/JSP` and
`experiment/classic-jsp`; nothing here is expected to merge back into them.

**Entry point:** `main.cpp` → `EvoLauncher` → algorithm → problem → results CSV

Usage:
```
./FuzzyFW <setup_file> <instance_file> [log_folder]
```
Build (Linux/WSL): `make -j14` in project directory → produces `../FuzzyFW`. Delete stale `*.d` files first after renaming or removing sources: the Makefile includes them before any rule runs.

---

## Namespaces

| Namespace | Role |
|-----------|------|
| `FuzzyFW` | Generic EA framework: abstract base classes for Algorithm, Individual, Fitness, Operator, Statistics |
| `JSP` | Job shop domain: Problem, Schedule, SGS, Encoder, Decoder, Creation, Evaluation, Crossover, Neighbourhood |
| `PostExecution` | Post-run analysers; on this branch only `jsp.certificate`, which writes a schedule `verify_certificate.py` can check |

---

## Key Architectural Patterns

### ClassRegister (string → object factory)
Maps string names from config files to factory functions. Call `registerClasses()`
once at startup, then `getXxxObject("name")` to instantiate. Files:
- `AlgorithmClassRegister` — algorithm names (GA, MA, ABC, ABCT, …) → `EvolutiveAlgorithm*`
- `JSPClassRegister` — SGS names (insertion, append, …) → `SGS_JSP*`
- `EvaluationClassRegister`, `StatisticsClassRegister`, `LocalSearchClassRegister`
- `PostExecutionClassRegister` — post-run analyzers

**Adding a new class:** include its `.h` in the register file, add one line in
`registerClasses()`. No other file needs changing.

### Umbrella headers and per-class `.cpp` files (one class per file)
Multi-class `.h` and `.cpp` files have been split into per-class files; the
original `.h` becomes a thin aggregator with only `#include` directives. The
original `.cpp` is deleted and replaced by one `.cpp` per class. `#pragma once`
in every header prevents circular-include issues.

Already split (`.h` + `.cpp`): `LocalSearch.h`, `Fitness.h`,
`NeighbourhoodJSP_Cmax.h` / `NeighbourhoodJSP_Cmax.cpp` (→ 8 files),
`EvaluationJSP.h`, `CrossoverJSP.h` / `CrossoverJSP.cpp` (→ 5 files),
`EncoderJSP.h`, `DecoderJSP.h`, `CreationJSP.h` / `CreationJSP.cpp`
(→ 10 files, incl. new `CreationJSP_Base.h` and `CreationJSP_helpers.h`),
`Selection.h` / `Selection.cpp` (→ 13 files each, incl. new `Selection_Base.h`).

Rule: if a `.h` or `.cpp` contains more than one class, split it.

### SharedVarsEvolutionary
Struct threaded through all algorithm components carrying shared state:
`rng`, `encoder`, `decoder`, `parameters`. **Owns** `encoder` and `decoder`
— its destructor deletes them. `EvolutiveAlgorithm::clearAll()` nulls both
before re-initialisation to prevent double-delete on a second `prepareToRun()`.

### SGS (Schedule Generation Scheme)
Converts a task-ordering genotype (integer permutation) into a concrete
`ScheduleJSP`. Each Decoder and Creation object owns one
`std::unique_ptr<SGS_JSP> sgs`. Created via `JSPClassRegister`.

`SGS_JSP` provides a concrete `buildSchedule()` (template method): cast
problem → reset/create schedule → loop calling `scheduleTask()` → call
`postBuild()` → return. Subclasses only override `scheduleTask()` (required)
and optionally `postBuild()`. `SGS_JSP_Insertion` overrides `postBuild()` to
call `schedule->verifyScheduling()`; `SGS_JSP_Append` needs no overrides.

### Creation class hierarchy (JSP)
`CreationRandomSchedule` is the base for all JSP creation classes. It owns
`sgsLabel`, `sgs` (the SGS), `randomRatio`, and implements `shouldUseRandom()`.
Its `setup()` loads SGS and `randomRatio`. All 9 concrete classes (SRT, LRTF,
LRTFInverse, SNTF, SCTF, LCTF, SPJF, SPJFInverse, Manager) inherit from it
and only override `createIndividual()` (and `setup()` for Manager which also
sets up its embedded SPJFSchedule and LRTFSchedule). Calling
`CreationRandomSchedule::createIndividual(svars)` from a concrete class
produces a pure-random individual using the shared SGS.

### Crossover dispatch (JSP)
`CrossoverJSP_Base` (inherits `FuzzyFW::Crossover`) provides a single
`apply()` that dispatches to `applyPermutation()` or `applyJobPermutation()`
based on whether `svars->encoder` is an `EncoderJSP_Order` or
`EncoderJSP_JobOrder`. All 4 crossover classes (JOX, GOX, GPMX, PPX) inherit
`CrossoverJSP_Base` and implement only the two typed variants.

`CrossoverJSP_Base` also provides a protected static helper
`buildTaskGenotype(ind, prob)` that converts a job-order individual (with
repetitions) into a task-order vector using `prob->getTaskId()`. GOX, GPMX,
and PPX call it at the start of `applyJobPermutation()` to avoid the repeated
15-line preamble.

---

## File Map (key files)

| File | Purpose |
|------|---------|
| `heading.h` | Global includes, macros (`Infd`, `AccuracyError`), platform helpers (`makeDir`, `FSEP`), utility templates |
| `EvolutiveAlgorithm.h/.cpp` | Abstract base for all algorithms; `prepareToRun()`, `clearAll()` |
| `EvoLauncher.h/.cpp` | Orchestrates N runs, writes CSV output, calls `PostExecutionManager` |
| `AlgorithmClassRegister.h/.cpp` | Factory for algorithm objects |
| `JSPClassRegister.h` | Factory for SGS objects |
| `SharedVarsEvolutionary.h` | Shared-state container (rng, encoder, decoder, params) |
| `ProblemJSP.h/.cpp` | Loads JSP instance file |
| `ScheduleJSP.h/.cpp` | Solution representation |
| `CrispTime.h/.cpp` | `Crisp`, the core time type: one `int`, all comparisons inline. Replaces `Interval` on `experiment/classic-jsp-crisp` |
| `LS_TabuBackJump.h/.cpp` | Tabu search with back-jump tracking (`tabu-backjump`, alias `tsab`). Added on `experiment/classic-jsp` |
| `JSPCertificateAnalyzer.h/.cpp` | Post-execution analyzer writing a verifiable schedule (`jsp.certificate`). Added on `experiment/classic-jsp` |

---

## Verification after refactoring

There is no standing regression check. `experiments/verify_refactor.sh` and its
baseline `experiments/statistical_results_exp7/` were written for one specific
refactoring campaign and removed with it in the cleanup commits `493a3be` and
`1fb4ae4`; they are not expected back.

So changes to an operator or to a register are verified by whatever the change
itself calls for -- a targeted comparison against the configuration being
changed, rather than a fixed suite. `experiments/classic_jsp_2026/` is an
example: it re-verifies every schedule it produces against the published
instance data, so a regression shows up as an infeasible or worse result rather
than going unnoticed.

---

## Known Technical Debt

| Issue | Location | Risk |
|-------|----------|------|
| ~~`-fpermissive` flag~~ | ~~`Makefile`~~ | **Fixed** — 14 rvalue errors patched in `CreationJSP.cpp` |
| ~~`throw new FuzzyFWException(...)`~~ | ~~Throughout~~ | **Fixed** — all 470 throw-by-pointer converted to throw-by-value |
| ~~Raw owning pointers (SGS)~~ | ~~Decoder/Creation `sgs` fields~~ | **Fixed** — converted to `std::unique_ptr<SGS_*>`; fixes memory leaks in all Creation classes |
| Raw owning pointers (encoder/decoder) | `SharedVarsEvolutionary` fields | Cannot use `unique_ptr` in header: `Encoder.h` → `SharedVarsEvolutionary.h` (circular). Fix: add `SharedVarsEvolutionary.cpp` to define destructor where types are complete. Currently safe: `clearAll()` nulls before destructor fires. |
| Delete-on-incomplete-type (UB) | `SharedVarsEvolutionary.h:56-57` | GCC warns: `delete encoder/decoder` on forward-declared types skips the real destructor. Same root cause as above; resolved by the `.cpp` split. |
| ~~Raw `NeighbourJSP_Arc*` vector~~ | ~~`NB_ParallelBase_MakespanJSP::neighbours`~~ | **Fixed** — `std::vector<std::unique_ptr<NeighbourJSP_Arc>>`; bumped Makefile to C++14 |
| ~~Duplicated `buildSchedule()` in SGS_JSP_Append and Insertion~~ | ~~`SGS_JSP_*.cpp`~~ | **Fixed** — moved to `SGS_JSP` base as template method; `postBuild()` hook for Insertion's `verifyScheduling()` |
| Raw owning pointers (operators) | `EvolutiveAlgorithm` `evaluator`; `GeneticAlgorithm` `creation`, `selection`, `crossover`, `mutation`, `replacement`, `bestSoFar`; `SharedVarsEvolutionary` `rng` | All managed via manual `delete` in `clearAll()`/destructors. Longer-term `unique_ptr` candidates; not urgent since patterns are correct. |
| `clone()` returns raw `T*` | All operators and problem classes | 20+ `clone()` methods return a raw pointer the caller must `delete`. Inconsistent with `unique_ptr<SGS_*>` introduced for `sgs`. Consider returning `unique_ptr<T>` or `shared_ptr<T>`. |
| ~~Dead `#include "EncoderFJSP.h"` in JSP creation~~ | ~~`CreationJSP.h`~~ | **Fixed** — replaced with `#include "Encoder.h"` (the real dependency) |
| ~~`EncoderJSP_JobOrder` copy ctor cross-type typo~~ | ~~`EncoderJSP_JobOrder.h`~~ | **Fixed** — ctor now takes `const EncoderJSP_JobOrder&` |
| ~~Duplicated `setup()` / `randomRatio` check in 8 Creation classes~~ | ~~`CreationJSP_*.h/.cpp`~~ | **Fixed** — pulled into `CreationRandomSchedule` base; `shouldUseRandom()` added |
| ~~Duplicated encoding dispatch in 4 Crossover `apply()` methods~~ | ~~`CrossoverJSP.cpp`~~ | **Fixed** — extracted to `CrossoverJSP_Base::apply()` |
| ~~Duplicated job→task preamble in `applyJobPermutation` of GOX/GPMX/PPX~~ | ~~`CrossoverJSP_*.cpp`~~ | **Fixed** — extracted to `CrossoverJSP_Base::buildTaskGenotype()`; also removed dead `std::cout << "Stop"` from GPMX |
| `NULL` vs `nullptr` | Throughout | C++11 codebase mixes both. `nullptr` is type-safe; `NULL` is a macro that can silently convert to `int`. Low risk, cosmetic. |
| Near-identical `tipo` blocks in `estimateHeadsTails` | `NeighbourhoodJSP_N3.cpp` | 4 blocks differ only by arc direction constant. Could extract to a private helper taking `tipo`. Low priority. |
| Repeated init block in 3 constructors | `ProblemJSP.cpp` | All 3 ctors call the same 5-line member init. Extract to a private `initMembers()`. Low priority. |
| Duplicated print loops in `EvoLauncher` | `EvoLauncher.cpp` | `printRuntimes()` and `printStatistics()` share the same CSV-writing loop structure. Could merge with a lambda or template. Low priority. |
| `evaluateNeighbour` returns `nullptr` as infeasibility signal | `NeighbourhoodJSP_*.cpp` | Using `nullptr` return to mean "infeasible" conflates error and domain state. Would be cleaner with `std::optional<ScheduleJSP*>` or a bool out-param. Medium priority. |

---

## Git / Session Notes

- Commits: `git commit` with `--author="HernanDiaz_ <hernan.diaz.rodriguez@gmail.com>"`
- Generated results go to `experiments/statistical_results/` — ignored by `.gitignore`
- `feature/JSP` — the interval job shop code this branch was refactored from
- `experiment/classic-jsp` — classic (crisp) JSP on the Taillard instances with
  the interval solver unchanged; see `experiments/classic_jsp_2026/README.md`
- `experiment/classic-jsp-crisp` — this branch. Verify any change to the solver
  the way `experiments/classic_jsp_2026/JOURNAL.md` does: same seed, compare the
  search trace generation by generation against `experiment/classic-jsp`, and
  measure speed only with `scripts/paired_compare.sh`
