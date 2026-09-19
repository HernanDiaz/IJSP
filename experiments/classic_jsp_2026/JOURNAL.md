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

### The back-jump hypothesis does not survive the full sample

Extended to all 22 open instances, 5 runs of 300 s. The result reverses.

| instances | back-jump better (per-instance mean) | Wilcoxon |
|---|---|---|
| `ta41`-`ta50` (30x20), looked at first | 8 of 10 | p = 0.084 |
| the other 12 open instances | **2 of 12** | **p = 0.022, favouring plain tabu** |
| all 22 | 10 of 22 | p = 1.000 |

On the 12 instances added afterwards, plain tabu is better at the 5 % level.
Over the whole open set there is nothing: 10 of 22, p = 1.000, and the mean best
gaps are 5.289 % against 5.173 %.

The parsimonious reading is that the `ta41`-`ta50` result was a selection effect.
It was one group, chosen because it was the hardest, inspected after the fact,
and it never reached significance on its own. The prediction it generated --
back-jump wins on hard instances -- was then tested on 12 instances that had not
been looked at, and failed, significantly, in the other direction. That is the
test doing its job.

The alternative reading, that back-jump genuinely helps only at 30x20 and hurts
below it, cannot be excluded from this data, but nothing here supports
preferring it over the simpler explanation. **Back-jump is not carried forward
as an improvement.** The class stays -- it is correct, capped, and costs nothing
unless a setup asks for it -- but the default configuration has no reason to use
it.

What this also retires is the methodological claim made when the 30x20 numbers
came in: that hard instances discriminate where easy ones cannot. The 12 added
instances discriminated perfectly well -- they just discriminated the other way.

### A bug in the comparison script, caught by the result

`compare.py` printed "B is better at the 5% level" whenever `p < 0.05`, without
checking the direction. On the 12 added instances it therefore announced
back-jump as the winner of a test that back-jump had lost 2-10. The p-value says
the two differ, not which way round.

It was visible here only because the per-instance counts printed on the line
above contradicted the verdict. A quieter version of this script would have put
the wrong conclusion in this journal and nothing would have caught it. The
verdict now names the side the counts actually favour.

## Where it stands

Six of `ta01`-`ta10` solved to proven optimality: 1231, 1244, 1218, 1175, 1217,
1241. `ta05`, `ta06`, `ta07`, `ta09` remain 0.08 %-1.33 % above the bound. None
of this improves a published result and none of it could — all ten are closed
instances.

All 22 open instances have now been run with both configurations, 5 runs of
300 s: mean best gap to the lower bound 5.289 % for plain tabu, 5.173 % for
back-jump, with best known solutions sitting 3-5 % below both. No published
result is in reach from here.

**Three changes were tried inside the local search and none of them helped**:
bounding the tabu tenure, lengthening the local search, and back-jump tracking.
The only clear gain in the whole line came from the stopping rule — letting runs
use their time budget instead of stopping after 20 generations without
improvement.

**Not addressed: the population collapse** seen in the very first run. Every
change tried here was inside the local search. The `seeding-study` branch
suggests the collapse is the binding constraint, which would explain why four
configurations of the local search land within a tenth of a percent of each
other.


---

## 2026-09-19 — the crisp refactor, step 1

Work moves to `experiment/classic-jsp-crisp`. The decision recorded on
2026-09-17 not to write a crisp specialisation was explicitly conditional --
"not worth it until speed is shown to be the binding constraint" -- and
SCALING.md then showed exactly that: every comparison in this directory has
been limited by statistical power, and power is bought with runs, which are
bought with speed.

### The machine, and why the published numbers are not the baseline

All runs below are on Roberto: Xeon E5-2680 v4 (Broadwell), inside WSL2 with 14
processors and 8 GiB, g++ 11.4.0, `-march=native`. **These are not comparable
with the entries above**, which were made on a 4-core Cascade Lake container at
2.80 GHz.

The difference is not small. The same tuned configuration
(`bj_j3_b5_60s.txt`, `ta01`-`ta10`, 10 runs of 60 s) completes **125.3
generations per run here against the 194 recorded on the container** -- 35 %
less search for the same clock. That has a visible cost: the untouched
`experiment/classic-jsp` build reproduces five of the six known optima on this
machine and misses `ta10`, reaching 1243 against 1241. Everything else lands
where it did before (`ta05` 1233, `ta06` 1243, `ta07` 1228, `ta09` 1291).

A miss of 2 units is 0.16 %, well inside the spread this directory has already
established it cannot resolve at ten runs, so this is the machine being slower
and not a port being wrong. But it does mean the published table cannot serve as
the reference for this refactor. **The baseline for every comparison from here
is the untouched branch measured on this machine**, which is the comparison that
was going to be required anyway: same machine, same compiler.

