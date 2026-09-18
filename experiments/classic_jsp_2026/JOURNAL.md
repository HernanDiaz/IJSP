# Lab journal — classic JSP line

Chronological record of what was tried on `experiment/classic-jsp`, in order,
including the things that did not work and the mistakes made. The README holds
the conclusions; this file holds how they were reached, so that a dead end is
not walked twice and a claim can be traced back to the run that produced it.

All runs below are on the session's container: Intel Xeon @ 2.80 GHz, **4 cores**,
15 GiB RAM, `-march=native` resolving to `cascadelake`. Wall-clock budgets are
comparable between entries here but not with runs on other machines.

---

## 2026-09-17

### Finding the work

The session's clone contained only `main`, which holds nothing but a LICENSE.
`git fetch --all` brought in 7 branches and 3 tags; the real work lives on
`feature/IJSP` (191 commits) and five `experiment/*` branches. Worth knowing:
**a fresh clone here does not show the repository's content.**

### Deciding not to write a new problem domain

`FuzzyFW::Interval` is `{double a, b}`. With `a == b` every ranking method
collapses to the order on the reals and the component-wise maximum is the
ordinary maximum, so a classic JSP instance is an IJSP instance with degenerate
intervals `[p, p]` and the existing solver optimises the classic makespan
exactly.

Checked before relying on it: the converted `ta01` reproduces the machine
routes of `SelectosYTaillardIntervalos/tai15_15_01.F.15_01.txt`, and its
durations are the centres of that instance's intervals. The interval benchmark
was built from the same Taillard data, so the two agree by construction.

The alternative — a crisp `int` specialisation — would be faster but is a
second implementation to keep correct. Not worth it until speed is shown to be
the binding constraint. It has not been.

### Getting the instances

Through the container's proxy, `jobshop.jzhang.xyz` and Taillard's own page at
`mistic.heig-vd.ch` both return 403. `raw.githubusercontent.com` works, so the
data comes from <https://github.com/thomasWeise/jsspInstancesAndResults>:
instances in OR-Library format and a bounds table. 80 Taillard instances,
**58 closed** (LB == BKS), **22 open**.

### First run, and what it showed

`ta01`, tuned N2 config, 30 s budget: makespan 1258, reached at generation 8
(3.3 s). Then the population average fell from 1284 to 1259.07 — everything
collapsed onto the best individual — and `noimprovement = 20` stopped the run
at generation 28.

Two things came out of this:
* the tuned stopping rule fires while the run is still improving, so the setups
  here use the time budget instead;
* the robustness analyser wrote 6.5 MB of scenarios per instance, all identical,
  because sampling inside a degenerate interval is a no-op. Replaced with the
  certificate analyser.

### Certificates before results

Before measuring anything, built the chain that makes a reported makespan
checkable: `JSPCertificateAnalyzer` writes start times, `verify_certificate.py`
re-checks them **against the published OR-Library data**, not against the
converted instance file, so a conversion bug cannot hide.

Tested that the checker actually rejects things, since a checker that never
fails proves nothing. Four tamper classes, all caught: moving a start time,
falsifying a duration, deleting a task, overlapping two operations on a machine.
The clean certificate passes.

### Baseline

`ta01`-`ta10`, 5 runs of 60 s: mean best gap **0.467 %**, `ta04` optimal.
Against the 3-4 % this algorithm family sits at on the interval instances, the
crisp problem starts much tighter — the work is in the last half percent.

---

## Parameter probes

### The tabu list is unbounded

`setup_N2_tuned.txt` sets `localsearch.tabu-size.min` but not `.max`, and
`TabuList::setup` defaults the maximum to infinity: moves are never released and
`isTabu()` scans a list that keeps growing.

**Probe, 3 runs of 60 s on the three hardest small instances:**

| tenure | bad-iter | LS max-time | ta01 | ta05 | ta09 | mean gap |
|---|---|---|---|---|---|---|
| unbounded | 20 | 2 s | 1240 | 1240 | 1291 | 1.12 % |
| 12 | 20 | 2 s | 1231 | 1233 | 1296 | 0.82 % |
| 12 | 500 | 5 s | 1231 | 1237 | 1298 | 0.98 % |
| 12 | 2000 | 15 s | 1240 | 1237 | 1296 | 1.17 % |

Read this as "bounding the tenure finds the optimum of ta01 and the unbounded
list does not", and wrote it up that way. **That was wrong.**

