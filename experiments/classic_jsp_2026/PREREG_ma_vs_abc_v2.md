# Pre-registration: memetic algorithm against ABCPSO, each with its own tuned configuration

> Replaces `PREREG_ma_vs_abc.md`, withdrawn on 2026-09-19 because both arms ran
> the configuration irace had tuned for the ABC. That compared a tuned ABC
> against an untuned memetic. This design gives each algorithm the parameters
> tuned for it, on training instances neither the comparison nor the record
> attempts ever use.
>
> **Status: complete and not started.** Both tunings have finished and both
> configurations are written down below. This document is committed in full
> before the first comparison run; nothing in it changes after that.

## Why this experiment, and why first

The goal of this line is to improve a best known solution on the Taillard
benchmark. That is only possible on the 22 open instances, and on those the
configuration used so far sits 3-5 % above the best known solutions (README,
"The open instances"). The journal's finding is that this gap did not move
under three separate changes to the local search, and that the population
collapses within a few dozen generations: the binding constraint is the
population-level algorithm, not the local search.

The memetic algorithm is the other population-level algorithm in this
framework that drives the same tabu search on N2. Before the winner can be
pointed at the open instances, it has to be decided which one it is.

## Disclosure: what was seen before this was written

1. While checking the in-place N2 evaluation (JOURNAL, 2026-09-19), the
   memetic completed about twice as many generations per minute as the ABC on
   the same build, and at a 60 s budget on `ta01`-`ta10` reached `ta06 = 1238`
   and `ta05 = 1231`, which no ABC configuration in this directory has. Ten
   instances, ten runs, noticed on the way to something else.
2. One diagnostic run per arm, to check the two diversity statistics, on
   `ta01` at 60 s and on `ta41` at 300 s, seed 1. `ta41` is one of the 22
   instances below. In that single run the memetic reached 2102 against the
   ABC's 2168 and was still improving at generation 400, where the ABC had
   stopped at 75.
3. The 22 open instances have never been run under either tuned
   configuration.

Points 1 and 2 are where the hypothesis comes from. They are recorded here so
that they cannot later be counted as confirmation of it.

## Hypothesis

**H1**: at an equal wall-clock budget, each algorithm running the
configuration tuned for it, the memetic algorithm with tabu search on N2
reaches better makespans than ABCPSO with the same local search on the open
Taillard instances.

**H0**: it does not.

## The two configurations, and where they come from

Each arm was tuned separately with irace 4.5, same budget, same training
instances, same seed, one arm at a time on an otherwise idle machine:
1500 runs of 300 s per arm, Friedman racing, seed 20260919.

**The training instances are not Taillard instances.** They are ten instances
generated from Taillard's own distribution (durations uniform on [1, 99], a
random machine permutation per job; `scripts/generate_taillard.py`) in the two
size classes where the open instances have room to move: 20x20, where 7 of the
10 Taillard instances are open with a mean BKS-LB gap of 2.9 %, and 30x20,
where all 10 are open at 2.7 %. The 20x15 and 30x15 classes sit at 0.5-1.4 %.
Tuning on generated instances rather than on the 22 keeps the comparison and
the later record attempts free of a configuration fitted to them, and is what
makes the result about the algorithms rather than about the tuning set.

The 20x15 and 30x15 open instances are in the comparison but were not tuned
for. That is a deliberate asymmetry, identical for both arms, and it is
declared rather than hidden: it is part of what "one configuration for the
whole benchmark" means.

**Fixed in both arms, not tuned**: the job-order encoding, the insertion SGS,
the N2 neighbourhood with heads-and-tails estimation, Lamarckian evaluation,
the local search at every generation (`localsearch.frequency = period`,
`period = 1`), the 300 s budget, and `noimprovement = 100000` so that only the
clock stops a run.

### Arm A — ABCPSO, configuration 136

Tuning finished 2026-09-20 04:49, 14.2 h of wall clock, 123 h of CPU.

| parameter | value |
|---|---|
| `population.size` | 247 |
| `elite.size` | 86 |
| `elite.selection` | 1 |
| `maxnumtrials` | 35 |
| `crossover` | `jsp.jox` |
| `crossover.probability` | 0.982 |
| `mutation` | `swap` |
| `mutation.probability` | 0.1801 |
| `localsearch.target` | 0.4645 |
| `localsearch.bad-iterations` | 15 |

The three surviving elites agree on `jsp.jox`, `swap`, a crossover probability
of 0.98-1.0, `elite.selection = 1` — the lowest selective pressure in the
space — and a short tabu search of 10-23 bad iterations. Setup:
`setup/prereg2_abc_300s.txt`.

### Arm B — memetic algorithm, configuration 164

Tuning finished 2026-09-20 16:52, 12.1 h of wall clock, 124 h of CPU, on the
same machine with nothing else on it.

| parameter | value |
|---|---|
| `population.size` | 99 |
| `crossover` | `jsp.jox` |
| `crossover.probability` | 0.9434 |
| `mutation` | `inversion` |
| `mutation.probability` | 0.0348 |
| `selection` | `shuffle` |
| `replacement` | `tournament` |
| `localsearch.target` | 0.7186 |
| `localsearch.bad-iterations` | 23 |

