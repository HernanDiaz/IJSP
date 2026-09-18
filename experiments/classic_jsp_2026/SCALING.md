# Running this on a bigger machine

Everything measured in this directory was produced on 4 cores, and the binding
constraint was never solution quality -- it was **statistical power**. Four
configurations of the local search landed within 0.07 percentage points of each
other on `ta01`-`ta10` and nothing could be concluded; the 30x20 comparison
reached p = 0.084 and still could not close. More cores buy exactly the thing
that was missing.

## Nothing needs changing to scale

`run_jsp.sh` sets `MAX_PARALLEL` from `nproc`, and each solver process is
single-threaded, so one process per core is the right setting. Override it only
to leave headroom on a shared machine:

```
MAX_PARALLEL=24 bash experiments/classic_jsp_2026/scripts/run_jsp.sh \
    experiments/classic_jsp_2026/setup/<setup>.txt <results-dir> ta41 ta42 ...
```

Re-running the same command resumes: an instance is skipped only when its
certificate holds every run the setup asks for, and one interrupted partway is
redone. A batch killed by a restart costs the runs it lost, nothing more.

## What to run, in order

### 1. Settle the back-jump question

The open question. Back-jump leads on 8 of 10 instances by per-instance mean
(Wilcoxon p = 0.084) on `ta41`-`ta50`; the extension to all 22 open instances is
what this session could reach. The test pairs by instance, so **power comes from
instances first and runs second**.

* All 22 open instances, both `open_baseline_300s` and `open_backjump_300s`,
  **15 runs** instead of 5.
* Cost: 22 x 2 x 15 x 300 s = 55 CPU-hours. Under 3 hours on 20 cores.
* Then `scripts/compare.py`. If it does not separate at 15 runs on 22 paired
  instances, back-jump is not worth carrying and the honest answer is no.

### 2. Re-run the easy-instance comparison at a budget that is not wasted

`ta01`-`ta10` were compared at 60 s, and `convergence.py` shows the median run's
last improvement at 12 % of that. At 30 s with 40 runs, the same CPU as 20 runs
at 60 s, roughly one run in eight is affected and the sample doubles.

* Cost: 10 x 2 x 40 x 30 s = 6.7 CPU-hours.
* This is also the cheapest test of the methodological claim above -- that these
  instances cannot discriminate. If the difference stays invisible at 40 runs,
  that is worth stating as a finding about the benchmark, not about the
  algorithm.

### 3. Give the hard instances the budget they want

16 % of `ta41`-`ta50` runs were still improving in the final tenth of 300 s, so
that budget is truncating the search.

* `ta41`-`ta50`, best configuration, **1800 s**, 10 runs, and re-run
  `convergence.py` on the result: if runs still improve at 90 %, raise it again.
* Cost: 10 x 10 x 1800 s = 50 CPU-hours.
* This measures how much of the 6.4 % gap is the algorithm and how much is the
  clock. Worth knowing before concluding anything about the algorithm's ceiling.

### 4. Only then, the things not yet tried

The population collapses -- on `ta01` the average makespan reaches the best one's
value within 30 generations -- and every change tried here was inside the local
search. The `seeding-study` branch suggests the collapse is the binding
constraint, which would explain why four local-search configurations land within
a tenth of a percent of each other. That is the next place to look, and it is a
design question, not a compute question.

## What more cores will not do

Best known solutions on the open instances sit **3-5 % below** anything this
configuration produces. Those numbers come from decades of specialised
algorithms; no amount of parallelism closes that from here. Steps 1-3 are worth
doing because they answer questions that are actually open. Improving a
published result is not one of them.
