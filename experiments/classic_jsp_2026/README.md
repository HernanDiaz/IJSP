# Classic JSP on the Taillard instances

This directory holds a line of work on the **classic (crisp) job shop problem**,
as opposed to the interval problem (IJSP) the rest of the repository targets.
The goal is to close the gap to the published lower bounds of the Taillard
instances, and to make every reported makespan checkable by someone who does
not run this code.

## Classic JSP without a new problem domain

A classic JSP instance is an IJSP instance whose processing times are all
degenerate intervals `[p, p]`.

`FuzzyFW::Interval` stores two endpoints `a` and `b`. When `a == b`:

* every ranking method (`Component`, `EV`, `Sakawa`, `Jiang`, `Lex1`, `Lex2`,
  `YX`) reduces to the order on the reals, because they all agree on points;
* the component-wise maximum is the ordinary maximum;
* addition is ordinary addition.

So the existing solver optimises the classic makespan exactly, and no new
Problem, Schedule, SGS, Encoder or Decoder is needed. The cost is that the
arithmetic runs on two `double` components instead of one integer; a crisp
specialisation would be faster, but it would also be a second implementation to
keep correct, so it is not worth it until speed is shown to be the binding
constraint.

**Sanity check.** The converted `ta01` reproduces the machine routes of
`SelectosYTaillardIntervalos/tai15_15_01.F.15_01.txt`, and its processing times
are the centres of that instance's intervals — the interval benchmark was built
from the same Taillard data.

## What "beating the lower bound" can mean

A feasible schedule can never have a makespan *below* a valid lower bound. For
the Taillard instances the published bounds are valid, so:

* on the **58 closed instances** (`LB == BKS`, including all of `ta01`-`ta10`),
  the best attainable result is to *match* the bound, which proves optimality;
* only the **22 open instances** (listed as `open` in `taillard_bounds.csv`,
  from `ta18` up to `ta50`) leave room to improve the best known solution.

A run that reports a makespan below the lower bound has found a bug, not a
result. `verify_certificate.py` reports that case as an anomaly.

## Verifiable results

`JSPCertificateAnalyzer` (selected with `postexecution.analyzer = jsp.certificate`)
writes, for every solution returned, one row per task with its job, position in
the job, machine, start time, duration and completion time.

`verify_certificate.py` re-checks such a certificate against the **published
OR-Library instance data** in `reference/taillard_orlib.txt`, not against the
converted instance file, so a conversion bug cannot hide. It verifies that

1. every task appears exactly once,
2. machine routes and durations match the published instance,
3. operations of a job run in order and do not overlap,
4. operations on a machine do not overlap,
5. nothing starts before time zero,
6. the makespan is the largest completion time,

and only then compares against the bounds. Moving a start time, shortening a
duration, changing a machine or deleting a task are all rejected.

## Layout

| path | what it is |
|---|---|
| `scripts/convert_orlib.py` | OR-Library / Taillard data to instance files in `../../TaillardJSP` |
| `scripts/verify_certificate.py` | independent feasibility and makespan checker |
| `scripts/summarize.py` | results table, recomputed from certificates |
| `scripts/run_jsp.sh` | run one setup over a list of instances, then verify everything |
| `setup/` | solver configurations |
| `reference/taillard_orlib.txt` | published instance data, as downloaded |
| `taillard_bounds.csv` | lower bound, best known solution and open/closed status |
| `results/` | run output (not tracked) |

Instances and bounds come from
<https://github.com/thomasWeise/jsspInstancesAndResults>.

Reproduce a run with:

```
make -j4
bash experiments/classic_jsp_2026/scripts/run_jsp.sh \
    experiments/classic_jsp_2026/setup/jsp_abc_n2.txt my_run ta01 ta02 ta03
```

## Findings

### Baseline

The irace-tuned N2 configuration of the COR-2026 study, run on crisp
instances, 5 runs of 60 s, `ta01`-`ta10`:

| | mean gap to LB |
|---|---|
| best run | 0.467 % |
| averaged over runs | 1.081 % |

`ta04` is solved to optimality. The same algorithm family sits around 3-4 % on
the interval instances, so the crisp problem starts from a much tighter
position and the work is in the last half percent.

Two things had to change before the tuned configuration made sense here:

* **the stopping rule.** `noimprovement = 20` fires while the run is still
  improving; on `ta01` the search reached 1258 at generation 8 and was stopped
  at generation 28 with the population average at 1259, i.e. collapsed onto the
  best individual. The setups here use the time budget instead.

### The tabu list was unbounded

`setup_N2_tuned.txt` sets `localsearch.tabu-size.min` but not
`localsearch.tabu-size.max`, and `TabuList::setup` then defaults the maximum to
infinity. Moves are therefore never released: a long tabu run forbids more and
more of its own neighbourhood, and each `isTabu()` call scans a list that keeps
growing. This barely shows with `bad-iterations = 20`, which is why the tuning
did not surface it, but it blocks any attempt to search longer.

Probe on the three hardest of the small instances, 3 runs of 60 s, best of 3:

| tenure | bad-iter | LS max-time | ta01 | ta05 | ta09 | mean gap |
|---|---|---|---|---|---|---|
| unbounded | 20 | 2 s | 1240 | 1240 | 1291 | 1.12 % |
| **12** | 20 | 2 s | **1231** (optimal) | **1233** | 1296 | **0.82 %** |
| 12 | 500 | 5 s | 1231 (optimal) | 1237 | 1298 | 0.98 % |
| 12 | 2000 | 15 s | 1240 | 1237 | 1296 | 1.17 % |

Bounding the tenure finds the optimum of `ta01`; the unbounded list does not.
Spending *longer* in each local search does not pay at a fixed total budget,
because time spent on one individual is taken from the number of generations.
