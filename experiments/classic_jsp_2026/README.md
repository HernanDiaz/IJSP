# Classic JSP on the Taillard instances

## About this branch

Branch `experiment/classic-jsp`, taken from `feature/IJSP` at `20d622d`
("ASOC resubmission: de-anonymize manuscript"). It follows the naming of the
other exploratory branches in this repository (`experiment/qea`,
`experiment/multiobjective`, ...) and is meant to be read the same way: a
self-contained line of work with its own experiment directory, whose findings
are recorded here whether they were positive or not.

It changes nothing that existing setups depend on. Everything it adds is opt-in
through a setup file, so `feature/IJSP` and the other experiment branches
behave exactly as before if this branch is merged.

**Files added or changed outside this directory:**

| file | what and why |
|---|---|
| `JSPCertificateAnalyzer.{h,cpp}` | new post-execution analyzer writing a verifiable schedule; used only when a setup asks for `postexecution.analyzer = jsp.certificate` |
| `LS_TabuBackJump.{h,cpp}` | new local search (tabu with back-jump tracking); used only when a setup asks for `localsearch = tabu-backjump` |
| `PostExecutionClassRegister.h` | one line registering `jsp.certificate` |
| `LocalSearchClassRegister.h` | two lines registering `tabu-backjump` and its alias `tsab` |
| `PostExecutionManager.cpp` | reads the new optional `postexecution.analyzer` setting; falls back to the objective as before when it is absent, so existing setups are unaffected |
| `Makefile` | the two new `.cpp` files added to `SOURCES` |
| `.gitignore` | ignores this directory's `results/` and Python bytecode |
| `TaillardJSP/` | the 80 Taillard instances converted to the framework's format |

**New setup parameters,** all optional:

| parameter | default | meaning |
|---|---|---|
| `postexecution.analyzer` | the objective, as before | which post-execution analyzer to run |
| `localsearch.backjump.size` | 5 | how many decision points the back-jump stack holds |
| `localsearch.backjump.max-jumps` | 3 | how many jumps one local search call may make |


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
| `JOURNAL.md` | chronological record of what was tried, dead ends included |
| `SCALING.md` | what to run first on a machine with more cores, and what more cores will not fix |

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

### The tabu list is unbounded, but bounding it does not help

`setup_N2_tuned.txt` sets `localsearch.tabu-size.min` but not
`localsearch.tabu-size.max`, and `TabuList::setup` then defaults the maximum to
infinity. Moves are therefore never released: a long tabu run forbids more and
more of its own neighbourhood, and each `isTabu()` call scans a list that keeps
growing. The mechanism is real, and worth knowing before anyone tries to search
longer.

It does not, however, cost anything at this budget. A first probe on the three
hardest small instances with **3 runs** suggested it did — bounding the tenure
at 12 found the optimum of `ta01` where the unbounded list reached only 1240.
Repeating the comparison properly, **10 runs of 60 s on `ta01`-`ta10`**, the
effect disappears:

| tenure | mean best gap | mean gap per run | instances solved to optimality |
|---|---|---|---|
| unbounded | **0.280 %** | **1.011 %** | 4 (`ta01`-`ta04`) |
| 12 | 0.328 % | 1.049 % | 3 (`ta01`, `ta02`, `ta04`) |

The unbounded list is marginally ahead and the difference is well inside the
run-to-run spread, so the honest reading is that the two are tied. The 3-run
probe was simply underpowered: with 10 runs the unbounded configuration finds
the optimum of `ta01` too.

The reason the tenure does not bite is that `bad-iterations = 20` ends each
local search long before the list has grown enough to matter. It would bite in
a configuration that searches longer — and longer local searches were also
tested, and also did not pay: at a fixed total budget, time spent on one
individual is taken from the number of generations.

**Lesson for later comparisons in this directory: 3 runs cannot separate
configurations on these instances. Differences below roughly half a percent
need 10 runs or more before they mean anything.**

### Back-jump tracking must be capped, or it starves the population

`LS_TabuBackJump` implements Nowicki and Smutnicki's back-jump idea: remember
the points where the search left a best-quality solution, and on stagnation
return to the most recent one with the move it took there forbidden.

Dropped into this hybrid unchanged, it is a disaster. On `ta01`-`ta10`, 10 runs
of 60 s:

| local search | mean best gap | mean gap per run |
|---|---|---|
| plain tabu | **0.280 %** | **1.011 %** |
| back-jump, uncapped | 6.650 % | 9.224 % |

The counters say why. Plain tabu search stops after 20 non-improving iterations
and returns in about 1.3 ms, so a 60 s run completes **183 generations**. An
uncapped back-jump search keeps jumping instead of stopping, so it spends its
whole `localsearch.max-time` allowance on every call. With a population of 250
one generation would need 500 s, and the run completes **one generation**: the
algorithm degenerates into tabu search from a few random starting points and
the population never evolves.

