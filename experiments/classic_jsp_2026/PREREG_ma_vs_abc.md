# Pre-registration: memetic algorithm against ABCPSO on the open Taillard instances

Written and committed before any run of this experiment was started. Nothing
below changes once the runs begin; anything learned on the way is a hypothesis
for the next experiment, not an amendment to this one.

## Why this experiment, and why first

The goal of this line is now to improve a best known solution on the Taillard
benchmark. That is only possible on the 22 open instances, and on those the
current configuration sits 3-5 % above the best known solutions (README,
"The open instances"). The journal's finding is that this gap did not move under
three changes to the local search, and that the population collapses within a
few dozen generations: the binding constraint is the population-level algorithm,
not the local search.

The memetic algorithm is the other population-level algorithm in this framework
that drives the same local search. While checking the in-place N2 evaluation
under it (JOURNAL, 2026-09-19), it was seen to complete about twice as many
generations per minute as the ABC on the same build, and at a 60 s budget on
`ta01`-`ta10` to reach `ta06 = 1238` and `ta05 = 1231`, which no ABC
configuration in this directory has. That was ten instances, ten runs, noticed
on the way to something else, and this directory's own rule is that such a
thing is a hypothesis to be tested on instances that did not generate it. The
open instances did not generate it.

## Disclosure: what was looked at before this was fixed

To check that the two diversity statistics work (the Hamming one had a bug
that made it report zero for every population; fixed in the same commit), one
diagnostic run per arm was made on `ta01` at 60 s and on `ta41` at 300 s, seed
1, before this document was committed. `ta41` is one of the 22 instances below.
In that single run the memetic reached 2102 against the ABC's 2168, and was
still improving at generation 400 where the ABC had stopped at 75. That is one
run on one instance and is not evidence for H1; it is recorded here so that it
cannot later be mistaken for an independent confirmation. The collapse curves
from those runs are in the journal entry of the same date.

## Hypothesis

H1: at an equal wall-clock budget, the memetic algorithm with plain tabu search
on N2 reaches better makespans than ABCPSO with the same local search on the
open Taillard instances.

H0: it does not.

## Design, fixed in advance

**Instances.** All 22 open Taillard instances, from `taillard_bounds.csv`:
`ta18`, `ta22`, `ta23`, `ta25`, `ta26`, `ta27`, `ta29`, `ta30`, `ta32`,
`ta33`, `ta34`, `ta40`, `ta41`-`ta50`. No instance is added or dropped after
the fact.

**Runs.** 10 per instance per arm, seeds 1 to 10 (`seed = 1`, `runs = 10`;
run r uses seed 1 + r as `EvoLauncher` does).

**Budget.** 300 s of wall clock per run, the budget of every open-instance run
in this directory so far. Convergence data says 30x20 wants more; that is a
separate question and is not mixed into this one.

**Arms.** `setup/prereg_abc_300s.txt` and `setup/prereg_ma_300s.txt`. They are
`open_baseline_300s.txt` -- ABCPSO, JOX crossover, insertion mutation, plain
tabu search on N2 with 20 bad iterations, population 250 -- with `runs = 10`
and two diversity statistics added, and they differ in exactly one line:
`algorithm = ABCPSO` against `algorithm = MEMETIC`. The memetic reads the same
keys and ignores the ABC-only ones (`maxnumtrials`, `elite.*`).

**Execution.** Both arms at the same time on the same instances, seven per arm
at a time, through `scripts/paired_setups.sh`. The machine drifts by about 9 %
between batches, and a wall-clock budget turns that into a difference in
search; running the arms together removes it.

**Cost.** 22 x 2 x 10 x 300 s = 36.7 CPU-hours; about 3.3 hours of wall clock on
14 cores.

## Analysis, fixed in advance

Everything is recomputed from certificates by `scripts/compare.py`, which
re-verifies each schedule against the published OR-Library data.

**Primary endpoint.** Per-instance mean makespan over the 10 runs, compared
across the 22 instances with the Wilcoxon signed-rank test, paired by instance.

**Secondary endpoints, reported alongside.** Best-of-runs per instance with the
sign test; mean best gap to the lower bound per arm; and, as description only,
the per-instance differences split by size group (20x15, 20x20, 30x15, 30x20).
No test is run on a subgroup, and no subgroup result is a finding of this
experiment.

**Decision rule.** If the primary endpoint separates the arms at p < 0.05, the
winner becomes the base algorithm for the attempts on the best known solutions.
If it does not, ABCPSO stays, because it is the incumbent, and the memetic is
not carried forward on the strength of any subgroup or of the secondary
endpoints alone.

**Also recorded, not tested.** The two diversity statistics per generation
(`hamming`: mean pairwise Hamming distance between genotypes, normalised;
`neri`: `1 - (avg - best) / (worst - best)` on the fitness) give the collapse
curve for each arm. They are there so that whatever the result, the next
experiment on diversity starts from a measurement.

## What would make this experiment worthless

Changing the budget, the run count or the instance set once started; reading
the 30x20 group before the rest; adding a third arm afterwards; comparing
against the older 5-run baselines instead of against the arm run alongside.
The journal records two dead ends and one retraction that came from exactly
these moves.