### Step 1: Interval becomes Crisp

`Crisp` holds one `int`; every operation on it is inline. `Interval` held two
`double`s and dispatched every comparison through an out-of-line seven-branch
switch over a runtime enum, ending in two epsilon comparisons -- in the
innermost loop of both the scheduler and the local search.

The substitution commit had never been compiled. Two things were needed.

`ScheduleIJSP` still did two-component arithmetic in three places the
substitution had missed: the makespan accumulator and the makespan check in
`verifyHeads`, and the head repair in `adjustHead`. The makespan check also
becomes exact equality rather than a tolerance on doubles, matching the head
check -- on integers a tolerance means nothing.

The other 70 errors were all in the interval robustness machinery, which samples
durations inside `[a, b]`. That is a no-op on a point, and it is what made the
very first run of this line write 6.5 MB of identical scenarios per instance. So
`IJSPRobustnessAnalyzer{,Makespan,Tardiness}`, `ScenarioManager`,
`MakespanMRAnalyzer`, `MakespanMRFileWriter`, `Interval`, `IntervalTest` and its
runner `mainTest.cpp` are deleted rather than ported. `RobustnessFileWriter`
stays, because `FJSPRobustnessAnalyzer` and `FJSPScenarioManager` still use it.

An IJSP setup must now name `postexecution.analyzer` explicitly. The old names
are left unregistered on purpose, so a setup that asks for interval robustness
on a crisp solver fails loudly rather than being quietly given something else.

### Verification: the same search, not merely the same answer

Matching final makespans would be weak evidence -- two different searches can
land on the same number. The check that actually settles it is the trace. With
the same seed on `ta01`, the crisp build and the untouched build agree
**generation by generation, on both the best makespan and the population
average, over the whole common prefix**. On degenerate intervals every ranking
method is the order on the reals, so the two solvers should take identical
decisions, and they do. The substitution changed the speed and nothing else.

Every certificate produced below was checked by `verify_certificate.py` against
the published OR-Library data, not against the converted instance files.

### What it bought

`ta01`-`ta10`, 10 runs of 60 s, both builds on this machine, one process per
instance so the load is identical. Reproduce with `scripts/generations.py`,
which reads the solver's own per-run generation counts rather than timing from
outside.

| | mean gen/s | generations per 60 s run |
|---|---|---|
| `experiment/classic-jsp` | 2.088 | 125.3 |
| crisp, step 1 | **3.761** | **225.7** |