This is not a defect of back-jump tracking, it is a defect of spending an
unbounded amount of time per individual inside a population-based method. The
class therefore caps the jumps per call with `localsearch.backjump.max-jumps`
(default 3), which keeps the cost of a call within a small factor of plain tabu
search.

It is the same lesson as the local-search-length probe, in a sharper form: in
this hybrid the total budget is the scarce resource, and anything that makes
one local search longer is paid for in generations.

### Capped back-jump, at a matched budget

With `max-jumps` capped and `bad-iterations` reduced so that the total work per
call matches the baseline (the run completes 194 generations against the
baseline's 183, so the budgets really are comparable), `ta01`-`ta10`, 10 runs of
60 s:

| local search | mean best gap | mean gap per run | solved to optimality |
|---|---|---|---|
| plain tabu, 20 bad iterations | 0.280 % | 1.011 % | 4 |
| plain tabu, tenure 12 | 0.328 % | 1.049 % | 3 |
| back-jump, 3 jumps, 5 bad iterations | **0.256 %** | 1.146 % | **6** |
| back-jump, 1 jump, 10 bad iterations | 0.296 % | **0.895 %** | 4 |

Read this carefully. The mean best gaps span 0.256 % to 0.328 %, which is well
inside the spread this directory has already shown it cannot resolve, so **no
configuration is demonstrably better than another on that measure**. The one
signal that is not a fraction of a percent is the count of instances solved to
optimality: capped back-jump with 3 jumps reaches 6 of 10 against the
baseline's 4, adding `ta08` and `ta10`. That is suggestive, not established —
separating it properly needs more runs than fit here.

The two back-jump variants also trade off against each other in a readable way:
more jumps with shorter passes finds better single solutions, fewer jumps with
longer passes gives better average runs.

## Verified solutions

`solutions/` holds the best schedule found for each instance, as a certificate
that passes `verify_certificate.py` against the published instance data.
`solutions/index.csv` lists them. Every file can be rechecked with:

```
python3 experiments/classic_jsp_2026/scripts/verify_certificate.py \
    --orlib experiments/classic_jsp_2026/reference/taillard_orlib.txt \
    --instance ta01 \
    --certificate experiments/classic_jsp_2026/solutions/ta01.csv \
    --bounds experiments/classic_jsp_2026/taillard_bounds.csv
```

Current state, pooling every run made so far:

| instance | makespan | LB | BKS | |
|---|---|---|---|---|
| ta01 | 1231 | 1231 | 1231 | optimal |
| ta02 | 1244 | 1244 | 1244 | optimal |
| ta03 | 1218 | 1218 | 1218 | optimal |
| ta04 | 1175 | 1175 | 1175 | optimal |
| ta05 | 1231 | 1224 | 1224 | 0.57 % |
| ta06 | 1240 | 1238 | 1238 | 0.16 % |
| ta07 | 1228 | 1227 | 1227 | 0.08 % |
| ta08 | 1217 | 1217 | 1217 | optimal |
| ta09 | 1291 | 1274 | 1274 | 1.33 % |
| ta10 | 1241 | 1241 | 1241 | optimal |

Six of the ten 15x15 instances are solved to proven optimality. None of these
improves on a published result — all ten are closed instances, so matching the
bound is the best outcome available on them. Improving a published best known
solution requires the open instances (`ta18`, `ta22`-`ta50`), which are 20x20
and 30x20 and need a far larger budget than the 60 s runs used here.

## Where the benchmark is actually hard

Run `scripts/analyze_hardness.py` to reproduce:

| group | n/m | LB == trivial | LB above trivial | open | mean (BKS-LB)/LB |
|---|---|---|---|---|---|
| 15x15 | 1.0 | 0/10 | +31.23 % | 0/10 | - |
| 20x15 | 1.3 | 0/10 | +15.34 % | 1/10 | 1.38 % |
| 20x20 | 1.0 | 0/10 | +27.40 % | **7/10** | 2.86 % |
| 30x15 | 2.0 | 5/10 | +3.43 % | 4/10 | 0.47 % |
| **30x20** | 1.5 | 0/10 | +8.36 % | **10/10** | 2.65 % |
| 50x15 | 3.3 | 9/10 | +0.15 % | 0/10 | - |
| 50x20 | 2.5 | 5/10 | +0.26 % | 0/10 | - |
| 100x20 | 5.0 | 9/10 | +0.03 % | 0/10 | - |

Size does not predict difficulty. The `100x20` instances have 2000 operations
and are all solved; the `30x20` instances have 600 and are all open.

What predicts it is the jobs-to-machines ratio, through the strength of the
trivial bound `max(busiest machine's workload, longest job)`. At `n/m = 5` that
bound already *equals* the published lower bound in 9 instances out of 10 and
is within 0.03 % of it in the rest: the bottleneck machine carries so much work
that keeping it busy is essentially all there is to do, and any competent
heuristic reaches the optimum. At `n/m = 1` the published bound sits 27-31 %
above the trivial one -- establishing it took real branch-and-bound work, and
the instance is combinatorially hard.

Difficulty therefore lives where an instance is square *and* too large for exact
methods. `15x15` is square and hard but only 225 operations, so branch and bound
closed all ten. `20x20` (400 operations) and `30x20` (600) are square and out of
reach: **17 of the 22 open instances are in those two groups**, and `ta41`-`ta50`
is the hardest group in the benchmark -- all ten open, mean gap 2.65 %.

Two cautions before competing against any of these numbers:

* **"Closed" means the published lower bound and the best known solution
  coincide**, so optimality is proven. It does not mean the instance is easy:
  `ta01` took the literature decades.
* **The bounds here are a snapshot of a literature survey**
  (`thomasWeise/jsspInstancesAndResults`), not a live registry. Instances may
  have been closed or best-known solutions improved since it was compiled. Any
  claim to have improved a published result must be checked against the current
  literature first -- this file is not sufficient evidence.

## The open instances (all 22)

Both configurations, 5 runs of 300 s each, on every open Taillard instance.
Recomputed from certificates; reproduce with `scripts/compare.py`.

| | mean best gap to LB |
|---|---|
| plain tabu | 5.289 % |
| back-jump, 3 jumps | 5.173 % |

**No published result is in reach.** Best known solutions on these instances sit
3-5 % below anything produced here, and they are the product of decades of
specialised work. The runs were made for a baseline and a comparison, not in the
expectation of beating a record.

### Back-jump: a promising subgroup that did not replicate

The `30x20` group was run first, because it is the hardest, and back-jump led
there on 8 of 10 instances. Extending to the remaining 12 open instances
reversed it:

| instances | back-jump better (per-instance mean) | Wilcoxon |
|---|---|---|
| `ta41`-`ta50`, looked at first | 8 of 10 | p = 0.084 |
| the other 12 | **2 of 12** | **p = 0.022, favouring plain tabu** |
| all 22 | 10 of 22 | p = 1.000 |

On the 12 instances added afterwards plain tabu is better at the 5 % level, and
over the whole open set the two are indistinguishable.

The parsimonious reading is that the first result was a selection effect: one
group, chosen for being hardest, inspected after the fact, never significant on
its own. The prediction it generated was then tested on 12 instances nobody had
looked at, and failed in the other direction.

**Back-jump is therefore not carried forward as an improvement.** The class
stays -- it is correct, capped, and inert unless a setup asks for it -- but the
default configuration has no reason to use it.

This also retires the claim made when the `30x20` numbers first came in, that
hard instances discriminate where easy ones cannot. The 12 added instances
discriminated perfectly well; they just discriminated the other way.

## Choosing a time budget

Every setup here stops on wall-clock time, so **a run costs its full budget
regardless of instance size** -- a 15x15 instance does not finish sooner than a
30x20 one, it just does more generations in the same 300 s. The budget is a free
parameter, and `scripts/convergence.py` prices it by reporting what fraction of
runs were still improving past a given point:

| budget spent | `ta01`-`ta10` (15x15, 60 s) still improving | `ta41`-`ta50` (30x20, 300 s) still improving |
|---|---|---|
| 40 % | 13 % | 52 % |
| 50 % | 12 % | 42 % |
| 75 % | 4 % | 22 % |
| 90 % | 2 % | **16 %** |

The answer is opposite in the two groups.

On the easy instances the search is done early -- the last improvement lands at
12 % of the budget for the median run -- so **half the budget could be cut while
touching only about one run in eight**, and the same CPU spent on twice as many
runs. Since every comparison in this directory has been limited by statistical
power rather than by solution quality, that trade is worth taking.

On the hard instances the opposite holds: **16 % of runs were still improving in
the final 10 % of their budget**, so 300 s is if anything too short there, and
shortening it would throw away results rather than time.

The budget of an experiment already running must not be changed -- the 22 open
instances are being compared at one budget and have to stay that way -- but this
is how the next one should be sized.

## Next steps

* Settle the back-jump question. The 30x20 runs put it at p = 0.084; the cheapest
  way to more power is more paired instances rather than more runs, since the
  test pairs by instance -- the other 12 open instances (`ta18`, the seven open
  `20x20`, and four open `30x15`) would roughly double the sample.
* A published result is not in reach on the open instances from this
  configuration, at 3-5 % above the best known. Closing that needs a different
  algorithm, not a longer run.
* The population collapses: on `ta01` the average makespan reaches the best
  one's value within 30 generations. Nothing here addresses that, and the
  seeding-study branch suggests it is the binding constraint.