The three surviving elites agree on `jsp.jox`, `inversion`, `shuffle`
selection, `tournament` replacement and a tabu search of 20-26 bad iterations,
and all three sit at a **small population, 99 to 152**, against the ABC's 247
to 292. The two algorithms were therefore tuned to opposite regimes — the ABC
to a large population with the weakest selective pressure the space allows,
the memetic to a small one — which is the kind of difference a shared
configuration would have hidden, and is the reason the withdrawn version of
this document was worthless.

The parameter space is `tuning/parameters_ma.txt`, fixed on 2026-09-19;
`generational` and `simple` replacement are excluded because they abort the
memetic, a pre-existing framework bug recorded in the journal. Setup:
`setup/prereg2_ma_300s.txt`.

### What the tuning does not say

irace reports a mean cost for the configuration it selected — 1762.43 for the
ABC, 1803.59 for the memetic. **These two numbers are not comparable and are
not evidence about H1.** Each is a mean over the instance-seed pairs that
configuration happened to survive on inside its own race, a different set of
different size for each arm, and racing keeps a configuration alive on the
runs where it does well. The comparison below is the only thing that speaks to
H1.

Both setups are generated from the tuning logs by
`scripts/make_arm_setups.sh`, which reads irace's own "Best configurations as
commandlines" line and substitutes it into the arm's tuning template. Nothing
is typed in by hand, so the setup that runs and the tuning that produced it
cannot disagree.

## Design, fixed in advance

**Instances.** All 22 open Taillard instances, from `taillard_bounds.csv`:
`ta18`, `ta22`, `ta23`, `ta25`, `ta26`, `ta27`, `ta29`, `ta30`, `ta32`,
`ta33`, `ta34`, `ta40`, `ta41`-`ta50`. None is added or dropped afterwards.

**Runs.** 10 per instance per arm, `seed = 1`, `runs = 10`.

**Budget.** 300 s of wall clock per run, the budget the configurations were
tuned at and the budget of every open-instance run in this directory so far.
Convergence data says 30x20 wants more; that is a separate question and is not
mixed into this one.

**Execution.** One queue, `scripts/sequential_compare.sh`, holding all 44 jobs
of both arms and alternating between them, at most 14 running at a time — one
solver per core, never two batches stacked on the machine. The script refuses
to start while any other solver or tuning is running. Running the arms one
after the other would confound each arm with when it ran, because a wall-clock
budget turns this machine's ~9 % drift between batches into a difference in
how much search each arm got; alternating them through one queue spreads both
arms evenly over the same window without ever asking the machine for more than
it has cores.

**Cost.** 22 x 2 x 10 x 300 s = 36.7 CPU-hours; about 3 hours of wall clock on
14 cores.

## Analysis, fixed in advance

Everything is recomputed from the schedules the solver writes, by
`scripts/compare.py`, which re-verifies each schedule against the published
OR-Library data: a run is scored on the makespan its own schedule supports,
not on a number it reports.

**Primary endpoint.** Per-instance mean makespan over the 10 runs, compared
across the 22 instances with the Wilcoxon signed-rank test, paired by
instance, two-sided, alpha = 0.05.

**Secondary endpoints, reported alongside and not used to decide.**
Best-of-runs per instance with the sign test; mean gap to the lower bound per
arm; and, as description only, the per-instance differences split by size
group (20x15, 20x20, 30x15, 30x20). No test is run on a subgroup and no
subgroup result is a finding of this experiment.

**Decision rule.** If the primary endpoint separates the arms at p < 0.05, the
arm with the lower mean rank becomes the base algorithm for the attempts on
the best known solutions. If it does not, ABCPSO stays, because it is the
incumbent; the memetic is not carried forward on a subgroup or on the
secondary endpoints alone. A p-value says the arms differ, not which is
better: the direction comes from the ranks, and is reported as such.

**Also recorded, not tested.** The two diversity statistics per generation
(`hamming`, mean pairwise distance between genotypes, normalised; `neri`,
`1 - (avg - best) / (worst - best)` on the fitness), for both arms on every
run. Whatever the result, the next experiment on diversity then starts from a
measurement rather than from the two diagnostic runs.

## What comes after, and what it is not

The winner is then run on the 22 open instances at a longer budget to try to
beat a best known solution. Tuning on the target instances is legitimate there
and will be declared: that is a record attempt, not a comparison, and a better
solution is proved by the schedule itself, which anyone can verify against the
published data. This experiment is what decides which algorithm gets that
budget, and its result does not transfer to the record attempts as a claim
about anything else.

## What would make this experiment worthless

Changing the budget, the run count or the instance set once started; reading
the 30x20 group before the rest; adding a third arm afterwards; comparing
against the older 5-run baselines instead of against the arm run alongside;
or letting either arm run while something else is on the machine. The journal
records two dead ends and one retraction that came from exactly these moves.