### Dead end 1: longer local searches

Rows 3 and 4 above. Giving each local search more time does not pay at a fixed
total budget, because time spent on one individual comes out of the number of
generations. This turned out to be the single most important fact about this
hybrid, and it came back twice more.

### Dead end 2: the tenure bound — and a correction

Repeated the comparison properly, **10 runs of 60 s on all of ta01-ta10**:

| tenure | mean best gap | mean per run | optimal |
|---|---|---|---|
| unbounded | **0.280 %** | **1.011 %** | 4 |
| 12 | 0.328 % | 1.049 % | 3 |

The effect disappeared. With ten tries the unbounded configuration finds the
optimum of `ta01` too. The 3-run probe was underpowered and the write-up was
corrected.

The mechanism is still real — it would bite in a configuration that searches
longer — but `bad-iterations = 20` ends each local search long before the list
grows enough to matter.

**Standing rule from this, used for every comparison since: 3 runs cannot
separate configurations on these instances. Differences below roughly half a
percent need 10 runs or more before they mean anything.**

---

## Back-jump tracking

Implemented `LS_TabuBackJump`: Nowicki and Smutnicki's idea of remembering the
points where the search left a best-quality solution, and returning to the most
recent one on stagnation with the move it took there forbidden.

### Dead end 3: it starved the population

`ta01`-`ta10`, 10 runs of 60 s: mean best gap **6.650 %** against the baseline's
0.280 %. Twenty times worse.

Diagnosed from the counters rather than by guessing. **Generations: 1, against
the baseline's 183.** Plain tabu returns after 20 non-improving iterations, about
1.3 ms; back-jump keeps jumping instead of stopping and spends its whole
`localsearch.max-time` allowance on every call. With a population of 250 one
generation needs 500 s, so the run never finishes one, and the hybrid degenerates
into tabu search from a handful of random starting points.

The same lesson as dead end 1, in a sharper form. Added
`localsearch.backjump.max-jumps` — an unbounded local search inside a
population-based algorithm is a defect regardless of how good the local search
is, so the cap belongs in the class, not in a setup file.

### Capped, at a matched budget

Reduced `bad-iterations` so total work per call matches the baseline, and
checked that it really did: 194 generations against 183.

| local search | mean best gap | mean per run | optimal |
|---|---|---|---|
| plain tabu, 20 bad iterations | 0.280 % | 1.011 % | 4 |
| plain tabu, tenure 12 | 0.328 % | 1.049 % | 3 |
| back-jump, 3 jumps, 5 bad iterations | **0.256 %** | 1.146 % | **6** |
| back-jump, 1 jump, 10 bad iterations | 0.296 % | **0.895 %** | 4 |

**Undecided.** The best gaps span 0.256 %-0.328 %, inside the spread the standing
rule above says cannot be resolved at 10 runs. The only signal larger than the
noise is the optimality count, 6 against 4 (`ta08` and `ta10` added). Suggestive,
not established.

A secondary pattern does look consistent: more jumps with shorter passes gives
better single solutions, fewer jumps with longer passes gives better average
runs — intensification against robustness.

### A reporting bug in my own tooling

`summarize.py` reported `ta09` and `ta10` as "all runs infeasible" while they
were still running. The certificates held only their header, `load_certificate`
returned nothing, and the empty case was reported as infeasible. A partially
written file looked exactly like a correctness failure — the sort of false alarm
that costs an afternoon. Now reads "no runs recorded yet".

---

---

## 2026-09-18 — the open instances

### Locating the difficulty first

Before spending compute, compared each instance's published lower bound against
the trivial `max(busiest machine's workload, longest job)`. Size turns out not
to predict difficulty at all: `100x20` has 2000 operations and is fully solved,
`30x20` has 600 and is fully open. The predictor is the jobs-to-machines ratio
(table in the README, reproduce with `scripts/analyze_hardness.py`).

That redirected the target to `ta41`-`ta50`, and ruled out `ta51`-`ta80`, which
would otherwise have looked like the impressive thing to attack.

### Calibration on ta41, and a first real separation

One run of 300 s each on `ta41` (30x20, 600 operations; LB 1906, BKS 2005):

| config | generations | makespan | over LB | over BKS |
|---|---|---|---|---|
| plain tabu | 494 | 2168 | +13.7 % | +8.1 % |
| back-jump, 3 jumps, 5 bad iterations | 520 | **2106** | +10.5 % | +5.0 % |