**1.80x**, and remarkably flat: the per-instance speed-up runs from 1.69x
(`ta06`) to 1.93x (`ta01`). (This figure comes from a block design and is
superseded by the paired measurement of 1.85x in the last entry below; the
substance is unchanged, the precision was not justified.) That is the low end of the 2-4x that motivated the
branch, and it is the honest figure -- the remaining interval scaffolding
(ignored ranking arguments, N2's second pass over the critical path) is still
in place.

Solution quality is unchanged or better on every instance:

| | ta01 | ta02 | ta03 | ta04 | ta05 | ta06 | ta07 | ta08 | ta09 | ta10 |
|---|---|---|---|---|---|---|---|---|---|---|
| original | 1231 | 1244 | 1218 | 1175 | 1233 | 1243 | 1228 | 1217 | 1291 | 1243 |
| crisp | 1231 | 1244 | 1218 | 1175 | 1233 | 1243 | 1228 | 1217 | 1291 | **1241** |

Nine of ten identical; `ta10` improves to its optimum. **The crisp build
recovers all six known optima on a machine where the original build reaches
five** -- the extra 100 generations per run buy back exactly the search depth
the slower processor had cost.

This is the first result in this directory that is not a null. It is also not a
better algorithm: it is the same algorithm, run twice as often, which is
precisely the resource SCALING.md identified as binding.


## 2026-09-19 — the crisp refactor, steps 2 and 3

### Step 2: the ranking arguments go

`Crisp` had kept `Interval`'s method names and still accepted the `Compare` and
`Maximum` arguments, which it ignored, so that step 1 could be a pure type
substitution. Both enums are now gone, with the arguments that selected them:

* `isGreaterThan`, `isLesserThan`, `isEqualTo`, `isGreaterEqualTo`,
  `isLesserEqualTo` and `EqualComponent` become `>`, `<`, `==`, `>=`, `<=`
  and `==`;
* `maximum(a, b, strategy)` and `minimum(a, b, strategy)` become `std::max`
  and `std::min`;
* `FitnessCrisp::FitnessCompareStrategy`, the static that carried the choice
  from the setup file into every fitness comparison, is gone, and
  `Fitness::Type::INTERVAL` is renamed `CRISP`;
* `evaluation.interval.comparison`, `evaluation.interval.maximum` and
  `sgs.interval.comparison` are removed from the setup files. The solver
  ignores them rather than rejecting them, so an old setup still runs, but a
  file that claims to select a ranking strategy which no longer exists is the
  kind of thing that gets believed later.

The rewrite was done by script rather than by hand, over the files whose names
contain `IJSP` -- no FJSP or FVRP file does, so the TFN ranking strategies, which
are genuinely different from each other, were never in scope. The one trap worth
recording is precedence: `!(a + b).EqualComponent(c, comp)` has to become
`!((a + b) == c)` and not `!(a + b) == c`, so a replacement sitting under a unary
`!` is parenthesised and every other context is left bare.

### Step 3: one pass over the critical path

`for (short int comp = 1; comp <= 2; comp++)` walked the critical path once per
interval endpoint, because with proper intervals G- and G+ are different graphs.
After step 2 the loop body does not mention `comp` at all. The loop is removed in
N1, N2, N3, N8, NH and Next.

Two things this turned up.

**N2Inter was not the same kind of loop.** It collects the boundary arcs of G-
and G+ into two separate vectors and returns their *intersection*, with the union
as a fallback when the intersection comes out empty, and it rebuilds
`criticalPath` on each pass -- so unlike the others its second pass was not
structurally inert. On crisp times it is still vacuous, because the two passes
run identical code over identical data and produce two copies of one vector, and
then intersection, union and either operand all coincide. It is rewritten by hand
to one pass, one vector, no fallback.

**On crisp data N2, N2Plus, N2Minus and N2Inter are the same neighbourhood.**
N2Plus walked G+ only, N2Minus G- only; with one endpoint there is one critical
graph. Their dead `comp` constants are removed and each carries a comment saying
so. Nothing in this directory selects them -- every setup uses
`ijsp.makespan.n2` -- but anyone choosing between them on crisp instances should
know they are choosing between four names for one thing.

### Verification

Same check as step 1, and it still holds: on `ta01` with the same seed the crisp
build agrees with the untouched `experiment/classic-jsp` build generation by
generation, on both the best makespan and the population average, over the whole
common prefix.

That check only covers what the tuned setup runs -- N2, the insertion SGS, the
makespan evaluation -- and the rewrite touched every neighbourhood. So each was
run on both builds with the same seed, at a population of 20 to keep the cost
down. **Seven of the eight produce identical traces**: N1, N2, N3, N8, N2Plus,
N2Minus and N2Inter, the last three despite now being N2.

**NH could not be checked.** It does not complete a single generation within 180 s
even at a population of 4 with a 1 s local-search budget -- *on either build*.
The time limit is only tested between generations, so a neighbourhood this
expensive overruns any budget by an unbounded amount; it is the same failure mode
as the uncapped back-jump search recorded above, and it is pre-existing rather
than a regression. NH is not used by any setup here. It is the one code path in
this refactor that no run has exercised, and that should be said plainly rather
than left implied.

### What steps 2 and 3 bought: nothing measurable

`ta01`-`ta10`, 10 runs of 60 s, identical protocol and identical load:

| | mean gen/s | generations per run | vs original |
|---|---|---|---|
| `experiment/classic-jsp` | 2.088 | 125.3 | - |
| crisp, step 1 | 3.761 | 225.7 | 1.802x |
| crisp, steps 1-3 | 3.767 | 226.0 | **1.805x** |

0.2 % apart, which is noise by any standard this directory has used. **The whole
of the speed-up came from step 1**, the type substitution and the inlining it
allowed; removing the ignored arguments and the second critical-path pass changed
the code for the better and the clock not at all.

That is less surprising on inspection than it was in prospect. The ignored
arguments were enum values passed by value into functions that were already being
inlined, so the optimiser had been deleting them all along. And the second pass
over the critical path was never the full walk it looks like: `added[]` and
`criticalPath[]` are not reset between passes, so the second pass re-seeded the
queue, popped the seeds, found every predecessor already marked, and stopped. It
cost one scan of `lastTaskMachine` -- fifteen machines -- not a second traversal.

The lesson is the same one this directory keeps relearning in different clothes:
the mechanism was real and the effect was not, and only measuring told the two
apart. Solution quality is unchanged: the same six optima, `ta10` included, and
the same makespans on all ten instances as the step-1 build.

### Where the branch stands

The interval machinery is gone: `Interval`, `IntervalTest`, the ranking enums,
the strategy arguments, the robustness analysers that sampled inside `[a, b]`,
and the setup keys that configured them. What is left is a crisp solver that
does the same search as the interval one, 1.8x faster, and reaches all six known
optima of `ta01`-`ta10` on a machine where the interval build reaches five.

The 2-4x that motivated the branch was optimistic at the top end; 1.8x is the
figure, and the paired re-measurement below puts it at 2.00x.

**What that does to SCALING.md's costings is not what it first looks like.** Every
setup here stops on wall-clock time, so a faster solver does not make a 6.7
CPU-hour experiment cheaper -- it still costs 6.7 CPU-hours, and spends them on
twice as much search. The gain is only realisable by *changing* the plan: step 2
of SCALING.md proposes cutting `ta01`-`ta10` from 60 s to 30 s because the median
run's last improvement lands at 12 % of the budget, and a 2x solver means 30 s
now buys what 60 s used to. Taken that way the same statistical power costs half
the wall clock. Left alone, the budget simply buys more search per run, which is
worth having on the hard instances where 16 % of runs were still improving at
90 % of their budget -- and worth nothing on the easy ones, which had already
converged.


## 2026-09-19 — what is left to optimise, and a measurement that was wrong

### Where the time actually goes

Phase timers, `ta01`-`ta10`, 10 runs of 60 s, from the solver's own Runtimes
block. Percentages of a fixed budget are misleading when one build does more
generations in it, so the third column divides by the generations actually
completed:

| phase | original | crisp | ms per generation, original -> crisp |
|---|---|---|---|
| local search | 83.7 % | 78.5 % | 401 -> 208  (**1.93x**) |
| selection | 8.7 % | 13.1 % | 41.8 -> 34.8  (1.20x) |
| crossover | 5.9 % | 5.7 % | 28.4 -> 15.1  (1.88x) |
| replacement | 1.0 % | 1.8 % | 4.9 -> 4.6  (1.05x) |

Selection barely moved, and so grew from 8.7 % to 13.1 % of the run.
`SelectionShuffle::apply` clones all 250 individuals every generation, and a
clone is a deep copy of the schedule; it is bound by allocation and memcpy, not
by the comparisons the refactor made cheap. It sped up 1.20x only because
`ScheduledTaskInfo` shrank from 32 to 24 bytes when `Interval` became `Crisp`.

A gprof profile (`-O2 -pg`, 30 s on `ta01`) puts 35 % in
`NB_ParallelN2_MakespanIJSP::evaluateNeighbour` and about 20 % in the
`ScheduleIJSP` copy constructor -- 6.3 million whole-schedule copies in 30 s,
from `evaluateNeighbour` (2.1 M), `ScheduleIJSP::clone` (3.1 M) and
`acceptNeighbour` (1.1 M).

**One warning about that profile.** gprof attributed those copies to
`ProblemIJSP::clone`, and even showed it calling the random number generator.
It cannot: `ScheduleIJSP`'s copy constructor copies the problem *pointer*, as
the source plainly shows. Both are one-line `clone()` methods defined in
headers and the symbol resolution picked the wrong one. The call counts are
usable, the names are not, and the source has to be read before any of it is
believed.

### Dead end 4: link-time optimisation

The Makefile has no `-flto`, and the hot path is full of one-line accessors
declared in a header and defined in a `.cpp` -- `Individual::getFitness` is
called 171 million times in a 30 s profile -- which no amount of `-O3` can
inline, because the definition is in another translation unit. An obvious win.

Measured against the step-3 build on `ta01`-`ta10`, 10 runs of 60 s: 4.122 gen/s
against 3.767, **+9.4 %**, faster on all ten instances. It went into the
Makefile.

Then the same build was measured again, in its own tree, and reported 3.812
gen/s. Same source, same flags. **The two binaries had the same md5.**

So one identical binary measured 4.122 and 3.812 gen/s in two batches, a 9 %
swing, with 100 runs behind each number. That is not run-to-run variance in the
algorithm -- the seeds are the same and the search is deterministic -- it is the
state of the machine between one batch and the next. **Every comparison in this
directory made by running configuration A and then configuration B is confounded
with when each one ran, and cannot resolve anything below about 10 %.**

Re-run properly, with both builds running *at the same time* on the same
instances, five processes each so the ten fit in fourteen cores:

| | mean gen/s | per-instance wins |
|---|---|---|
| no LTO | **3.922** | 7 of 10 |
| LTO | 3.858 | 3 of 10 |

**0.984x. Link-time optimisation buys nothing here**, and the `+9.4 %` was an
artefact of the design. The flag is reverted; the mechanism is real and the
effect is not, which makes it the fourth entry on that list after the tenure
bound, the longer local search and back-jump tracking.

`scripts/paired_compare.sh` runs two builds side by side and should be used for
every comparison of builds from now on. It does not help with comparisons of
*configurations*, which need separate processes anyway, but those are compared
by solution quality rather than by speed.

### The headline figure, re-measured

The 1.80x reported earlier came from the same block design, so it was re-run
paired, both builds at once on the same instances:

| | mean gen/s | generations per 60 s run |
|---|---|---|
| `experiment/classic-jsp` | 2.071 | 124.3 |
| crisp | **3.837** | **230.2** |

**1.85x**, and the crisp build is ahead on all ten instances. The block-design
figure of 1.80x was not wrong in substance -- the effect is far larger than the
9 % band that design cannot see -- but the number to quote is 1.85x, and it is
worth saying why the earlier one should not be quoted to three decimals.

Quality reproduces too: under the paired design the original build again stops
at 1243 on `ta10` while the crisp build reaches the optimum of 1241, so the
five-optima / six-optima difference is not an artefact of one batch either.

### What is still on the table, and what it would cost

Measured sizes, not guesses:

* **Whole-schedule copying, about 20 %.** `evaluateNeighbour` copies the entire
  schedule to try one move. Applying the move in place and undoing it would
  remove most of 6.3 million copies per 30 s. This is the biggest remaining
  item and also the most dangerous change in the file: it rewrites the code
  that decides which neighbours are accepted, and the trace check would have to
  be extended to every neighbourhood before it could be believed.
* **Selection, 13 %,** and the least improved by the refactor. The 250 clones
  per generation are mostly overwritten by crossover immediately afterwards.
  Reusing a buffer population instead of allocating a new one is a contained
  change with a clear mechanism -- which, on this directory's record, is exactly
  the kind of thing that then measures at zero. Worth trying, worth measuring
  paired.
* **`FitnessCrisp::convertType`,** a virtual `getType()` plus a `dynamic_cast`
  on every fitness comparison, 113 million calls in a 30 s profile, to compare
  two integers. The type check already guarantees the type, so the cast can be
  static. Small, safe, and cheap to try.
* **Profile-guided optimisation** is the other build-flag idea. After the LTO
  result it should be assumed worthless until measured paired.

None of these is needed for the refactor to stand. They are what a second pass
would look at, in that order.


## 2026-09-19 — the dynamic_cast under every comparison

`FitnessCrisp::convertType` guarded its downcast with `f->getType() ==
Type::CRISP` and then did the downcast with `dynamic_cast`, which asks the same
question a second time by walking the RTTI graph at runtime. It sits under every
fitness comparison in the solver -- `isBetterThan`, `isWorseThan` and the three
others all start with it -- and after the crisp refactor the thing it guards is
a comparison of two integers.

The guard is sufficient on its own: `FitnessCrisp` is the only class in the
`Fitness` hierarchy that returns `Type::CRISP`, and nothing derives from it.
(`TimeWindow` also has a `CRISP` enumerator; that is `TimeWindow::Type`, an
unrelated enum.) So the cast becomes `static_cast`.

### Measured, and then measured again

Following the rule from the LTO retraction above, this was measured with
`scripts/paired_compare.sh` and then replicated with the two builds' roles
swapped, in case the launch order mattered:

| | dynamic_cast | static_cast | ratio |
|---|---|---|---|
| first run | 3.603 gen/s | 3.880 gen/s | **1.077x** |
| replication, roles swapped | 3.827 gen/s | 4.190 gen/s | **1.095x** |

`static_cast` is ahead on all ten instances in both runs -- twenty paired
comparisons, twenty wins.

The two runs are also a clean demonstration of why the paired design is
necessary. The absolute rate of the *same* binary moved by 6 % between the two
twenty-minute windows (3.603 to 3.827 for the old build, 3.880 to 4.190 for the
new one), which is the same drift that produced the phantom LTO result. The
ratio moved by 1.7 %. A block design measures the drift; a paired design
measures the change.

Call it **+8 %**. The trace check holds: same seed, same trajectory, generation
by generation.

### The branch, end to end

Paired against the untouched `experiment/classic-jsp`, `ta01`-`ta10`, 10 runs of
60 s:

| | mean gen/s | generations per 60 s run |
|---|---|---|
| `experiment/classic-jsp` | 1.951 | 117.1 |
| crisp | **3.900** | **234.0** |

**1.999x.** The crisp build does exactly twice the search in the same wall clock,
and reaches the optimum of `ta10` (1241) where the original build stops at 1243 --
six known optima against five, reproduced now in three separate paired runs.


## 2026-09-19 — two more nulls around evaluateNeighbour, and where that leaves it

The `convertType` result made the obvious next move look obvious: there are more
`dynamic_cast`s on the hot path, and more heap traffic. Both turned out to be
worth nothing, and the reason is worth recording because it says where the
remaining cost actually is.

### The selection phase fixed itself

Before the `convertType` change the phase timers read:

| phase | ms per generation, original -> crisp | |
|---|---|---|
| local search | 401 -> 208 | 1.93x |
| **selection** | **41.8 -> 34.8** | **1.20x** |

Selection was the phase the refactor had barely improved, and it had grown from
8.7 % to 13.1 % of the run. The plan was to attack the deep copies in it.

After the `convertType` change, with nothing else touched:

| phase | ms per generation, original -> crisp | |
|---|---|---|
| local search | 429 -> 211 | 2.03x |
| **selection** | **44.5 -> 22.2** | **2.01x** |

Selection is back to 8.6 % of the run and scaling like everything else. It was
never bound by copying: ABCPSO re-sorts the population inside the per-individual
loop, every sort is a few thousand fitness comparisons, and every comparison was
doing an RTTI walk. **The candidate disappeared when the cause was removed
somewhere else entirely**, which is an argument for fixing the measured thing
rather than the plausible one.

(The sort itself is not removable. `replaceIndividual` is called inside that
loop, so the population really does change between iterations and the re-sort is
not loop-invariant. Changing it would change the algorithm, and the trace check
that guards this whole refactor would rightly reject it.)

### Dead end 5: the other dynamic_casts, and the heap-allocated pruning bound

`evaluateNeighbour` -- 35 % of the run, entered 2.1 million times per 30 s --
began with

    lowerBound = dynamic_cast<FitnessCrisp *>(this->currentFitness->clone());

a virtual call, a heap allocation and an RTTI walk, for a scratch value used to
abort the evaluation early when the partial schedule is already worse than the
incumbent. `currentFitness` is declared `FitnessCrisp *`, so the cast decides
nothing; and a `FitnessCrisp` is an int and a flag, so it belongs on the stack.
Both were changed, in N1, N2, N3, N8 and NH.

| | mean gen/s | |
|---|---|---|
| before | 4.199 | |
| after | 4.210 | **1.003x** |

Nothing, paired, 100 runs each side. An earlier paired run of the cast alone had
already come out at 0.997x.

The explanation is that the allocation being removed sits next to a much larger
one that stays: the line above it is `new ScheduleIJSP(*this->schedule)`, a deep
copy of the whole schedule -- four vectors, about 6 KB for a 15x15 instance --
and the allocator is being asked for that on every call regardless. Removing a
small fixed-size allocation from beside a large variable-size one buys nothing.

**Both changes are kept**, and the distinction from the reverted `-flto` matters:
those added a build flag for no benefit, these remove code. A static cast is
simpler than a dynamic one, and a stack object cannot be leaked down an exit path
that forgets to free it -- the heap version had three such paths to get right.
They are simplifications that measured zero, not optimisations, and the commit
message says so.

### Where this leaves the remaining 20 %

What is left of the profile's schedule-copying cost is the copying itself: 6.3
million whole-schedule copies per 30 s, from `evaluateNeighbour` (2.1 M),
`ScheduleIJSP::clone` (3.1 M) and `acceptNeighbour` (1.1 M). Two attempts to
shave the overhead *around* those copies have now measured zero each, which is
evidence that the memcpy is the cost and not the bookkeeping.

Removing it is an architectural change, not a micro-optimisation. `evaluateNeighbour`
materialises a full schedule for every neighbour it evaluates and hands it to
`Neighbour::setEvaluation`, which owns it; `acceptNeighbour` then *clones that
again* into the neighbourhood's own schedule. There are two separate things to do:

1. **Transfer ownership on accept** instead of cloning -- the neighbour's
   evaluated solution is discarded immediately afterwards anyway. This is
   contained: one `release` method on `Neighbour` and one line in each
   `acceptNeighbour`. It addresses 1.1 M of the 6.3 M copies, so expect about
   3 % -- which is barely above what the paired design can resolve, and on this
   directory's record that means it will probably measure as nothing.
2. **Do not materialise a schedule to evaluate a neighbour at all**, only to
   accept one. This addresses 2.1 M copies and is the only change here with a
   double-digit prospect. It also changes the interface between the local search,
   the neighbourhood and `Neighbour`, which is shared with the FJSP
   neighbourhoods, and the trace check would have to be extended to every
   neighbourhood before any of it could be believed.

The second is worth doing by someone with a morning in front of them, not at the
end of a session. It is the last item in this directory with a plausible
double-digit gain, and it is also the one most likely to break the search in a
way that only shows up as a worse makespan three hours later.


### Dead end 6: profile-guided optimisation

The other build-flag idea, and the last one. Built in three stages in a scratch
tree -- instrumented with `-fprofile-generate`, profiled on a 30 s run of `ta01`
with the tuned setup, rebuilt with `-fprofile-use` -- and measured paired against
the committed build.

| | mean gen/s |
|---|---|
| plain `-O3` | **4.222** |
| profile-guided | 4.082 |

**0.967x: 3 % slower**, and behind on eight of the ten instances. Not adopted,
and it never touched the repository -- the whole experiment lived in
`/opt/scratch`, which is the right way round for a change that has to earn its
place.

Two build-flag ideas, `-flto` and `-fprofile-use`, both of which had a clear
mechanism and neither of which did anything. The code generated by `-O3` for
this solver is apparently already close to what the extra information would buy,
and the time is going somewhere the compiler cannot help with.

---

## Where the optimisation pass ended up

Starting point for this pass: 3.767 gen/s, the step-3 crisp build.

| change | paired result | kept |
|---|---|---|
| `FitnessCrisp::convertType`, static cast | **1.077x and 1.095x** (replicated, roles swapped) | yes |
| `-flto` | 0.984x | no -- reverted |
| `lowerBound` cast and heap allocation | 0.997x, 1.003x | yes, as a simplification |
| profile-guided optimisation | 0.967x | no |
| selection deep copies | candidate dissolved: the `convertType` fix took selection from 1.20x to 2.01x | n/a |

One change in five did anything, and it was the smallest edit of the lot: one
word, `dynamic_cast` to `static_cast`, in a function of four lines. It was worth
8 %, and it also dissolved the candidate that the phase timers had pointed at
hardest.

**End to end, paired against the untouched `experiment/classic-jsp`: 1.951 gen/s
against 3.900. Exactly 2.00x**, with the same search trajectory and six known
optima of `ta01`-`ta10` against the original build's five on this machine.

The method note that matters more than any of the numbers: **on this machine a
comparison of two builds run one after the other cannot resolve anything below
about 10 %.** Every figure above comes from `scripts/paired_compare.sh`, and the
one result that was adopted was replicated with the two builds' roles swapped.
The first version of this pass reported `-flto` as a 9.4 % win. It was worth
nothing.


## 2026-09-19 — evaluating a neighbour without copying the schedule: +10 %

The last item on the list, and the one held back for a morning with someone in
the room. Done now, with the trace check as the guard.

### What the copy was for

`evaluateNeighbour` in N2 deep-copied the whole schedule, reversed the arc on the
copy, propagated the heads, read the makespan off it, and handed the copy to
`Neighbour::setEvaluation`, which owns it. `acceptNeighbour` then *cloned that
copy again* to make it the current schedule. And `LS_TabuBackJump` clones the
chosen `Neighbour` twice more -- once for `lastNeighbour`, once into the tabu
list -- and `Neighbour`'s copy constructor clones the solution it carries. So
one accepted move cost four schedule copies, and every evaluated-but-rejected
neighbour cost one, whose only reader was a tabu list that needed the arc.

That was the 6.3 million copies per 30 s in the profile: 2.1 M from evaluate,
1.1 M from accept, and most of the 3.1 M attributed to `ScheduleIJSP::clone`
from the two neighbour clones.

### What replaces it

The move is applied to the live schedule and undone. `applyArc` reverses the arc
and runs the same head propagation the copy used to get, logging every write --
the six link fields, the machine's last task if it changes, and each head before
it is overwritten; `revertArc` plays the log back in reverse, so a task whose
head rose in several steps ends on the value it started with. `evaluateNeighbour`
is now apply, read the makespan, revert, and store *only the fitness* in the
neighbour through a new `Neighbour::setEvaluatedFitness`. `acceptNeighbour`
applies the move again for real. Nothing is copied on the hot path at all, and
the neighbours the tabu list clones now carry an int instead of a schedule.

The propagation is a fixpoint computation -- heads are longest paths from the
source, which are unique -- so re-running it on accept lands on the heads the
copy would have held. That is the argument; the trace check is the evidence.

### Verification

* Same seed on `ta01`, full tuned configuration: the crisp build agrees with the
  untouched `experiment/classic-jsp` build generation by generation, best and
  population average, over the whole common prefix.
* Every neighbourhood re-run on both builds at a population of 20 (`Neighbour`
  is a base class they all share): N1, N2, N3, N8, N2Plus, N2Minus and N2Inter
  identical over about 200 generations each. NH still completes no generation on
  either build.
* All certificates verified; the same six optima.

Only N2 was converted. N1, N3, N8 and NH keep the copying protocol; each
neighbourhood's evaluate and accept are a matched pair and nothing outside them
reads the stored solution, so the two protocols coexist.

### Measured

Paired, then replicated with the roles swapped:

| | copy | in place | ratio |
|---|---|---|---|
| first run | 4.042 gen/s | 4.420 gen/s | **1.093x** |
| replication, roles swapped | 3.957 gen/s | 4.365 gen/s | **1.103x** |

In place is ahead on all ten instances in both runs: twenty paired comparisons,
twenty wins. **+10 %**, and after the two nulls that nibbled at the edges of
this copy, the confirmation that the memcpy itself was the cost.

Where the branch stands against the original, by composition of paired ratios:
2.00x from the crisp type and the static cast, times 1.10 from this, about
**2.2x**. A direct paired measurement against the original is the number to
quote and is the next thing to run.


## 2026-09-19 — 2.23x against the original, and the same change under the memetic

### The headline, measured directly

Paired against the untouched `experiment/classic-jsp`, `ta01`-`ta10`, 10 runs of
60 s, tuned ABC configuration:

| | mean gen/s | generations per 60 s run |
|---|---|---|
| `experiment/classic-jsp` | 2.070 | 124.2 |
| crisp, in-place N2 | **4.618** | **277.2** |

**2.23x**, ahead on all ten instances, and the crisp build again reaches the
optimum of `ta10` (1241) where the original stops at 1243: six known optima
against five. The composition of the paired ratios predicted about 2.2x; the
direct measurement is the number to quote.

### Does it hold up under the other algorithm that drives N2?

Every measurement in this directory is ABCPSO. The in-place evaluation changes
the protocol between the local search, the neighbourhood and `Neighbour`, and
the memetic algorithm (`algorithm = MEMETIC`, `MemeticAlgorithm`) drives the
same local search through the same interface, so it is the other consumer that
could be broken by a mistake the ABC would never exercise.

`setup/jsp_ma_n2_60s.txt` is `jsp_abc_n2_60s.txt` with `algorithm = MEMETIC`
and nothing else changed -- the memetic reads the GA keys plus `localsearch.*`
and ignores the ABC-only ones -- so the two algorithms run the same operators,
the same plain tabu search on N2, and the same budget.

**It does not break.** Same seed on `ta01`, the crisp build agrees with the
untouched build generation by generation over the whole common prefix, best and
population average; both certificates verify.

**It improves by the same amount.** Paired, copy against in place, then
replicated with the roles swapped:

| | copy | in place | ratio |
|---|---|---|---|
| first run | 7.706 gen/s | 8.663 gen/s | **1.124x** |
| replication, roles swapped | 8.009 gen/s | 8.966 gen/s | **1.120x** |

In place ahead on all ten instances in both runs; the makespans reached are
identical between the two builds on every instance, as they must be with the
same trajectory. **+12 %** under the memetic against +10 % under the ABC -- the
memetic spends a larger share of its generation in the local search, so it has
more of the copying to lose.

### A side observation, recorded and not claimed

The memetic completes about twice as many generations per minute as the ABC
(462 against 242 per 60 s on the same build) -- its generation is cheaper -- and
at this budget it reaches `ta06 = 1238`, the optimum, and `ta05 = 1231`, which
no ABC configuration in this directory has, while missing `ta08` by one unit.
That is a comparison of two algorithms on ten instances at ten runs, made on the
way to something else, and this directory's own rule says what that is worth:
a hypothesis for a pre-registered run, not a result. It goes here so it is not
rediscovered as a surprise.


## 2026-09-19 — pruning the other problem domains

The repository carried three problem domains in one source tree: the interval
job shop this branch grew out of, the fuzzy job shop (FJSP, triangular fuzzy
numbers), and the fuzzy vehicle routing problem (FVRP). On a branch whose
solver reads crisp integers, the second and third are dead weight that still
has to compile, and that every reader has to step around.

Removed: every `*FJSP*` and `*FVRP*` file, `TFN` and `FitnessTFN`, and
`RobustnessFileWriter`, whose only remaining users were the FJSP analysers --
77 tracked files. Then the shared code that reached into them: the FJSP and
FVRP registrations in the five class registers, the `FitnessTFN` class and the
`FUZZY` fitness type, the `agreementIndex(TFN)` and `delay(TFN)` overloads of
the four time-window classes (the `double` overloads, which the crisp tardiness
uses, stay), and an unused `TFN` local in `Population`.

What stays, deliberately: every algorithm (GA, memetic, Neri, the four ABC
variants, simulated cooling), every selection and replacement operator, and the
whole JSP domain including the creation heuristics and crossovers no setup here
selects. Those are alternatives a setup file can reach; the fuzzy domains were
not reachable from any crisp instance at all.

Verified the same way as every change on this branch: same seed on `ta01`, the
build agrees with the untouched `experiment/classic-jsp` generation by
generation under both the ABC and the memetic; every neighbourhood identical on
the cross-check; certificates verify.