62 units apart, 2.9 %. On `ta01`-`ta10` the four configurations sat within 0.07
percentage points of each other and nothing could be concluded; here a single
run separates them by two orders of magnitude more. **Hard instances
discriminate between configurations; easy ones do not.** If this holds across the
group it also explains the earlier null results -- they were measured on
instances with no room left to differ.

First mistake of the day, for the record: the two calibration runs were pointed
at the same log folder and started in the same second, so their signatures
collided and one overwrote the other. Separate folders per configuration.

### What is and is not on the table

At 2106 against a best known of 2005, this configuration is 5 % away from the
published result. Improving a BKS on this group is not going to happen from
here; those numbers are the product of decades of specialised algorithms.

What is worth having: a baseline for this algorithm family on the hardest group
in the benchmark, where it has never been run, and a proper test of whether the
back-jump advantage seen above is real. Launched both configurations, 5 runs of
300 s on `ta41`-`ta50`, about two hours on four cores.

### ta41-ta50, both configurations

5 runs of 300 s each. Mean best gap to the lower bound: plain tabu 6.826 %,
back-jump 6.380 %. Per-instance differences up to 29 makespan units, against
0.024 percentage points between the same two configurations on `ta01`-`ta10`.

`compare.py` pairs by instance -- instances differ from each other far more than
configurations do, so an unpaired test over pooled makespans would mostly
measure which instances are hard:

```
best-of-runs:      back-jump wins 7, loses 2, ties 1   sign test p = 0.180
per-instance mean: back-jump better on 8 of 10         Wilcoxon  p = 0.084
```

Consistent direction, far larger effect than on the easy instances, still not
significant. Extended the comparison to the other 12 open instances -- the test
pairs by instance, so 22 pairs buys more power than doubling the runs would.

**The methodological point is the one worth keeping: a comparison run on closed
15x15 instances cannot detect a difference this algorithm actually has.** Any
tuning calibrated only on those instances is measuring noise.

### Budget: too long on easy instances, too short on hard ones

Every setup stops on wall-clock time, so a run costs its full budget whatever
the instance size -- a 15x15 does not finish sooner than a 30x20, it does more
generations in the same seconds. `convergence.py` prices the budget by asking
what fraction of runs were still improving past a given point:

| budget spent | 15x15 @ 60 s | 30x20 @ 300 s |
|---|---|---|
| 50 % | 12 % still improving | 42 % |
| 75 % | 4 % | 22 % |
| 90 % | 2 % | **16 %** |

Opposite conclusions. On the easy instances the median run's last improvement
lands at 12 % of the budget, so half of it could go while touching about one run
in eight -- and every comparison here has been limited by statistical power, not
by solution quality, so that CPU is better spent on more runs. On the hard
instances 16 % of runs were still improving in the final tenth, so 300 s is if
anything too short.

Did **not** change the budget of the running comparison. The 22 open instances
have to be compared at one budget or the pairing is worthless.

### A killed batch, and a flaw it exposed

The second batch died after four instances -- `rest.log` empty, `ta18` cut off at
"Run 2", almost certainly a container worker restart. The runs themselves were
fine; the problem was what happened next.

`run_jsp.sh` skipped any instance whose certificate file *existed*. Four
certificates held one run out of five. On a re-run those would have been skipped
as finished, and the comparison would have ended with 54 runs on one
configuration and 50 on the other, with nothing in the output looking wrong. The
script now counts the runs inside each certificate and compares against the
setup's `runs` value, redoing an instance that has fewer.

Also switched `MAX_PARALLEL` to default to `nproc` instead of a hardcoded 4, in
preparation for running this on a larger machine.

## Where it stands

Six of `ta01`-`ta10` solved to proven optimality: 1231, 1244, 1218, 1175, 1217,
1241. `ta05`, `ta06`, `ta07`, `ta09` remain 0.08 %-1.33 % above the bound. None
of this improves a published result and none of it could — all ten are closed
instances.

**Not started: the 22 open instances** (`ta18`, `ta22`-`ta50`). They are the only
place a published best-known solution can be improved, they are 20x20 and 30x20,
and 60 s runs on 4 cores will not touch them.

**Not addressed: the population collapse** seen in the very first run. Every
change tried here was inside the local search. The `seeding-study` branch
suggests the collapse is the binding constraint, which would explain why four
configurations of the local search land within a tenth of a percent of each
other.
