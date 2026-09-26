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


## 2026-09-19 — IJSP becomes JSP

With the intervals gone the `I` in every class name was a fossil. Renamed, in
one mechanical pass over the source tree: 88 files (`ProblemIJSP` ->
`ProblemJSP`, `NeighbourhoodIJSP_N2` -> `NeighbourhoodJSP_N2`,
`IJSPClassRegister` -> `JSPClassRegister`, `IJSPException` -> `JSPException`,
and so on), the `IJSP` namespace, the leftover `FJSP_SGS_*` macros in the append
SGS, and the registered names a setup file uses: `ijsp.makespan` is
`jsp.makespan`, `ijsp.makespan.n2` is `jsp.makespan.n2`, `ijsp.job-order` is
`jsp.job-order`, `ijsp.jox` is `jsp.jox`. The setups in this directory are
updated; the ones under `experiments/cor_tabu_2026/` belong to the interval
study, target the interval solver, and are left as they were.

The word is not replaced in prose. In README, JOURNAL and SCALING "IJSP" means
the interval problem, and the sentences would stop being true. `CLAUDE.md` is
rewritten where it described the interval and fuzzy code.

Two stale files went with it: `Makefile.asan` and `build_asan.sh`, an
address-sanitiser build that listed `Interval.cpp` and a dozen other sources
that no longer exist on any recent commit, and could not have been run.

A defect of the prune surfaced here and is fixed here: `TimeWindow.h` had no
include of its own and reached the standard headers through `TFN.h`, so with
`TFN.h` gone it no longer compiled from a clean tree. It had not shown because
the prune was built on top of the previous objects. The prune commit therefore
does not build on its own; the include is restored in this one.

One trap for whoever renames or removes sources next: the Makefile `include`s
the generated `*.d` dependency files before any rule runs, `make clean`
included, so a stale `.d` naming a header that no longer exists stops `make`
before it can clean. Delete `*.d` by hand first.

Verified as everything else: same seed on `ta01`, trace identical to the
untouched build under the ABC and under the memetic; every neighbourhood
identical on the cross-check, the crisp side now asking for `jsp.makespan.*`
and the original for `ijsp.makespan.*`; certificates verify.


## 2026-09-19 — measuring the collapse, and pre-registering the next experiment

### The diversity statistic reported zero for everything

`StatisticsHamming` compared every individual with itself: the inner loop
fetched `getIndividual(i)` where it meant `getIndividual(j)`. Any population,
however spread, measured as fully collapsed. The Kendall variant next to it was
written correctly but costs O(n^2) per pair, which at 250 individuals of 225
genes is a billion operations per generation and unusable. Fixed the Hamming
one; the `neri` statistic (`1 - (avg - best) / (worst - best)` on the fitness,
1 when the population sits on its best individual) was already fine.

### What the collapse looks like, measured

One run per algorithm, seed 1, with both statistics on
(`scripts/collapse.py` prints these from the stats CSV):

`ta01`, 60 s:

| gen | ABC best | ABC avg | Hamming | Neri | | MA best | MA avg | Hamming | Neri |
|---|---|---|---|---|---|---|---|---|---|
| 10 | 1258 | 1274 | 0.75 | 0.76 | | 1285 | 1331 | 0.91 | 0.45 |
| 20 | 1258 | 1260 | 0.65 | 0.94 | | 1271 | 1304 | 0.90 | 0.64 |
| 50 | 1254 | 1273 | 0.71 | 0.97 | | 1248 | 1271 | 0.88 | 0.63 |
| 100 | 1254 | 1271 | 0.54 | 0.97 | | 1243 | 1255 | 0.77 | 0.80 |
| 300 | 1254 | 1303 | 0.70 | 0.93 | | 1243 | 1250 | 0.58 | 0.78 |
| end | 1254 (323 gen) | | | | | 1243 (587 gen) | | | |

`ta41`, 300 s:

| gen | ABC best | Hamming | Neri | | MA best | Hamming | Neri |
|---|---|---|---|---|---|---|---|
| 30 | 2177 | 0.91 | 0.97 | | 2235 | 0.95 | 0.43 |
| 75 | 2168 | 0.69 | 0.99 | | 2190 | 0.94 | 0.59 |
| 150 | 2168 | 0.81 | 0.94 | | 2149 | 0.92 | 0.67 |
| 300 | 2168 | 0.81 | 0.95 | | 2126 | 0.86 | 0.85 |
| 400 | 2168 | 0.81 | 0.95 | | **2102** | 0.71 | 0.87 |
| end | 2168 (569 gen) | | | | 2102 (1087 gen) | 0.61 | 0.95 |

Three things are now measured rather than inferred.

The ABC's population sits on its best individual by generation 20-30 (Neri
above 0.9) and the best makespan never moves again: 1254 from generation 50 on
`ta01`, 2168 from generation 75 on `ta41` -- with four minutes of budget left.
The Hamming distance then *rises* again, to 0.7-0.8, because the ABC's scout
phase replaces exhausted food sources with random individuals; that keeps the
genotypes apart without ever producing a better one. Diversity by injection is
not the same thing as a search that is still going.

The memetic collapses later and more gradually (Neri 0.6-0.8 for most of the
run), completes about twice the generations, and on `ta41` keeps improving
until generation 400, two and a half minutes in. It then stops too: 2102 at
generation 400, 2102 at generation 1087.

So in both algorithms the run is over well before the budget is, and what
differs is how long the population stays alive before that. This is one run
each and decides nothing; it is what the experiment below is for.

### Pre-registered: memetic against ABC on the 22 open instances

`PREREG_ma_vs_abc.md` fixes instances, runs, budget, arms, execution, endpoints
and decision rule, and is committed before any run starts. The two setups
differ in one line. `scripts/paired_setups.sh` runs both arms at the same time
on the same instances, because on this machine a wall-clock budget is only
equal if the two runs share the clock.


### Withdrawn twenty minutes in

The PI's objection, on seeing the design: the two arms ran the same
configuration, the one irace had tuned for the ABC on the interval instances.
That compares a tuned ABC with an untuned memetic and says nothing about the
algorithms. Correct, and it should have been obvious from the previous entry,
which credits the tuned parameters with the ABC's only clear gain in this
directory. The runs were killed before any result was read and the partial
results deleted; `PREREG_ma_vs_abc.md` stays, marked withdrawn, because the
record of a wrong plan is the point of writing plans down.

What replaces it has to start one step earlier: each algorithm tuned for
itself, on the crisp solver, at the budget it will be run with -- and a
decision, recorded before tuning, on whether that tuning is done on the target
instances or on separate ones. The irace scenario under
`experiments/cor_tabu_2026/irace/` is the starting point.


## 2026-09-19 — tuning each algorithm for itself, on instances that are not the targets

### The decision, and why it is two decisions

The goal is a better solution on an open Taillard instance. That is proved by
the solution itself -- a schedule anyone can recompute -- and how the
parameters that found it were chosen does not enter into it. So for the record
attempts, tuning on the target instances is legitimate, provided it is
declared.

Choosing *which* algorithm to make those attempts with is a different kind of
claim, one that is supposed to generalise, and there tuning on the targets
would make the comparison measure the tuning. So that step is done on
instances that are not the targets, and the winner's configuration then goes
into the record attempts as the starting point.

There is a practical obstacle: all ten 30x20 Taillard instances are open, so
there is no closed 30x20 instance to train on. `scripts/generate_taillard.py`
draws new instances from Taillard's recipe -- durations uniform in 1..99,
random machine order per job -- with Python's generator rather than his, so
they are fresh draws from the same distribution. Twenty of them, five per size
class of the open set (20x15, 20x20, 30x15, 30x20), seed 20260919, under
`tuning/instances/`.

### The irace setup

`tuning/` holds the scenario for each arm. The runner substitutes irace's
parameters into a template setup and scores a configuration by the makespan of
the schedule it wrote, recomputed from the schedule. The budget per run is
300 s, the budget the tuned configuration will be used with: the journal's
first finding on this line was that parameters tuned under one stopping rule
stopped the run early under another, and the solver has since become 2.2x
faster, so the old values carry no presumption.

The ABC space is the COR-2026 one widened: population size and the two
operator probabilities, fixed there at 250 / 0.9 / 0.10, are tuned here. The
memetic has never been tuned in this repository, so every knob the GA and the
memetic read is open, restricted to the operators registered for the job-order
encoding. Two replacement operators are left out: `generational` and `simple`
abort the memetic with "Population: Access to a non-existing individual", found
by exercising the runner before trusting it. 1000 runs per arm, about six hours
each on 14 cores.

Neither tuning has been started. The parameter spaces are recorded here for
review first.


### What exercising the runner turned up before any tuning ran

irace's `--check` runs the target runner on one random configuration per
scenario. The memetic one crashed the solver: `ReplacementParents` walks the
offspring two at a time and, with an odd population (57), reads one past the
end. Every setup in this repository uses a population of 250, so it had never
happened. Fixed: the loop stops at the last pair and the unpaired offspring is
compared with its own parent alone. Even populations are untouched -- the
memetic's trace at population 250 is identical to the original build's
before and after -- and the odd sizes now run.

Two smaller things: `logFile` in an irace scenario is resolved relative to
`execDir`, not to the scenario file; and the shell's "Segmentation fault"
notice lands on the runner's output, which irace reads as the answer, unless
the solver is run in a subshell whose output goes to the log. Both checks
pass now. R 4.1.2 and irace 4.5 are installed in the WSL distro; `fs`, an
irace dependency, would not build from source without `libuv` and comes from
`r-cran-fs` instead.


### Tuning decisions, recorded before the first irace run

Taken with the PI on 2026-09-19:

* **Budget: about nine hours per arm**, 1500 runs of 300 s each on 14 cores,
  the run count the COR-2026 tuning used.
* **Tune where the open instances have room, and nowhere else.** The README's
  hardness table puts it at 20x20 (7 of 10 open, mean BKS-LB gap 2.9 %) and
  30x20 (all 10 open, 2.7 %); 20x15 and 30x15 sit at 0.5-1.4 %. Each arm gets
  one configuration tuned on the ten generated instances of those two classes,
  `tuning/instances_hard.txt`. The 20x15 and 30x15 open instances will be run
  with it too, but were not tuned for.
* **The local search stays at every generation** in the memetic
  (`localsearch.frequency = period`, `period = 1`), as in the ABC; the
  frequency is not a tuned parameter.

The ABC tuning runs first, the memetic after it, sequentially, so that each
arm has the machine to itself the way the other did.


### 2026-09-20: the ABC's tuned configuration, and the comparison made ready

The ABC tuning finished at 04:49, 14.2 h of wall clock and 123 h of CPU for
1500 runs of 300 s. Elite configuration 136: population 247, elite size 86,
`elite.selection` 1, `maxnumtrials` 35, `jsp.jox` at 0.982, `swap` at 0.1801,
`localsearch.target` 0.4645, tabu search stopped after 15 bad iterations.

What the three surviving elites agree on is more informative than the winner's
exact numbers. All three pick `jsp.jox` and `swap`, a crossover probability of
0.98-1.0, `elite.selection = 1` -- the lowest selective pressure the space
allows -- and a short local search, 10 to 23 bad iterations, against the 20 of
the hand-set baseline and its `localsearch.target` of 1.0. Read against the
collapse curves measured on 2026-09-19, irace has converged on the
configuration that takes longest to collapse and spends the saved time on more
generations rather than on deeper local search. It is the same diagnosis the
diversity statistics gave, arrived at without being told to look for it.

The memetic tuning started straight after and is running.

Prepared meanwhile, and not started:

* `PREREG_ma_vs_abc_v2.md`, replacing the version withdrawn on 2026-09-19.
  Each arm runs the configuration tuned for it; arm B's section and setup are
  written from its tuning log when that finishes, and the document is
  committed in full before the first comparison run.
* `scripts/make_arm_setups.sh`, which builds each arm's setup by reading
  irace's own "Best configurations as commandlines" line and substituting it
  into that arm's tuning template, then fixing what the comparison fixes: ten
  runs from seed 1 and the two diversity statistics. Nothing is transcribed,
  so a setup cannot claim a configuration the tuning did not produce. It
  refuses to write an arm whose tuning has not finished, which is how the
  memetic's setup is absent rather than wrong.
* `scripts/sequential_compare.sh`. The arms may not run one after the other:
  a wall-clock budget turns this machine's 9 % drift between batches into a
  difference in how much search each arm gets, which is the confound behind
  this directory's one retraction. Nor should two batches be stacked on the
  machine. So there is one queue holding all 44 jobs of both arms, alternating
  between them, at most 14 running at a time -- one solver per core. Both arms
  are spread evenly over the same window and the machine is never asked for
  more than it has. The script refuses to start while any other solver or
  tuning is running.

One thing the check caught: the generator dropped the `seed` line. `seed` is
itself a placeholder in the tuning templates, because irace varies it, so the
sweep that removes unused placeholders removed it before the override could
set it. The overrides now run before the sweep. The comparison would have run
without a declared seed and not been reproducible.


### 2026-09-20: the memetic's tuned configuration; the comparison is ready

The memetic tuning finished at 16:52, 12.1 h of wall clock and 124 h of CPU,
1499 of its 1500 runs used. Elite configuration 164: population 99, `jsp.jox`
at 0.9434, `inversion` at 0.0348, `shuffle` selection, `tournament`
replacement, `localsearch.target` 0.7186, tabu search stopped after 23 bad
iterations.

The two arms came out in opposite regimes. All three memetic elites sit at a
small population, 99 to 152, where all three ABC elites sit at 247 to 292 with
the weakest selective pressure the space allows. They also disagree on the
mutation operator -- `inversion` against `swap` -- and agree only on `jsp.jox`
and on a short local search of 15 to 26 bad iterations. A single shared
configuration could not have been right for both, which is what the withdrawn
pre-registration got wrong and what this one is built to avoid.

irace's own mean costs, 1762.43 for the ABC and 1803.59 for the memetic, say
nothing about which algorithm is better: each is a mean over the instance-seed
pairs its configuration survived on inside its own race, a different set of a
different size for each arm, and racing keeps a configuration alive on the
runs where it does well. Recorded in the pre-registration as something not to
be read as a result.

Both setups were generated from the tuning logs and smoke-tested, 20 s on
`ta01`: both run, both emit a feasible schedule that verifies against the
OR-Library data (1240 and 1248 against the optimum of 1231; one run of 20 s on
a closed instance, which is a check that the setups work and nothing else).
The fix from yesterday held -- both carry `seed = 1`.

`PREREG_ma_vs_abc_v2.md` is now complete, with both configurations written
down, and committed before any comparison run. The comparison is ready to
launch on an idle machine: 44 jobs through one queue, both arms alternating,
14 at a time, about 3 hours.


### 2026-09-21: the comparison, and H1 rejected

The pre-registered comparison ran from 20:01 to 00:36 on an idle machine, 440
runs of 300 s through one queue with the two arms alternating, 14 at a time.
Every schedule verifies against the OR-Library data. Full table in
`results/prereg2_comparison.txt`.

**The ABC wins the primary endpoint.** Per-instance mean makespan over ten
runs, paired by instance across the 22 open instances: Wilcoxon W = 30,
p = 0.002, the ABC better on 18 of 22. H1 -- that the memetic reaches better
makespans -- is rejected, and by the pre-registered decision rule the ABC
stays as the base algorithm for the record attempts.

**The margin is small and the secondary endpoint does not separate them.** The
mean of the per-instance differences is 5.4 units of makespan on a mean of
about 1850, 0.29 %. On best-of-runs the arms are indistinguishable: the
memetic wins 9 instances and loses 13, sign test p = 0.523, and the mean best
gap to the lower bound is 4.36 % against 4.47 %. So the ABC is reliably
slightly better on average, and neither is better at producing one good run.
That second fact is the one that matters for beating a best known solution,
and this experiment does not settle it -- the endpoint that decides was fixed
in advance as the mean, and is not being switched now that the other one is
more interesting.

**What the diagnostic runs were worth: nothing.** The hypothesis came from a
single run on `ta41` where the memetic reached 2102 against the ABC's 2168.
Under the tuned configurations the same instance gives means of 2084.6 for the
ABC and 2093.0 for the memetic, with bests of 2067 and 2061 -- both arms now
beat the number that started this, and the ordering it suggested is gone. The
disclosure section of the pre-registration was written so this could be
checked, and it is the third time in this directory that something noticed on
the way to something else failed to replicate.

**What did replicate is the tuning's own finding.** The two algorithms wanted
opposite regimes -- population 247 against 99 -- and the one tuned to the
larger population, the lowest selective pressure in its space and a short
local search is the one that won. The diversity work stands: the collapse is
still the constraint, and neither arm addresses it. Both sit 4.4 % above the
best known solutions, which is where the configuration sat before any of this.

The 22 x 2 collapse curves from these runs are recorded and untouched; they
are the input to the next experiment rather than a result of this one.


### 2026-09-21: the research loop, and I-001's filter

From here on the line runs as a loop with a written protocol,
`RESEARCH_IDEAS.md` at the repository root: one idea per iteration, a cheap
filter on four instances fixed for good that can only discard, a full
confirmation on the 21 open instances with a seed bank, a control cell in
every batch, parameters frozen at irace's configuration 136 (the PI's
decision: retuning per iteration would make each loop too long), and one
experiment on the machine at a time. Three earlier ideas -- back-jump,
path relinking, the memetic -- are entered as discarded with their numbers.

**I-001** is the PI's question: if restarting helps because a run's fate is
decided at the start, should the start be optimised rather than drawn at
random? The seeding study on the interval problem (branch `seeding-study`,
58,560 runs) answers half of it: seeding is worth 0.1-1.0 % at the full
budget and 1.3-5.2 % at a tenth of it, and what helps is the *composition*
of the pool, not the quality of its seeds -- a mixed pool starts worse and
ends better. Yesterday's measurement says the right budget per run is about
an eighth of what we used. Same regime. I-001 seeds the ABC's initial
population with 25 of 247 individuals from the study's bank in five cells
that separate quality (v2 top-250) from spread (v2 max-min 250) from
heterogeneity (the mixed pool), against a random control, at 40/100/150 s
per run by size class, 30 runs each.

The seeded creation of the study was ported as `jsp.seeded` (routing of the
interval instances verified identical to the crisp ones on `ta29`; a job
permutation is a solution of any instance with that routing). The pools are
derived from the read-only bank by `scripts/prepare_pools.py`, md5 recorded.

Filter (ta23, ta29, ta30, ta45), means of 30 runs, seeded minus control:
v2rand -3.56, v2top -2.43, v2maxmin -3.14, mix -2.75. The seeds went in
(best at generation 0 about 1750-1830 against 2025-2130 random). The rule
was "discard if mix - control > +2": it passes, and four instances say
nothing else -- all p >= 0.25, as they must at n = 4. The full run, 105
jobs, about 6.5 hours, is what decides.


### 2026-09-21: half-hour iterations

The PI's objection to the loop as it stood: a step that takes six and a half
hours is not an iteration. The machine has 14 threads, so wall clock is
CPU-hours over 14 and half an hour is 7 CPU-hours. One run on each of the 21
open instances, at the class budgets, costs 0.61 CPU-hours; the 30x20 class
alone, ten instances at 150 s, is 69 % of a full batch. The 6.5 hours were
the product of three factors, and two of them were slack.

Five cells was the first. The acceptance criterion only ever looks at the
proposed cell against the control; the other three cells of I-001 answer a
question -- quality against spread against heterogeneity -- that the cheap
filter can answer on four instances. Confirmations now carry two cells:
90.8 CPU-hours become 36.3, and the decision is untouched.

Thirty runs in one go was the second, and this one is free. Run r of a
solver process is seeded `seed + r` (`EvoLauncher.cpp:95`), so six waves of
five runs with `seed = 1, 6, 11, 16, 21, 26` are *exactly* the thirty runs
of one batch with `seed = 1` -- and, for a seeded cell,
`creation.seed.offset = 5(w-1)` keeps the pool blocks the same too. Each
wave is 6.1 CPU-hours, 26 minutes of wall clock, and it is a complete look
at all 21 instances rather than a prefix of the instance list. The waves
write to their own result tags because `queue_jobs.sh` counts completed runs
per directory, and the analysis sums the `_w*` directories.

Looking after every wave is a repeated test, so it is paid for rather than
taken: Pocock's constant boundary for six equally spaced looks, nominal
two-sided 0.0142 at every look including the last. `scripts/wave_power.py`
simulates the design with the run-to-run spread measured on the I-001 filter
(sd about 8 makespan units against an effect around 3). The type-I rate
comes out at 2.3 %, conservative; power at an effect of -3 is 97 % by the
sixth wave against the 99 % of the monolithic test at 0.05; and 70 % of
those decisions land by the *second* wave, an effect twice that size in the
first. Six full waves are the worst case, not the normal one.

What was actually too small to cut is the filter's thirty runs: at ten, the
standard error of the four-instance mean rises to 1.8 units and the "discard
if worse by more than 2" rule would start discarding neutral ideas. The
filter loses cells, not runs: two cells, four instances, 19 minutes.

I-001 closes as pre-registered -- one batch, five cells, thirty runs -- for
two reasons: it was past two thirds when this was written, and changing a
design after seeing its filter is how a result stops meaning anything. The
waves start at I-002.


### 2026-09-21: an outside reading, and two things already in the tree

The PI asked for a second opinion, so the problem, the frozen configuration,
every measurement, the three discarded ideas with their numbers and the whole
backlog went to an outside model (OpenAI Codex, `gpt-5.6-sol`, web search on).
Briefing and reply are kept verbatim in
`experiments/classic_jsp_2026/reviews/2026-09-21_codex_*.md`.

Its diagnosis is one sentence: the deficiency is not population throughput
nor the quality of the starts, it is that all the intensification uses the
same very shallow N2 trajectory. The evidence it reads that way is ours.
Restarts work because independent starts reach different basins. Mixed seeds
start worse and end better because they widen the basin distribution. Path
relinking is quality-neutral because paths between elites stay inside the
same broad region. Back-jump fails because returning to an earlier point of
a short trajectory creates no new structure. The memetic loses because more
generations are not more independent local-search trajectories. And the
detail that ties it together is one we had in the setup file all along:
`localsearch.bad-iterations = 15`. A tabu call dies after fifteen
consecutive non-improving moves. TSAB and i-TSAB cross worsening regions for
hundreds or thousands. What we call a tabu search may be a mildly
non-monotone descent, which would also explain H-1: back-jumping around a
trajectory that never went anywhere cannot help.

Two of its proposals were checked against the code before being written
down, and the check changed their price.

The first, sampling several critical paths, is inapplicable: N2 already
seeds its queue with every machine-last task whose completion equals the
makespan and walks back through all tight predecessors, machine and job
alike (`NeighbourhoodJSP_N2.cpp:38-88`). That is the critical graph, not a
path.

The second, a richer neighbourhood with reinsertions, is already
implemented. `jsp.makespan.n8` is N2's boundary swaps plus extra-block
reinsertion moves (`NeighbourhoodJSP_N8.cpp:17-22`), which is the
Balas-Vazacopoulos family the review points at, it is registered, and
selecting it costs one line of a setup file. The caveat is real: N8 still
evaluates on a copy of the schedule while N2 evaluates in place, so at equal
wall clock N8 pays a speed toll. Equal wall clock is the honest comparison
and it is ours; if N8 shows through the toll, porting the in-place
evaluation is the obvious follow-up.

So the strongest structural idea available costs no code at all, which is
not how these consultations usually end.

The review also took our measurements apart, and it is right four times.
The best restart length per class was chosen on the same traces that
measured the gain, so there is selection optimism, and "21 of 22" hides both
magnitude and dependence. The 61.8 % after the last improvement is
descriptive rather than proof of waste, because "last" is defined using the
future; what decides an early cutoff is the conditional hazard of another
improvement after s seconds of stagnation. "Throughput is not the
constraint" was overstated: the memetic result says only that generation
count in that architecture does not predict quality. And the acceptance
criterion measures a shift in the mean while the objective lives in the
lower tail, so the protocol now also wants pre-declared tail endpoints: the
probability of matching the BKS on ta30, of reaching BKS+d per shortlist
instance, the best verified makespan after a fixed campaign, and the
expected shortfall in the bottom tenth of runs. The Wilcoxon stays for
choosing an algorithm. The tail is the product.

Backlog after the reading: B-9 tabu depth, instrumented before it is
changed; B-10 N8 instead of N2; B-11 a structured kick with reoptimisation;
B-12 an exact repair window on the incumbent; B-13 heavy-tailed run lengths
instead of adaptive ones. B-3, B-4 and B-5 drop.


### 2026-09-21: I-001 rejected, and the number that archives two more ideas

The confirmation ran from 05:22 to 14:23: 21 open instances, five cells,
thirty runs each, 3,150 schedules, every makespan recomputed from its own
schedule, all verified. The pre-declared endpoint was the mixed pool against
the random control, paired by instance over the 21.

It says no. Mean difference -0.90 makespan units, mix better on 13 of 21,
W = 88.5, p = 0.348. No other cell separates either: v2rand -1.87 and 14 of
21 at p = 0.079, v2top -1.32, v2maxmin -0.67. Best-of-seven over blocks,
which is what a record attempt actually sees, says the same between -0.5 and
-1.8. I-001 is discarded.

The interesting result is not the rejection, it is why. The seeding worked,
spectacularly, at the only place it can work: at generation 0 the seeded
cells start **294 makespan units below** the control on average. By the end
of a forty-to-a-hundred-and-fifty second run, **0.3 % of that advantage is
left**. Whatever this solver is, it is something that erases where it
started from. That single number does more than reject one idea. It archives
B-2, optimising the composition recipe of the pool, and B-3, training the
generator with the ABC's result as reward, because the ceiling on both is
0.3 % of whatever they buy at the start: to be worth three units at the end
they would have to be worth a thousand at the beginning. They come back only
if something first changes the part of the search that does the erasing.

Descriptively, and deciding nothing, the difference lives in 20x20: -2.17
with mix better on 6 of 7, against +0.72 on 30x15 and -0.65 on 30x20. Three
of the four filter instances are 20x20. That is why the filter read -2.75
where the 21 read -0.90, and it is worth writing down as a property of the
filter: it is biased toward the class where this effect was largest, it
still only discards, and its magnitude is not a forecast of the final one.

Nothing gets reverted. jsp.seeded is opt-in from the setup file and the
current configuration keeps creation = jsp.random, so a discarded idea
leaves nothing switched on. The class and the pools stay, because a
controlled start may be wanted later for entirely different reasons.

The batch also leaves the loop its missing reference. The control cell is
now the short-run regime measured with real runs: 21 instances, 30 runs, the
class budgets, and a mean distance of its best run to the BKS of 37.4 units.
Everything from here is measured against that, not against our all-time
bests, which come from far more runs at longer budgets. And B-1 now has one
of its two sides; the other, the 300-second batch, is ten runs against
thirty, so the analysis has to equalise CPU time before it compares
anything.


### 2026-09-21: N8 was wrong, and the certificate caught it

I-002 put the tabu search on `jsp.makespan.n8` -- N2's block-end swaps plus
extra-block reinsertions -- by changing one line of a setup file. The filter
ran from 14:37 to 16:14 and the queue exited 1: verification rejected
schedules. N8 was emitting infeasible solutions, operations overlapping on a
machine, 24 of 30 runs on ta45, 1 of 30 on ta23, none on ta29 or ta30. Every
ta45 violation sat on machine 7 among the same three jobs.

The defect is one missing root in the head propagation of a reinsertion.
Moving task T between newMp and newMs changes the machine predecessor of
newMs from newMp to T, so newMs's head must always be recomputed. The queue
was seeded with T and with oldMs only, and newMs is reached solely by pushing
T's successors, which happens only when T's own head changes. Whenever T's
head came out unchanged, newMs kept the head it had behind newMp, started
before T finished, and the makespan was computed from heads that were too
small. The move therefore looked like an improvement, which is why the search
took it.

Nothing inside the solver would have noticed: it reported these as solutions
and as records-in-waiting. The rule that every makespan is recomputed from
its own schedule, which until today looked like bookkeeping, is what stopped
a run of phantom improvements from entering the line. The smoke test that
preceded the launch is the other lesson: four runs on ta29, and ta29 is one
of the two instances where the defect does not show.

The fix adds newMs as a third root. After it, 72 runs over ta45, ta23, ta29,
ta30, ta41 and ta47 are all feasible and verified.

The filter is void on validity grounds, and the call was made without
computing the comparison, which over a 24-of-30-infeasible cell would have
meant nothing. Its result directories are kept, renamed I-002_voidfilter_*,
so they cannot be mistaken for data. The filter is relaunched from scratch,
this time in six chunks of five runs, with the pre-declared rule and endpoint
untouched.


### 2026-09-21: the ABC does not abandon anything

The PI closed the configuration direction: the algorithm has to change. The
place our own data pointed at was the scout phase. When a food source
exhausts its trials the classical ABC injects a fresh random solution, and
I-001 had just measured that a random start is worth nothing after forty
seconds -- 294 makespan units of disadvantage at generation 0, of which 0.3 %
survives. An individual injected mid-run is born outside every basin the
population occupies and cannot catch up. So `abc.scout = kick` was
implemented: replace the exhausted source with a clone of a random elite
under three mutations, turning abandonment from noise injection into a
restart inside a promising basin, which is the only mechanism this project
has positive evidence for.

The smoke test came back feasible and with identical makespans on three of
the four instances. The solver's own counters, which it has been writing into
every CSV all along, say why: `Total replacements in ABC` is zero. In every
run, on every instance, across all three size classes. With maxnumtrials =
35, no food source ever exhausts its trials, because the counter resets
whenever the source improves, and with the tabu search touching 46 % of the
population every generation under Lamarckism no source survives 35
consecutive failures. The scout phase is dead code. I-003 is withdrawn before
it ran, on validity grounds, without spending a batch.

The measurement is worth more than the idea was. irace swept maxtrials and
chose a value that switches the mechanism off, which means the tuning
preferred an ABC with no scouts; any idea hanging off abandonment is dead on
arrival, and that includes a good deal of the ABC literature. It also means
the population never loses a member, so the only diversity pressure left is
crossover and mutation, while the seeding study documented that the
population collapses. That points at the next idea, which depends on nothing
switched off: enforce a minimum distance in the population, acting at every
insertion, with the hamming and neri statistics the setup already computes
and nobody reads as its instrument.

A second measurement came free, and retires the probe I had launched: an
average tabu call runs between 26 and 41 iterations depending on size class.
The calls do not die after a handful of moves, and they do not die on the two
second cap, which would allow many more; they die on the fifteen
non-improving counter after twenty-six to forty-one moves. Still shallow
beside TSAB, but ten times what the outside review assumed when it read
"fifteen moves".

The kick code stays, with a comment recording that it is unreachable while
abandonment is off, the measurement and the date. Deleting it would delete
the finding.


### 2026-09-21, later: a retraction, and the counter that was never counting

The entry above, "the ABC does not abandon anything", is wrong. It is worth
writing down why, because the mistake is the one this project's central rule
exists to prevent.

The setup says `algorithm = ABCPSO`, and `ArtificialBeeColonyPSO` inherits
from `GeneticAlgorithm`, not from `ArtificialBeeColony`. It is a separate
implementation with its own phases. The kicked scout, and then the plateau
change, went into `ArtificialBeeColony`, which never runs. That is why both
smoke tests returned identical cells, and I read that as the mechanism being
switched off when what was switched off was my own code.

Worse, the number I leaned on was not a measurement. `Total replacements in
ABC` read zero because in the class that actually runs, `abc_replacements` is
initialised to zero and never incremented. I took a figure the solver printed
without checking that anything computed it -- precisely the error the rule
about recomputing every makespan from its own schedule exists to prevent. The
rule now extends to any counter the solver reports.

Instrumented properly, over three runs: the scout step fires 496 times per
run on ta29 and 815 on ta41. So abandonment happens hundreds of times per
run, each time injecting the random solution that I-001 showed is worth
nothing after forty seconds, and I-003's premise was right all along. It is
now ported into the class that runs, and the branch is verifiably live: the
replacement counters differ between cells, 426 against 371 on ta29 and 702
against 492 on ta33, and makespans differ on three of four instances, all
feasible.

The same instrumentation turned up the next idea with its mechanism already
measured. Two sites in that class refuse an offspring for tying the
incumbent's makespan, and the one that matters sits inside the Lamarckian
write-back: when the tabu search improves an individual to exactly the
incumbent value, the improvement is thrown away and a failed trial is
counted. That is 1,324 discards per run on ta29 and 3,415 on ta41 against
54,440 and 140,020 improvements kept -- about 2.4 % of the local search's
useful work discarded, with the count swinging from 71 to 8,073 between runs
of the same instance. On the JSP the plateaus at the incumbent value are
vast, and moving sideways across one is how a search leaves a basin, so the
veto forbids the move that matters. It is a duplicate filter built on fitness
instead of on the genotype: it rejects distinct solutions for tying and
admits clones that differ. Two conditions, no new parameter. It is B-14.

What survives from the retracted entry is only the tabu depth figure, 26 to
41 iterations per call, because `iterationsLS` is incremented in the class
that runs.


### 2026-09-22: N2's estimate was not a lower bound, and the search does not care

The PI pushed back twice: what was being tested were patches, none of them
touching the algorithm's logic, and the key had to be in N2's neighbours, in
how they are ordered and in what order they are visited. Following that took
the line to a defect.

`LS_Tabu` sorts the neighbourhood by the heads&tails estimate and stops the
sweep at the first neighbour whose estimate no longer beats the best real
value found. Sound only if the estimate is a lower bound. On ta29, evaluating
every neighbour, the estimate exceeded the neighbour's true value in 882,481
of 1,362,270 evaluations, 64.8 %. With every tail rebuilt from scratch, 0 of
1,352,160. The cause is the incremental tail maintenance in
NB_ParallelN2_MakespanJSP::acceptNeighbour, whose backward sweep continues
only while a tail changes -- the same incomplete-seeding pattern behind the
N8 defect found during I-002. Stale tails inflate estimates by up to 75 units
and push to the back of the ordering neighbours whose real value beats those
actually evaluated; the pruning then never reaches them.

So the tabu search has not been taking the best move in N2. It takes the best
of a truncated, misordered prefix, tens of millions of times per run.

I-004 tested the repair over six waves, 30 runs per cell, 21 instances, 1,260
runs, zero infeasible. The differences per look were +2.02, +0.84, +0.92,
+1.23, +0.33 and +0.01. At the sixth: +0.01 makespan units, better on 13 of
21, p = 0.677. Rejected by the pre-declared boundary. The lower tail says the
same: the best of thirty improves on nine instances and worsens on eleven.

Two things have to be held together. The defect is real and measured, not a
matter of taste about heuristics. And the search is indifferent to it: making
the estimate exact changes the final makespan by a hundredth of a unit. What
that means is that this algorithm does not care which of N2's good neighbours
it picks.

That sharpens the PI's intuition into the question that survives. If picking
*better* inside N2 changes nothing, what might change something is picking
deliberately *differently* -- a perturbed order or a tie-break with a
criterion of its own, rather than a more correct order. The caveat is that
the defect had been perturbing the order by accident in two thirds of cases,
and the outcome was the same, which bounds the hope for that route too.

The repair stays available as `localsearch.tails = full` and does not become
the default. Not for the cost, which is under 1 %, but for continuity: the
control cell is the loop's reference and has now reproduced four times to
within half a tenth of a unit. Changing default behaviour would break that
comparability for a benefit measured at zero. It will be revisited if an idea
needs the bound to be valid, such as anything using the estimate as a guide
rather than as a filter.

Also measured along the way, and kept as inert counters: N2 offers on average
1.5 to 2.0 eligible neighbours tied at the best value, with up to 27, and
which one is taken is decided today by the random pivot of the quicksort that
sorts the neighbourhood.


### 2026-09-22: the tie-break is discarded, and the filter may be measuring the wrong thing

I-005 broke ties among N2's equally good moves by long-term frequency: among
the neighbours tied at the best value, take the arc used fewest times in the
run. It is the only form of the PI's ordering idea the previous measurements
left open, since I-004 had shown that making the ordering correct is worth
+0.01 units and the tail defect had been randomising the ordering in two
thirds of cases to the same end.

The filter discards it. Means of 30 runs, freqtie minus control: ta45 +7.70,
ta23 +4.20, ta29 +0.60, ta30 -2.30, mean +2.55 against a threshold of +2.0.
The rule is applied as written and not argued with after the fact.

The lower tail, though, moves the other way, and on one instance it moves a
lot. The best of thirty runs improves on two of the four: 1595 against 1607
on ta30, twelve units better and the best this short-run regime has produced
on that instance in the whole line, and 1571 against 1574 on ta23. On ta29
and ta45 the tail worsens.

That does not change the verdict, and saying it is not reopening it. But it
does expose a hole in the protocol that the outside review named on the 21st
and that I had not closed: this line's objective is a record, a record lives
in the lower tail, and every criterion we have used measures a shift in the
mean. A mechanism that worsens the mean while widening the lower tail is
exactly what a record hunt wants, and our filter throws it away.

So the protocol now asks every batch for two pre-declared tail endpoints
alongside the mean: the best of the thirty runs per instance, paired by
instance with a sign test, and the mean of the three lowest makespans as an
expected-shortfall proxy. The filter rule itself does not change -- it stays
on the mean, cheap, and discarding only. An idea discarded on the mean whose
tail endpoints improve is still not accepted, but it is tagged in the history
and enters the backlog as a candidate for a record attempt, which is the one
setting where tuning on the target instances is legitimate. The change is
declared now and applies from I-006; I-005 gets the tag as an annotation
only, with no effect on its verdict.

Nothing is reverted. localsearch.tiebreak defaults to first and the current
configuration does not mention it, so the code sits inert alongside
jsp.seeded, abc.scout = kick and localsearch.tails = full: four measured
mechanisms, none of them switched on.


### 2026-09-22: a record attempt, two units short, and a better best on ta23

Six hundred verified runs on the four instances whose best sits closest to a
BKS, seventy-five per cell on each, zero infeasible. No record. The closest
was ta29 at 1627 against a BKS of 1625, two units away, from the control
cell; nothing else came within two.

What the batch did produce is a better best of our own on ta23: 1564, seven
units below the 1571 the line had carried since the comparison in September,
verified from its own schedule with the certificate kept in
iter/I-006/evidence/. It is still seven above the BKS of 1557, so it is not a
record and is recorded as what it is.

The batch also settled the question it was designed to double as. I-005 had
been discarded on the mean while its lower tail went the other way on ta30,
1595 against 1607, and the protocol had been changed to stop throwing such
things away. Running both cells over four instances and 600 runs says that
advantage does not generalise: it repeats in direction on ta30, the one
instance where it was seen, and the pooled sign test on the best of blocks of
five is 26 against 30 with p = 0.689. It was one instance plus noise. The
tail tag on I-005 is withdrawn, though the protocol change stays, because
measuring the tail remains the right thing for this objective.

Where that leaves the hunt is clearer than before. ta29 at two units with
seventy-five forty-second runs is the closest this regime has come, and the
line has already matched 1625 once at three hundred seconds. Both facts point
the same way: on ta29 the record is within reach of more volume rather than of
another mechanism. A concentrated attempt that spends the whole budget on
ta29 instead of splitting it four ways is the obvious next step, and it needs
no new idea.


### 2026-09-22: ta29 matched again, and the run length that a record needs

Three hundred and seven runs on ta29, the whole budget on one instance, split
three ways by per-run budget at equal total CPU. No record: nothing came in
strictly below the best known 1625. One run matched it, verified, from the
300-second cell, and its certificate is kept under iter/I-007/evidence/. The
line had matched 1625 once before in the long regime, so this reproduces that
rather than extending it.

Before the numbers, a correction to my own analysis. The first version of the
script printed the match as a record and announced RECORD FOUND, because the
record label tested `value <= BKS` and left the equality branch unreachable. A
record is beating the best known solution, not equalling it. The script is
fixed and the mistake is written down, because it erred in the direction of
overselling the result.

The measurement that survives the absence of a record is the run length. At
equal CPU on ta29: forty seconds by two hundred runs reached 1627, a hundred
seconds by eighty runs reached 1627, three hundred seconds by twenty-seven
runs reached 1625. The three means are indistinguishable -- 1640.5, 1639.8,
1639.4 -- and the minima are not. Only the long budget touched the best known
value, with seven times fewer runs. The budget chosen on the mean is the wrong
budget for a record hunt, and that is now measured on the instance that
matters instead of assumed.

The honest qualifier is that this is one instance and one lucky run out of
twenty-seven. With n = 1 in the winning cell it is a strong indication of
where to spend, not a test. What it licenses is declaring three hundred
seconds in the next record attempt; it does not license rewriting the
per-class budgets, which stay untouchable from the loop and stay tuned to the
mean.

The next step needs no new idea: the whole budget on ta29 at three hundred
seconds a run, eighty-four runs in seven CPU-hours, three times what this
batch gave the only configuration that has touched 1625 twice.


### 2026-09-22: brute force on ta29 closes, and yesterday's conclusion is downgraded

Eighty-four runs of 300 s on ta29, seven CPU-hours, zero infeasible. No
record, and this time no match either: the batch minimum is 1628.

The part worth writing down is the correction. Yesterday I-007 concluded that
the long budget gives the lower minimum at equal CPU, on the strength of its
300 s cell reaching 1625 in 27 runs while 40 s and 100 s stopped at 1627. With
eighty-four more runs of that same configuration the 1625 does not repeat. It
was a lucky draw. I had flagged the n = 1, but the conclusion was written with
more weight than it could carry. Put as hits per CPU-hour, which is the
comparison I-007 should have made: at every threshold except that single 1625
event, the short and medium budgets produce more hits per CPU-hour than the
long one. The claim that long runs are the right budget for the minimum is not
established, and the choice of 40 s on the mean is not refuted.

What is established is the other outcome the pre-registration named. Between
I-007 and I-008 there are 111 runs of 300 s on ta29 with one match and no
1624; adding the 200 runs at 40 s and the 80 at 100 s makes 391 verified runs
on this instance without a single value below 1625. ta29 sits against a hard
barrier at 1625 for this configuration, and beating it is not a matter of
volume. Brute force on ta29 closes here, with a number behind it rather than
from fatigue.

The map is much better drawn than when the loop started. Where an individual
starts does not matter (I-001, I-003). Which of N2's good neighbours is taken
does not matter (I-004, I-005). A richer neighbourhood does not help (H-4,
reproduced in I-002). Volume does not break ta29's barrier (I-006, I-007,
I-008). What no iteration has touched is what happens to a move once chosen:
the effective depth of the trajectory, which H-5 closed only as a global
parameter and which remains open in its asymmetric form, and the families the
outside review named that require leaving the complete-schedule neighbourhood
altogether.


### 2026-09-22: the depth of this tabu search is capped by dead ends, not by its parameter

Four redesigns of one idea, each forced by a number, ending in a finding that
reframes two entries of the history.

The idea was the last untouched corner of the map: what happens to a move once
chosen. The local search is about 242 shallow dips a generation, each dying
after 15 non-improving moves having made 26 to 41 in total, and there is never
one deep trajectory, which is exactly what distinguishes TSAB.

I-009 fired one deep call per run. Its filter read -0.02, and the reason was
arithmetic: solving for the deep call's share of the tabu work from the rise
in average iterations per call gave 0.06 % to 2.3 %. Withdrawn as underpowered
by construction, not for its sign. I had bounded the cost and bounded the
effect along with it.

A quota of 25 % of local-search time replaced the single shot. It reached
0.03 %, because the once-per-episode gate limited opportunities rather than
cost and episodes are few. Letting the quota be the only limiter brought 40 to
150 calls a run and still 0.3 %, because of scale: a tabu iteration costs
about 1.7 us here, so the shallow calls add up to some 20 million iterations a
run and a thousand-move trajectory is 0.005 % of that. Raising the depth from
1000 to 30000 changed neither the share nor the average iterations at all.

That was the finding, and it took instrumenting the reason. Every deep call
ends in a dead end, after 75 to 92 moves: a state where no neighbour is
admissible because every critical-block move is tabu without meeting the
aspiration criterion or is the reverse of the last one. LS_Tabu treats that as
the end of the call.

So localsearch.bad-iterations has an effective ceiling near 80. Above it the
value is irrelevant -- 15, 1000 and 30000 give the same trajectory because it
dies first. irace's sweep from 5 to 40 fell entirely below that ceiling, so
H-5 stands for the range it swept and says nothing about greater depths, which
were unreachable by construction. It also explains H-1: back-jumping to a
better point cannot help when the trajectory never goes anywhere, and with an
eighty-move ceiling there is no excursion to return from.

The classical escape works, and works hard. Taking the best neighbour anyway
when none is admissible, instead of ending the call, lifts trajectories from
47-102 moves to 286-1909, and the declared 25 % quota finally binds: 14.0 % on
ta29 and 25.3 % on ta41, with 21,333 and 282,854 escapes a run.

I-010 tests that, with three nested cells in the filter -- control, escape
alone, escape plus the deep quota -- so the filter separates what the escape
gives from what the depth it enables gives, and two cells in the waves.

The protocol gains a rule these four rounds paid for: no idea is
pre-registered without a counter showing that the mechanism receives the share
of the computation the hypothesis assumes. Checking that the branch executes,
which was I-003's lesson, is not enough. Checking how much it weighs is the
rest of it. Without that counter I would have launched four filters and
recorded four zeros.


### 2026-09-22: depth reaches the search and the search does not care

Six waves, 30 runs a cell, 21 instances, 1,890 runs, zero infeasible. The deep
escape against the control per look: +0.16, -0.32, -0.13, -0.06, -0.41, -0.25.
At the sixth, -0.25, better on 10 of 21, p = 0.639. Rejected.

This zero is worth more than the earlier ones. Those were mechanisms that did
not act, or acted where nothing depended on them. Here the counters say the
mechanism acted at full strength: hundreds of thousands of escapes a run,
trajectories of 286 to 1909 moves instead of 47 to 102, and the declared 25 %
quota binding. The depth of the tabu trajectory, which is what distinguishes
TSAB, buys nothing inside this ABC. Together with I-004 that says this search
tolerates an estimator violating its bound in two thirds of cases and
tolerates trajectories forty times longer, without the final makespan
noticing.

The informational cell says something else. The escape alone comes out at
-1.10 on the mean and better on 15 of 21, against 10 of 21 for the full
mechanism. Nothing is concluded from it: it rode along without a boundary and
the pre-registration said in as many words that this case would become a new
pre-registration with its own confirmation rather than a conclusion drawn
here.

That resolves I-011's composition rule, and the two halves of the rule pointed
different ways. It said the dead-end switch joins the portfolio if I-010
closes without crossing its boundary, "that is, if it is measured mean-neutral
over the 21". But I-010's boundary applied to the combination, not to the
escape on its own, and the escape on its own does not look neutral. What was
confirmed neutral is the combination. So the switch does not join the
portfolio, on the rule's stated intent rather than its letter, and the
divergence is written down along with which half was followed.

It also changes the order. I-011 needs to know whether the escape is neutral
or good, so I-012 comes first: the escape alone as a position idea with its
own confirmation. Neutral, and it joins the portfolio. Good, and it enters the
current configuration and the portfolio is built without it.


### 2026-09-22: the signal did not reproduce, which is the result

Six waves, 30 runs a cell, 1,260 runs, zero infeasible. The escape against the
control per look: +1.30, +1.09, +0.59, +0.27, +0.39, +0.07. At the sixth,
+0.07, better on 10 of 21, p = 0.835. Rejected.

Set beside what prompted the batch: I-010's informational cell gave -1.11,
better on 15 of 21, p = 0.0325, on the same 21 instances with the same 30 runs
and the same budget. The only difference is the seeds. The signal vanished
entirely.

That is this protocol's whole argument demonstrated on our own data, and it is
worth writing down because it is the kind of thing that gets published without
anyone noticing. The 0.0325 came from a cell that rode along without a
boundary, declared as information, and was looked at because it stood out
among three. A p chosen that way is not a p: it is the minimum of several, and
its distribution is not the one the test assumes. Ninety minutes of machine
time settled it. Without the protocol it would have cost a false claim in a
paper.

It also settles I-011's composition with a measurement instead of an
interpretation. Closing I-010 I left the dead-end switch out of the portfolio,
reasoning that what had been confirmed neutral was the combination rather than
the switch, and that the switch did not look neutral. The switch now has its
own confirmation with a boundary over the 21 instances: +0.07 at p = 0.835. It
is mean-neutral as the rule requires, so it joins. The earlier decision was
right on the information available then and this replaces it with better
information rather than correcting an error.

The portfolio therefore has four switches and sixteen combinations, every one
of them confirmed mean-neutral over the 21 instances: seeded creation, full
tails, the kicked scout and the all-tabu escape.

## I-011's filter passes its rule while its mechanism does not move

Sixty-four jobs, sixty-four certificates, no infeasible schedule. The
(results tag, instance) pair was unique this time and the analysis read all
240 runs it was supposed to read.

The mechanism check comes first, as pre-registered, and it is flat. The
portfolio spreads wider than the control on two of four instances, mean ratio
1.03. The whole hypothesis was that a mixture of mean-neutral components keeps
the mean and adds between-component variance, lengthening the lower tail by
construction. At ratio 1.03 that between-component variance is
indistinguishable from zero, so the hypothesis was never exercised.

The endpoint moved anyway. Best-of-five, portfolio minus control: ta23 -2.00,
ta29 -3.00, ta30 -6.83, ta45 +1.67, mean -2.54 against a rule that discards
above +2.0. It passes. The mean moved -2.16, and this iteration was allowed to
worsen the mean, so that number decides nothing. The filter has no boundary; it
only discards, and it did not discard.

Where the movement does not come from, all checked against data already on
disk. No switch carries it: the on-minus-off contrasts over the fifteen
combinations are -0.48 for seeding, -0.41 for tails, -0.28 for the kick, and
+1.83 for the escape, the only large one going the wrong way. Combination 0 is
the frozen configuration, the control itself, and it measured -0.97 against the
control; a shift that includes the cell identical to the control is not a
treatment effect. And there is no within-process drift, which was the
structural candidate: the control runs as one 30-run process and the portfolio
as fifteen 2-run processes, and since the budget is wall-clock, decay with run
index would have handed the portfolio exactly this shift. The measured slope is
+0.011 units per run index, +0.31 across thirty.

What the per-combination table does show is its own noise. Combination 0 runs
seeds 1 and 2 under the control's configuration, and the control's own runs 1
and 2 use those same seeds; they differ by 0.9 units. The budget is wall-clock,
so a run is worth whatever it computes in its seconds and the same seed does
not reproduce the same number. With two runs per cell the expected spread of
the fifteen means is about 2.8 and the observed spread is 3.13. That table
resolves nothing and is not interpreted.

What remains is an aggregate -2.16 at about 1.9 standard errors with a flat
mechanism check. That is not a finding; it is precisely the state in which a
filter should send an idea to the waves, where the boundary lives. It goes,
with the note that a crossing will need an explanation, because the
pre-registered one no longer applies.

From wave 1 both cells are chunked the same way, one run per job. In the filter
the control was a single 30-run job that held one slot for 75 minutes while the
other thirteen drained, so the batch lasted as long as its longest job. The
change touches neither configuration nor seeds, brings a wave from about 41
minutes to about 29, and removes the last structural asymmetry between the
cells.

## I-011 is rejected, and its mechanism never moved once

Six waves, 1260 confirmation runs, thirty per cell and instance, no infeasible
schedule. The tail endpoint, mean of bests over blocks of five, portfolio minus
control, across the six looks: +1.43, -0.43, -1.46, -0.79, -0.79, -0.32, with
p at 0.574, 0.602, 0.106, 0.213, 0.192, 0.777 against a symmetric Pocock
boundary of 0.0142. No crossing in either direction, so the idea is discarded.

The number to read is not the rejection but the dispersion ratio: 1.03, 1.03,
1.14, 1.09, 1.04, 1.05, plus 1.03 in the filter. Across seven independent
readings the portfolio never spread meaningfully wider than the control. The
hypothesis was arithmetic: a mixture of mean-neutral components keeps the mean
and adds between-component variance, so its lower tail is longer by
construction. The arithmetic is right; the empirical premise is wrong. Adding
between-component variance requires the components to produce different
distributions, and if the four switches produce essentially the same
distribution, the mixture is the frozen configuration itself. That is what
I-001, I-003, I-004 and I-012 measured when they called the switches neutral,
and I read "neutral in the mean" where the data said something stronger.

The lesson outlives the iteration: neutral in the mean and equivalent in
distribution are not the same thing, but in these four cases they turned out to
be, and a portfolio can only lengthen a tail if its components differ. Before
proposing another mixture, check first that the components have different
distributions, which is cheap on data that already exists, instead of assuming
it.

The filter keeps its place. It returned -2.54 in favour with the mechanism
already flat, and six looks returned -0.32. This is the second time in this
loop that an unbounded measurement failed to reproduce under a boundary: the
first was I-010's informational cell at -1.11, 15 of 21, p = 0.0325 against
I-012 at +0.07, 10 of 21, p = 0.835. Two out of two. A filter exists to discard
cheaply, not to suggest findings, and when look 3 produced 15 of 21, the exact
shape that fooled I-010, the boundary did its job and waited.

Nothing to revert: I-011 was a job-generation matter and never touched the
solver. The four switches remain implemented and off by default.

No records: the best of the 1260 runs per instance is in iter/I-011/records.txt
and none is at or below its best known solution, as expected from waves that
run the per-class budgets of 40, 100 and 150 seconds rather than the 300 of a
record attempt.

## I-013 is discarded, and it is the first measurement in this loop with a number in it

240 jobs, no infeasible schedule, thirty runs per cell and instance.

The mechanism was exercised with room to spare, which is the first thing to
check: the first-improvement branch takes 69.7% of the moves, the sweep goes
from evaluating 20% of the neighbourhood to 55%, and tabu iterations per call
rise from 26-42 to 39-61, because improving more often resets the
non-improving counter. This is not an idea that failed to fire.

Per-instance means, first minus control: ta23 +29.27, ta29 +9.20, ta30 +16.90,
ta45 +30.83, mean +21.55 against a rule that discards above +2.0. Discarded, by
a factor of ten and on all four instances.

The number matters more than the discard. Twelve iterations measured zeros:
+0.01, +0.07, 0.9 units, flat dispersion ratios. The first one to touch the
decision rule rather than the data feeding it jumps to +21.55. The order in
which N2's neighbours are visited does decide the result, and decides it
heavily; the standing rule, take the best, already sits at the good end of that
lever and is worth about 21 makespan units against the natural alternative.

Attribution is clean. The cell changes rule, order and prune together, but the
last two exist only to find the best: under a best rule they do not change
which move is taken except through the broken bound, and that is exactly what
I-004 measured at +0.01 over 1260 runs. The 21 units belong to the rule.

Five measurements now cover the neighbour-order avenue: I-004 made the sweep
see the true best, +0.01; the tails defect scrambled the order in two thirds of
cases with no effect; I-005's tie-break gave +2.55; the probe found the choice
wrong a quarter of the time and correcting it changed nothing; and I-013
abandoned the rule and lost 21.55. Read together they say something more useful
than "order does not matter": inside the best rule everything is flat, and
outside it a great deal is lost. The surface is flat around the greedy optimum
and falls away as soon as you leave it. That closes the question of which
neighbour to take with a positive measurement rather than a zero, and sends the
focus to what is done with the chosen move and how long each run is given.

Nothing is reverted from the records. localsearch.select stays implemented and
off by default, like tails, scout, deadend and tiebreak, and the frozen
configuration walks exactly the same path as before.

## I-014's filter passes, and this time the mechanism is genuinely exercised

240 jobs, no infeasible schedule, thirty runs per cell and instance.

The mechanism check comes first and is finally satisfactory: 1.36 stall
restarts per run on average, 1.07 on ta23, 1.90 on ta29, 1.27 on ta30, 1.20 on
ta45, against 0.00 in the control. At the withdrawn 0.4 threshold it was 0.25.
The rebuilds cost almost nothing: generations per run rise rather than fall,
90.4 to 101.8 on ta23 and 228.5 to 243.5 on ta45, and tabu iterations per call
move by under 3%. Repopulating around the incumbent is cheap, as it should be
when there is no ground to make up.

Per-instance means, restart minus control: ta23 -0.50, ta29 -0.90, ta30 +0.37,
ta45 -1.77, mean -0.70 against a rule that discards above +2.0. It passes.

That is all it says. A -0.70 over four instances sits inside the noise, the
filter's aggregate standard error being about 1.0, and this loop has already
seen two unbounded measurements fail to reproduce under a boundary: I-010's
informational cell at -1.11, p = 0.0325 against I-012 at +0.07, p = 0.835, and
I-011's filter at -2.54 against its six looks at -0.32. A filter discards
cheaply; it does not suggest findings. The waves decide.

## I-014 is rejected, and it is the cleanest zero this loop has produced

Six waves, 1260 confirmation runs, thirty per cell and instance, no infeasible
schedule. Per-instance mean, restart minus control, across the looks: -0.70,
-0.37, -0.15, -0.17, -0.10, -0.16, with p at 0.186, 0.271, 0.805, 0.664, 0.702,
0.516 against a symmetric Pocock boundary of 0.0142. No crossing; discarded.
All 21 final differences lie between -1.67 and +1.30.

The zero is clean, which is what makes it worth having. I-014 is the first idea
in this loop to reach the waves with both halves of its mechanism check green:
it fired at every look, 1.01 to 1.36 restarts per run, and it took no work away
from the tabu search, since generations per run went up rather than down when
repopulating around the incumbent spared any ground to make up. It was not
under-exercised and it did not cost more than it returned. It was exercised in
full and did nothing.

The stall hazard measures about zero over the last 18% to 44% of every run, so
that stretch really is sterile. I-014 offered it the best exit available without
touching the metric, a restart from the incumbent with diversity and no
catch-up cost, and it did not help. The incumbent of a stalled run sits in a
basin that neither a 1-to-10-move perturbation nor the search that follows it
can improve on in the time left. That agrees with the kicked scout (I-003,
flat), the all-tabu escape (I-010 and I-012, flat) and leaving the greedy rule
(I-013, +21.55): everything tried so far to move the search away from where it
settles either does nothing or makes things worse.

The correction made before launch is vindicated after the fact. The cold form
was withdrawn because the traces said a fresh population could not reach the
incumbent in the budget left. The warm form, which had no such handicap, gives
zero; the cold one would have given zero or worse, at the cost of the same
filter and probably six waves to measure what was already known.

Nothing is reverted from the records. abc.restart stays implemented and off by
default like the switches before it. No records: the best of the 1260 runs per
instance is in iter/I-014/records.txt, the closest being ta29 at 1629 against
1625.

## I-015's filter passes with the most favourable figure yet, which calls for more caution, not less

240 jobs, no infeasible schedule, thirty runs per cell and instance.

The mechanism is green in both halves. The allow cell admits 2457, 5252, 2010
and 2394 plateau moves per run in the local search on ta23, ta29, ta30 and
ta45, and 265 to 426 in the crossover, about 3000 per run that the control
throws away. They cost nothing: generations per run rise, 94.8 to 99.2, 100.1
to 103.4, 91.0 to 96.4, 227.9 to 234.5. The 4 to 7 percent drop the pre-launch
check showed was single-run noise.

Per-instance means, allow minus control: ta23 -2.67, ta29 -0.03, ta30 -3.00,
ta45 -3.83, mean -2.38 against a rule that discards above +2.0. It passes.
Every instance moves in favour or ties, and best-of-five moves the same way,
-2.87, with ta30 at -6.17.

This is the most favourable filter in the loop, and that is a reason for more
caution. I-011's filter gave -2.54, nearly the same, and its six looks ended at
-0.32. Two things set this one apart and neither is evidence: I-011's mechanism
was already flat in its own filter while I-015's is exercised hard and at no
cost, and I-011's effect was carried by no single switch while here there is
only one possible cause. None of that replaces the boundary. The waves go, and
the sixth look or a crossing decides, whichever comes first.

## The PI's circle seeding, and the biased random creation it uncovered

The PI proposed seeding the population so that every area of the search space
is represented: picture the space as a circle, cut it into 247 equal arcs, let
each cut be one initial solution, and rotate the circle for every seed. It is
realised exactly. Operation sequences are numbered in lexicographic order, from
0 to (nm)!/(m!)^n - 1, about 10^501 for 20x20, and an exact integer unranking
turns a position on the circle into its sequence, checked by enumerating all
90 sequences of three jobs by two. Circle g places 247 points at equal arcs
from a golden-ratio rotation, and run g reads circle g through the seeded
creation.

Coverage before decoding is identical to random sampling: for 4000 random
probes, the mean distance to the nearest seed is 0.9169 for the circle and
0.9168 for random seeds on ta23, 0.9443 for both on ta45. In a space of 10^500
points no 247 of them cover anything, however they are placed. A random
population already sits at 98.5% to 99% of the largest diversity any
population can have under the solver's own measure.

After decoding there was a surprise. The circle starts much better, an average
makespan near 2127 on ta23 and 2754 on ta45 against 2269 and 2952 for the
frozen random creation, at the same diversity. The obvious explanation, that
equal arcs spread each job more evenly along the sequence, is false: prefix
imbalance is 3.70 against 3.72. The decisive control settles it: uniformly
random sequences through the same seeded path start just as well, near 2117
and 2753. The gain is not the circle.

It is the solver's random creation, which is biased. jsp.random draws each next
job uniformly among the jobs that still have operations, regardless of how
many, so jobs drawn early run out early and the tail of every sequence fills,
in blocks, with the last operations of a few jobs left behind. A uniform
permutation draws in effect in proportion to what each job has left and
corrects itself. The bias costs 6% to 7% at generation 0, and it matters
because the ABC's scouts draw through the same creation, 500 to 800 times a
run.

creation.random.draw = uniform fixes it, drawing each job in proportion to its
remaining operations. The default is untouched and verified to the unit: the
control's generation 0 repeats its earlier values exactly under the same seed.

I-016 has three cells: control, the circle seeding the first population with
the scouts left as they are, and the uniform draw, which reaches both the first
population and every scout. The risk against both was measured by I-001: a
294-unit head start at generation 0 survived as 0.9 units at the end, and the
head start here is 140 to 200. The uniform cell has what no seeding had, an
effect that does not end at generation 0.

I-015 pauses between its second and third looks, which the Pocock design
tolerates at no cost, so that two experiments never share the machine.

## I-016's filter passes both cells, and what it measures is I-001's lesson again

360 jobs, no infeasible schedule, thirty runs per cell and instance.

The mechanism is green in both cells: the average makespan of generation 0
falls by about 156 units on every instance, from 2949 to 2752 on ta45, and the
circle and the uniform draw start identically, 2752.2 against 2751.7, as the
earlier measurement predicted. What improves the start is uniform sampling,
not the circle.

At the end of the run nothing is left. Per-instance means against the control:
circle +1.43, -0.80, -1.90, +1.27, mean +0.00; uniform +4.10, -0.93, -1.63,
+1.30, mean +0.71. Neither exceeds +2.0, so both pass and go to the waves with
the boundary split to 0.0071 each. A filter only discards; it neither accepts
nor rejects a zero.

What it already says, without a boundary and claiming no more: a 156-unit head
start at generation 0 evaporates completely, as I-001's 294 did. The uniform
cell, which also improves the 500 to 800 scouts of every run, leaves no trace
on the mean either, so a scout that arrives in better shape does not change
where the run ends. That agrees with everything before it: what decides the
result is what the local search does during the run, not where it starts or
what it is fed.

I-015's looks 3 to 6 run first, then I-016's six waves, whose jobs are already
generated and verified with circles for all 21 instances.

## I-015 is rejected with the most favourable final of any iteration

Six waves, 1260 confirmation runs, thirty per cell and instance, no infeasible
schedule. Per-instance mean, allow minus control, across the looks: +1.46,
+0.62, +0.22, +0.50, -0.38, -0.78, with p at 0.237, 0.848, 0.674, 0.715, 0.434,
0.122 against a symmetric Pocock boundary of 0.0142. No crossing; discarded,
and there is no seventh look.

It is the most favourable final of any iteration, which deserves stating both
ways. For it: the mean fell across the last three looks, the treatment wins on
14 of 21 instances, and the large favourable differences (ta43 -4.33, ta45
-3.80, ta48 -3.60, ta23 -3.20) outweigh unfavourable ones that never pass
+2.77. Against it: p = 0.122 sits nearly ten times the boundary, the filter
promised -2.38 while the first looks ran the other way, and this loop has twice
seen a signal of this shape fail to reproduce. The decision is the rule's. If
the question is ever reopened, the right move is not another look at these data
but a replication with fresh seeds, pre-registered as its own idea, as I-012
did for I-010's signal.

The mechanism was exercised throughout, 2149 to 2879 plateau moves admitted per
run at no cost in generations. abc.plateau stays implemented and off by
default. No records.

Correction to the I-015 closing: there are no records, but there are two
matches of ta30's best known 1584, both in the allow cell, at 40 seconds per
run: one in the filter and one in wave 5. The two schedules are identical, both
verified by the independent checker against the original OR-Library data and
kept in iter/I-015/evidence/. The control matched none. Two against zero over
60 runs per cell is nothing statistically (Fisher p = 0.50) and changes no
decision; it is recorded because it fits the mechanism, moving along the
plateau at the incumbent's value, and because it is the loop's second and third
match of ta30. The records script had looked only at the waves, so the first
closing said "no records" without mentioning the match.

A second correction to the same note: the two ta30 schedules are identical,
and it is one match seen twice, not two. The filter's job 25 and wave 5's job 5
carry the same seed, 25, because the filter uses seeds 1 to 30 and the six
waves cover the same 1 to 30 again; with the same seed and configuration the two
runs took the same path to the same schedule despite the wall-clock budget. One
against zero means nothing.

It also exposes a design fact that holds for every iteration: on the four
filter instances the filter's runs and the waves' runs are not independent,
since they share seeds. No decision is affected, because the boundary uses only
wave data, but it partly explains why filter and waves agree on those
instances more than fresh seeds would, and future filters should draw seeds
outside the waves' range.

## I-016 is rejected in both cells: a better start does not help, and leaned against at every look

Six waves, three cells, 1890 confirmation runs, thirty per cell and instance,
no infeasible schedule, the boundary split to p <= 0.0071 per treated cell.
Circle minus control across the looks: +1.94, +0.98, +1.38, +2.19, +1.79,
+1.28, final p = 0.054. Uniform minus control: +3.22, +1.95, +1.74, +2.01,
+1.03, +0.82, final p = 0.299, having come within one step of the control-side
boundary at look 4 (p = 0.011). Neither crosses either way; both are rejected.
The mechanism held at every look: both cells start 159 to 162 units better at
generation 0.

Both cells were worse at every look, by +0.8 to +3.2 on average and on 14 to 17
of the 21 instances. Without a formal control-side crossing it cannot be claimed
that a better start hurts, but the direction held across six independent looks
and two cells. Together with I-001, where a 294-unit head start left 0.9 units,
this is the second measurement saying that the quality of the starting point
does not improve this algorithm's result, and this one suggests it can
subtract: a more uniform and better initial population converges sooner onto
the same place.

On the PI's idea itself: the circle covers the space exactly as random sampling
does, because 247 points represent no areas in a space of 10^501 however they
are placed, and its head start came from uniform sampling rather than from the
circle. It did leave a side finding, the biased random creation, now fixable
with creation.random.draw = uniform; fixing it does not improve the result, so
the frozen configuration keeps the original generator.

No records or matches in the 2250 runs, filter and waves included, checked with
the new scripts/records_any.py, which no longer leaves the filter out.

## I-017 is discarded at the filter: the bug was helping

240 jobs, no infeasible schedule, thirty runs per cell and instance, seeds
1001 to 1030 under the new rule.

The mechanism is exact: in the chosen cell every second tabu call reaches the
other child, none the child already searched, while the control splits them
about 47 to 53; generations fall 4 to 14 percent. Per-instance means, chosen
minus control: ta23 +8.40, ta29 +1.93, ta30 +0.87, ta45 +8.27, mean +4.87
against a rule that discards above +2.0. Discarded, worse on all four instances,
best-of-five moving the same way at +4.62.

The original code, through an index bug, spends a little over half its second
calls re-searching the better child: a second tabu search from its local
optimum with an empty tabu list, which is more depth on the better solution of
the pair. Giving each child exactly one call costs about five units. The bug was
helping: intensifying on the better solution is worth more than spreading the
effort. It is I-013's lesson seen from the other side, where leaving the greedy
rule cost +21.55.

It leaves a hypothesis with a measured direction, I-018. If going from 53% of
second calls on the better child to 0% costs +4.87, going to 100%, always giving
the second call to the better child, should gain if the relation is monotone.
This differs from I-009 and I-010, which gave one deep call per run to the
incumbent and weighed nothing in the total tabu effort; here the extra depth is
given at every pair, tens of thousands of times a run.

abc.ls.pick stays implemented and off by default, and I-018 builds on it. No
records or matches in the 240 runs.

## I-018's filter passes at -1.12, and the curve is asymmetric

240 jobs, no infeasible schedule, seeds 1001 to 1030, independent of the waves.
The mechanism is exact and cheap: every second call lands on the better child
and generations rise on all four instances, on ta45 from 178.9 to 267.2, +49%,
because re-searching a local optimum stops quickly.

Best minus control: ta23 +1.30, ta29 -0.30, ta30 -3.07, ta45 -2.40, mean -1.12
against a rule that discards above +2.0. It passes, best-of-five moving the
same way at -1.25.

The curve is asymmetric: going from 53% of second calls on the better child to
0% cost 4.87 in I-017, going to 100% gains 1.12 at the filter. There is a slope,
but it flattens on the good side; what the bug already does by accident
collects much of the benefit. A -1.12 over four instances is inside the
filter's noise and is not evidence. The waves decide. It is at least the
loop's first favourable filter drawn on seeds of its own.

## I-018 is accepted at look 3: the first mechanism to enter the configuration

Three waves, 630 confirmation runs, fifteen per cell and instance, no
infeasible schedule anywhere. Per-instance mean, best minus control: -1.12 at
the filter on seeds 1001 to 1030, then -1.84 (13 of 21, p = 0.164), -2.60 (16
of 21, p = 0.046) and -3.38 (18 of 21, W = 21.0, p = 0.0010). At look 3 the
p-value is under the symmetric Pocock boundary of 0.0142 with the treatment on
the lower rank, which is the protocol's acceptance criterion: Wilcoxon on the
per-instance mean, paired over the 21, on the full evaluation and never on the
filter. The design stops at a crossing; looks 4 to 6 are not run.

It improves 18 of 21 instances, most on the large ones: ta43 -9.60, ta40 -7.40,
ta23 -7.20, ta42 -7.20, ta44 -6.93, ta45 -6.93. It loses on ta32 +3.33, ta29
+2.60 and ta49 +1.73. Best-of-five moved the same way at every look, -4.19 at
the third.

Why this one. It is the first idea in the loop whose direction was measured
before it was proposed: I-017 fixed an index bug and lost 4.87 because the bug
was giving the better child of each pair a second tabu search half the time.
I-018 gives it always. It is also the first whose filter and looks agreed from
the start, on independent seeds, with the difference growing as runs
accumulated instead of evaporating, the opposite of I-010, I-011 and I-015.

What is not separated, and was said before launch: the best cell deepens the
search on the better child and, since re-searching a local optimum is cheap,
also runs more generations, +5% to +49% at the filter. The acceptance is of the
mechanism as it stands; which of the two carries the gain is open, and is the
natural question for a following iteration run at equal generations rather
than equal time.

No records or matches in I-018's 870 runs. The gain is in the mean; records
live in the tail, which is where it has to be taken next.

abc.ls.pick = best enters the configuration in setup/ref_I-018.txt, which is
the control from I-019 on; prereg2 is left untouched so that every earlier
iteration stays reproducible. This is acceptance 1 of 3 before the 300-second
recomparison against the original reference.

## I-019: no record, and I-018's gain reaches the tail

600 verified runs, no infeasible schedule, fresh seeds 2001 to 2075. No record
and no near miss: nothing came within two units of a best known.

Best of 75, old configuration against the new one: ta22 1614 to 1613, ta23
1566 to 1565, ta29 1631 to 1630, ta30 1598 to 1589. Mean of the three lowest,
better with the new configuration on all four, ta30 1603.7 to 1595.0. On
best-of-five blocks the new configuration wins 40, loses 16 and ties 4, sign
test p = 0.002.

I-018 was accepted on the mean. This attempt, on seeds no batch had used, says
the gain reaches the tail, which is what a record consumes. It is not a second
acceptance, since an attempt has no boundary, but it is an independent
replication in the direction of the accepted mechanism, exactly what I-010 and
I-015 lacked.

ta29 and ta30 sit at +5 after 75 runs of 40 seconds. Own bests are unchanged,
so the new configuration brings the tail closer without crossing it.

## I-020 is discarded at the filter: what I-018 bought is depth

240 jobs, no infeasible schedule, seeds 1001 to 1030, the control being the
current configuration. The none cell makes no second tabu call and runs 56% to
73% more generations, 256.4 to 411.3 on ta45. None minus control: ta23 +5.97,
ta29 +2.77, ta30 +7.33, ta45 +7.77, mean +5.96 against a rule that discards
above +2.0. Discarded, worse on all four, best-of-five worse still at +7.00.

The question I-018 left open has a clean answer. With far more generations and
no second search on the better child, six units are lost: I-018's gain is
depth, not throughput.

Three measurements now trace the curve of the second tabu call. None at all,
against the current configuration: +5.96. Always on the other child, against
the original where 53% landed on the better one: +4.87. Always on the better
child, against the same original: -3.38, accepted. The more of the second call
goes to searching again from the better child's local optimum, the better.

Why it may work, linking to I-010: the second call starts with an empty tabu
list and a reset non-improving counter. I-010 measured that deep calls die in
an all-tabu state after 75 to 92 moves, not from their parameter, and the
second call is in effect a way out of that dead end, emptying the memory and
starting again from the best point. The escape I-012 tested, taking a tabu move
anyway, gave zero; emptying the list and restarting from the optimum does not.

It suggests I-021: more of the same, searching again from the better child's
local optimum for as long as it keeps improving, with no fixed number of
repeats and so no new parameter.

## I-021's filter passes at -2.58 while running a quarter fewer generations

240 jobs, no infeasible schedule, seeds 1001 to 1030, against the current
configuration. The mechanism is exercised hard: 13,733 to 53,132 extra calls
per run on the better child, chains of up to 10 or 11, and 21% to 26% fewer
generations. Repeat minus control: ta23 -2.60, ta29 -0.10, ta30 -2.67, ta45
-4.97, mean -2.58, passing a rule that discards above +2.0. It is better on all
four or tied while running a quarter fewer generations, which is what I-020
predicts if depth is what counts. The waves decide.

## I-021 is accepted at look 3, on top of I-018

Three waves, 630 confirmation runs, no infeasible schedule anywhere, the
control being the configuration that already includes I-018. Repeat minus
control on the per-instance mean: -2.58 at the filter on seeds 1001 to 1030,
then -3.56 (14 of 21, p = 0.022), -2.95 (15 of 21, p = 0.017) and -2.62 (16 of
21, W = 27.0, p = 0.0021), under the symmetric Pocock boundary of 0.0142 with
the treatment on the lower rank. Accepted; the design stops.

Largest gains on ta46 -13.40, ta32 -8.00, ta30 -5.40, ta26 -4.93; losses on
ta33 +2.27, ta44 +1.60, ta48 +1.40, and two near-ties.

The four iterations of this line together: a second tabu call on the better
child wins (I-018, -3.38); dropping it loses even with 56% to 73% more
generations (I-020, +5.96); repeating it while it improves wins again (I-021,
-2.62) even with 21% to 26% fewer. The lever is search depth on the better
solution of each pair, restarting from its local optimum with an empty tabu
list, and its ceiling has not been seen yet: every step towards it has won and
every step away has lost.

No records or matches in the 870 runs. best-repeat enters the configuration as
setup/ref_I-021.txt, the control from I-022 on. Acceptance 2 of 3; the third
triggers the 300-second recomparison against the original reference.

## I-022 matches ta29's best known, and I-021's tail gain does not replicate as I-018's did

600 verified runs, no infeasible schedule, fresh seeds 3001 to 3075. No record.
A match: ta29 = 1625, its best known, from the current configuration at 40
seconds per run, verified by the independent checker against the original
OR-Library data and kept in iter/I-022/evidence/.

The analyzer printed "RECORD" and it is not one: a record is strictly below the
best known. I-006's analyzer, copied for I-019 and I-022, labelled anything at
or below as a record, the slip I-007 had already fixed in its own. All three
now tell RECORD from MATCH; only I-022 ever produced a value at the best known,
so no earlier output changes.

Best of 75, previous against current: ta22 1613 both, ta23 1568 to 1567, ta29
1630 to 1625, ta30 1599 to 1595. Mean of the three lowest better with the
current configuration on all four, narrowly. Best-of-five blocks: 32 better,
25 worse, 3 tied, p = 0.427, with ta30 going the other way.

The match is real and it came from the current configuration, but I-021's
tail replication does not come out as I-018's did, where the block test gave
p = 0.002 with clear margins. I-021's accepted gain lives in the mean over 21
instances; on these four 20x20 instances and in the tail, the step from
ref_I-018 to ref_I-021 cannot be told from noise. The acceptance stands, being
about the mean and bounded, but for the record hunt I-021's benefit is small
at best.

## I-023 is discarded: the kick succeeds, and still loses

240 jobs, no infeasible schedule, seeds 1001 to 1030, against ref_I-021. The
mechanism behaved as measured: 8,843 to 23,193 kicks per run, 29% to 42% of the
kicked copies ending better than the stuck child. Generations fell to under
half. Kick minus control: ta23 +7.17, ta29 +2.57, ta30 +10.30, ta45 +8.20, mean
+7.06, best-of-five +8.17. Discarded, worse on all four.

Six measurements now map the depth lever on the better child: nothing after the
first call, +5.96; the second call spent on the other child, +4.87; one more
call, -3.38 and accepted; calls while each improves, -2.62 and accepted; one
more call after the first failure, 1% success and not launched; a kick and a
new chain after the failure, +7.06. Depth has an optimum, and it is I-021's
chain: search again from the same point while it pays, stop at the first
failure. Going past it costs more than it returns at these budgets, whether by
repeating from the same place or by moving it. The kick's three was fixed
beforehand; trying another value now would be tuning on these data.

Correction to the I-023 entry: there is no record, but there is a match. ta30
reached its best known, 1584, in the control cell of I-023's filter, that is
with the current configuration ref_I-021 at 40 seconds, verified by the
independent checker and kept in iter/I-023/evidence/. It is a different
schedule from the earlier ta30 matches, all 400 start times differing from
I-012's and 146 from I-015's, so there are several distinct solutions at 1584.
The first closing said "no records or matches"; the records script had found
it and the sentence was written before its output was read.

## I-024: the best known is reached with many schedules, and never undercut

800 verified runs, no infeasible schedule, fresh seeds 4001 to 4400, the
current configuration. No record. ta29 reached its best known 1625 once in 400
runs, ta30 reached 1584 twice; all three schedules are new, different from one
another and from every earlier match, verified and kept in
iter/I-024/evidence/. The 1% quantile sits at 1628 and 1589, the median at
1639 and 1616.

With the current configuration the best known is reached in 0.25% of 40-second
runs on ta29 and 0.5% on ta30, and reached through many distinct schedules,
five on ta29 and eight on ta30 across the iterations. There is a wide plateau
at the best known, and 800 more runs found no way below it. That does not show
the best known is optimal, since the lower bounds sit 52 and 65 units away,
but it does show that more sampling with this algorithm will not beat it: the
problem is no longer reaching the plateau but finding the exit beneath it, and
N2 tabu search from the same kind of starting points is not finding it.

This is the case the backlog keeps B-12 for, exact window repair on the
incumbent, which the external review named as the one family with a credible
mechanism for the loose unit N2 cannot see. It is a larger change than the
loop's and is put to the PI before it starts.

## I-025 (B-1): restarts against one long run at equal CPU, on existing data

Nothing was run. Short side: I-001's control cell, 30 independent runs per
instance at the per-class budgets. Long side: prereg2_abc, 10 runs at 300 s.
At equal CPU, one 300 s run against the best of k = 300 / L short runs (7, 3
and 2), reported as distributions. Declared before reading: the two batches ran
on different days and the machine drifts about 9% between batches, so this is
descriptive.

Best-of-k short is better on the median on 17 of 21 instances, worse on 2,
tied on 2; mean difference of medians -3.50, Wilcoxon p = 0.0004, and the worst
short sample is almost always better too. But the outright minimum sometimes
comes from the long run: ta29 1625 at 300 s against 1628, ta27 1698 against
1703, ta45 2019 against 2024, ta50 1973 against 1982, though 10 long samples
against 4 short blocks on 20x20 is not a fair contest for the minimum. With the
configuration of the time, per-class restarts dominate a 300 s run on the
median; for the extreme tail the question stays open, and it has not been
measured with the current configuration.

## I-026: a long run does not give a better extreme tail

106 verified runs of 300 s on fresh seeds 5001 to 5053 with the current
configuration, no infeasible schedule, no record. At the same CPU as I-024's
400 runs of 40 s: ta29 matched its best known 0 times (best 1631) against 1,
ta30 once against 2. The 300 s runs improve ta30's median a little, 1613
against 1616, but not the extreme tail. The ta30 match is a schedule different from all 7 earlier ta30 matches,
verified and kept in iter/I-026/evidence/. Descriptive, two batches at
different hours, but there is no sign that longer runs bring the record closer
with the current configuration; hunting stays at 40 s.

## The code of every discarded idea is rolled back

The protocol says to revert only the idea's code when it is discarded, never
the records, and the PI pointed out that I was not doing it: the cycle is test,
commit what works, roll back what does not. I had been leaving every discarded
idea in the solver behind a switch that was off by default.

Nine files go back to their version at the loop's starting commit, 6544637,
removing the full tails (I-004), frequency tie-break (I-005), deep call
(I-009), dead-end escape (I-010, I-012), first improvement (I-013), kicked
scout (I-003), stall restart (I-014), plateau veto switch (I-015), uniform
draw (I-016), the chosen, none and best-patient modes (I-017, I-020, the probe),
the chain-end kick (I-023) and all their diagnostic counters. What stays is
what was accepted, abc.ls.pick with index as the original default, best
(I-018) and best-repeat (I-021) and their mechanism counters; the genuine bug
fixes, N8's head propagation and the abc_replacements counter that never
counted; and the jsp.seeded port, part of the declared starting point. The
difference from 6544637 falls from 1323 lines in 14 files to 360 in 7.

Verified rather than assumed: with a fixed number of generations and a time
limit that never binds, runs are deterministic, and the binaries before and
after produce identical makespans and identical per-generation traces on all
three configurations in use, prereg2, ref_I-018 and ref_I-021, on ta23 and
ta45, two runs each.

One slip on the way, fixed before any push: git checkout of a commit's files
stages them, so the next commit, I-026's closing, swept in a half-done rollback
with the accepted mechanism missing. It was unpushed, so a soft reset undid it
without losing anything and the work was redone as two commits.

## I-027: B-12 on one machine; the tabu search's optima are already optimal per machine

I-024 left the best known of ta29 and ta30 reached through many schedules and
never undercut, so what is missing is an exit beneath the plateau that N2 tabu
search cannot see. B-12, exact window repair, is the family with a credible
mechanism for that. It starts with one machine, where the subproblem can be
solved exactly and fast, and as a probe on stored schedules without touching
the solver.

The model is exact. With every machine sequence fixed but M's, the disjunctive
graph without M's arcs is acyclic and gives each operation on M a head, a
tail, and for each ordered pair the longest path between their starts through
the rest, a delayed precedence. For a sequence of M the earliest starts follow
from those, and the makespan is the longest path of the full graph. The best
sequence is found by depth-first branch and bound with the Jackson preemptive
bound, keeping only strict improvements, machine by machine to a local optimum.
Every improvement found was checked against the full graph and the model always
agreed.

All 16,820 searches completed exactly. None of the 20 stored schedules at the
best known improves. Of I-024's 800 final schedules, 19 improve (2.4%), by 1
to 4 units, and none reaches the best known. The tabu search's local optima
are almost always optimal against reordering any single machine in any way,
and those at the best known all are. N2 swaps adjacent pairs at the ends of
critical blocks; this allows any order of a whole machine, and there is still
nothing. The exit is not on one machine. B-12 spoke of two machines for exactly
this reason, which is I-028. No solver change, so nothing to roll back.

## I-028: B-12 on two machines; the exit is not there either

The exact extension of I-027: without the arcs of two machines the graph is
acyclic and gives heads, tails and delayed precedences for both; the best pair
of sequences is found by branch and bound over the active schedules of those
two machines (Giffler and Thompson restricted to them), with a Jackson bound on
each, strict improvements only, each checked against the full graph.

On the stored schedules at the best known it was run exhaustively: a pair not
touching the critical path cannot shorten it, so every pair with at least one
critical machine suffices, 2,839 searches over the 20 schedules, all complete
and exact, and none improves. On I-024's 800 final schedules, with the six
pairs of the four most critical machines, 20 improve (2.5%) by 40 units, where
one machine gave 19 and 41; none reaches the best known.

With I-027: the schedules at the best known are optimal against reordering any
machine and any pair of machines that could matter, exactly, and on ordinary
tabu optima two machines find nothing one does not. The exit beneath the
plateau, if there is one, is not in reordering one or two whole machines. What
is left of B-12 is its literal window form, freeing every operation in a slice
of time across all machines at once, which is I-029.

## I-029: B-12 by time window, and the whole family closes

The literal form of B-12: a window is a run of consecutive operations in start
order; on each machine they are consecutive in its sequence, so every machine
splits into a fixed prefix, a free block and a fixed suffix, and the free
operations on all machines are reordered at once, exactly, by branch and bound
over their active schedules with a Jackson bound per machine, each improvement
checked against the full graph. Windows of 30 operations sliding by 15, fixed
before reading any result; then, declared before seeing it, one larger size of
60 sliding by 30, on the schedules at the best known only.

The stored schedules at the best known: 0 of 20 improve with windows of 30
(500 exact searches) and 0 of 20 with windows of 60 (240 exact searches). The
probe does work: on I-024's ordinary finals it improves 10 of 800 (1.2%) by
22 units, the model agreeing with the graph every time.

The B-12 family closes. The schedules at the best known of ta29 and ta30 are
exactly optimal against reordering any whole machine (I-027), any pair of
machines that could matter (I-028), and any slice of 30 or 60 consecutive
operations across all machines at once (I-029), neighbourhoods far larger than
N2, none with an exit. With the best known reached through thirteen distinct
schedules on these two instances, the simplest reading is that those values
are optimal or very nearly so, and that the gap to the lower bounds in
taillard_bounds.csv reflects weak bounds rather than real room. Not a proof,
but the strongest evidence the line has about where a record is not.

## I-030: both acceptances hold at 300 s

210 verified runs of 300 s, no infeasible schedule, the two cells interleaved
in one batch on fresh seeds 6001 to 6005. The current configuration beats the
original on 18 of 21 instances by a mean of -5.82 units, Wilcoxon p = 0.0010.
The two acceptances were measured at the short budgets at -3.38 and -2.62,
about -6 together, and at 300 s the gain stays at that size rather than fading
with more time. It is largest on the big instances, ta41 -20.2, ta43 -14.2,
ta33 -13.6, and also clear on the 20x20 instances closest to their best known,
ta29 -9.2, ta30 -8.0; it loses on ta44 +5.6, ta22 +4.6 and ta32 +2.0, with five
runs per cell making single instances noisy. One more match, ta29 = 1625 at
300 s with the current configuration, a schedule distinct from all 4 earlier ta29 matches, verified and kept. What
was accepted at short budgets is an improvement of the algorithm, not of the
regime.

## I-031 is discarded in both variants, and its code is rolled back

360 jobs, no infeasible schedule, seeds 1001 to 1030, against ref_I-021. The
mechanism was exercised: the mean parent distance went from 0.82-0.90 in the
control to 0.95-0.97 with far and 0.66-0.70 with near. Far minus control:
ta23 +5.53, ta29 +0.63, ta30 -1.03, ta45 +4.20, mean +2.33. Near minus
control: +15.73, +5.13, +7.87, +18.47, mean +11.80. Both exceed the +2.0 rule
and are discarded, far narrowly and near by a wide margin. A random elite
partner beats choosing it by distance either way: the nearest partner starves
recombination of variety, the farthest costs generations without paying them
back.

The ta30 match in the control is not new: it is identical to the one in
I-023's filter control, same seed 1009 and same configuration.

The idea's code is rolled back, the first time under the test, commit, roll
back cycle: ArtificialBeeColonyPSO returns to its version before I-031 with no
trace of the switch, and the rebuilt solver reproduces the reference exactly
at a fixed number of generations on ta23 and ta45. The records stay.

## I-032: no record on ta23, and a new own best, 1561

800 verified runs of 40 s on fresh seeds 8001 to 8800 with the current
configuration, no infeasible schedule, no record and no match. The best is
1561, four above the best known 1557 and three below this line's previous best
of 1564 (I-006), verified by the independent checker and kept in
iter/I-032/evidence/. The 1% quantile is 1567, the median 1583. The current
configuration brings ta23 closer too, but unlike ta29 and ta30 the best known
has not been reached here, so the plateau diagnosis of I-027 to I-029 does not
apply yet: on ta23 the gap is one of arriving, not of leaving.

## I-033: ta18 stays far

400 verified runs of 40 s on fresh seeds, no infeasible schedule, no record and
no match. The best is 1414, 18 above the best known 1396 (1.3%); the 1%
quantile is 1417. The current configuration does not come near ta18's best
known, and the hunt does not justify more runs. It is the line's first data on
a 20x15 instance.

## I-034's filter passes with a zero

240 jobs, no infeasible schedule, seeds 1001 to 1030, against ref_I-021. Between
10 and 231 scouts polished per run, about four tabu calls each. Polish minus
control: ta23 +3.30, ta29 -0.10, ta30 +0.13, ta45 -3.23, mean +0.03, so it passes
a rule that discards above +2.0, which at a filter only means it is not
discarded; best-of-five leans slightly in favour at -1.38, mostly on ta45. The
ta30 match in the control is again seed 1009's schedule, reproduced a third
time. The waves go.

## I-034 is rejected and rolled back: polished scouts do not help

Filter and six waves, 1500 runs, no infeasible schedule, against ref_I-021.
Polish minus control: +0.03 at the filter, then -0.14, -0.11, -0.10, +0.21,
+0.30, +0.31, final p = 0.056, better on only 6 of 21. No crossing, ending on
the control's side, with the mechanism exercised throughout at about 203
polished scouts per run.

Bringing new, independent local optima into the population, a polished restart
for every abandoned food source, does not improve the result. With I-003 and
I-014 (reinjecting around what is held), I-016 (better random scouts) and I-031
(crossing by distance), that is five different ways of steering the search
towards other basins from inside the population, and none helps. What has
worked is added depth on the best the population already has, I-018 and I-021.

No records; the only match is the known ta30 schedule of seed 1009. The code is
rolled back and the rebuilt solver reproduces the reference exactly at a fixed
number of generations.

## I-035's filter passes, but against

240 jobs, no infeasible schedule, seeds 1001 to 1030. About 15,000 second pairs
kept per run, their better child 43 to 46 units better before any search, and
more generations. Pair minus control: ta23 -1.03, ta29 +1.57, ta30 +2.70, ta45
+1.27, mean +1.13, which passes a rule that discards above +2.0 while pointing
the wrong way; best-of-five is worse still at +3.29. A better raw starting child
does not seem to become a better optimum. The waves go, as the rule says.

## I-035 is rejected and rolled back: a better raw start does not help

Filter and six waves, 1500 runs, no infeasible schedule, against ref_I-021.
Pair minus control: +1.13 at the filter, then +1.01, +0.49, +0.75, +1.30,
+0.96, +0.80, final p = 0.099, better on only 7 of 21. No crossing, ending on
the control's side, and close to the harm boundary at looks 4 and 5 (p = 0.033
and 0.039). The mechanism was exercised throughout, about 18,400 second pairs
kept per run.

Starting the deep chain from a child some 45 units better before any search
does not lead to a better optimum; the second crossing and its evaluation cost
time and the starting points lose variety. Raw quality of the starting point
does not predict the quality of the optimum the chain reaches.

No records and no matches. The code is rolled back and the rebuilt solver
reproduces the reference exactly at a fixed number of generations.

## I-036 is discarded by its filter and rolled back: path relinking hurts

240 jobs, no infeasible schedule, seeds 1001 to 1030. About 36,800 PR
crossings per run, the first child some 208 of 245 differing positions away
from its parent, and more generations (81 to 110 on ta23, 187 to 280 on ta45).
PR minus control: ta23 +32.10, ta29 +11.50, ta30 +21.60, ta45 +35.47, mean
+25.17, far above the +2.0 discard line, worse on all four; best-of-five
+26.88.

The interpolated point of the path between the parents is much worse than JOX.
One reading, not shown: the interpolation is deterministic given the parents
and keeps everything they share, so children land between them, chains end
sooner and the search loses the variety JOX's random job mask gives it. With
H-2, which returned the best point of the path, that is two forms of path
relinking that do not help here.

No records; the only match is the known ta30 1584, in the control cell. The
operator is removed and the relinked solver reproduces the reference exactly.

## I-037 is discarded by its filter and rolled back: deeper calls do not pay

240 jobs, no infeasible schedule, seeds 1001 to 1030. About 18,700 doubled
calls per run at the end of chains, 8 to 16 per cent of them improving, at the
cost of a third of the generations. Esc minus control: ta23 +1.10, ta29 -0.83,
ta30 +7.93, ta45 +1.23, mean +2.36, just above the +2.0 discard line;
best-of-five +5.25.

The doubled call does find improvements the 15-iteration call gave up on, but
not enough to pay for the generations it costs. With I-020 and I-021 the map of
depth on the better child reads: more normal calls, yes; deeper calls, no.

No records and no matches. The code is rolled back and the rebuilt solver
reproduces the reference exactly.

## I-038 is withdrawn: the fifth of the time exists only on an empty machine

The filter passes (nostat minus control -0.15), but the mechanism read first is
absent: generations per run 79 to 78, 95 to 95, 80 to 78 and 177 to 190. The
pre-launch timing had one process on the machine; production runs 14 at once,
and there generations for the same configuration and seed depend mostly on
placement (55 to 97 within one cell on ta23). Measured directly with 14
concurrent processes, 7 per cell on paired seeds: launching the control first,
nostat did fewer generations in all 7 pairs; launching nostat first, 608
against 605. Traces are identical generation by generation, so the difference
is speed only, and under that load the statistic costs nothing measurable.

Withdrawn, as I-009 was, because the mechanism the idea needs does not exist in
the regime where it is measured. Nothing to roll back: it was one setup line.
Lesson for any speed idea: measure with 14 concurrent processes, paired seeds
and alternated launch order, never with one process alone.

## I-039 is discarded by its filter and rolled back: JOX's half mask is right

240 jobs, no infeasible schedule. About 37,400 crossings per run with the keep
probability drawn from U(0,1), mean 0.50, mean deviation 0.25; generations rise
sharply (72 to 111 on ta23, 191 to 292 on ta45). Umask minus control: ta23
+13.20, ta29 +6.27, ta30 +8.70, ta45 +18.63, mean +11.70, far above +2.0.

With I-036, the two crossovers that raise the generations are the two that sink
the result. A child near one of its parents sits almost in that parent's local
optimum: the chain ends at once and returns where the search already was.
JOX's half mask puts the child as far as possible from both parents at once,
and that is what feeds the deep chain new places. Here high generation counts
are not speed but a symptom of short chains.

No records and no matches. The code is rolled back and the rebuilt solver
reproduces the reference exactly.

## I-040's filter passes: the time goes where it should

240 jobs, no infeasible schedule, seeds 1001 to 1030. About 12,500 chains cut
per run, 27 to 44 per cent of those checked; tabu calls stay the same and
generations rise (74 to 114, 90 to 107, 74 to 101, 188 to 266), so the time
saved on lost chains is spent on new ones, as intended. Cut minus control: ta23
-2.03, ta29 -1.97, ta30 +3.20, ta45 +1.00, mean +0.05, passing the +2.0 rule.
Split: better on ta23 and ta29, worse on ta30. The waves go.

## I-040 is rejected and rolled back: cutting lost chains is neutral

Filter and six waves, 1500 runs, no infeasible schedule. Cut minus control:
+0.05 at the filter, then -0.18, -1.78 (p = 0.037), -1.28 (p = 0.042), -0.65,
-0.37, -0.20, final p = 0.651, better on 12 of 21. No crossing; the signal of
looks 2 and 3 was noise returning to the mean, which is what the Pocock
boundary is for.

The mechanism worked as designed (about a third of chains cut, tabu calls
unchanged, generations up 16 to 55 per cent) and the result does not move:
reassigning the time of lost chains to new ones neither wins nor loses. With
I-020 and I-037, the employed-phase tabu budget is at its point: not deeper,
not shorter, not reassigned.

No records; the only match is the known ta30 1584, in the control cell. The
code is rolled back and the rebuilt solver reproduces the reference exactly.

## I-041's filter passes by a hair, pointing against

240 jobs, no infeasible schedule. About 4,000 sideways moves per run, scouts
and generations close to the control's. Side minus control: ta23 +3.33, ta29
-0.50, ta30 +3.97, ta45 +0.40, mean +1.80, two tenths inside the +2.0 line;
best-of-five +3.25. The waves go, as the rule says.

## I-042: CP-SAT does not move our best schedules either

With OR-Tools installed (PI's authorisation), CP-SAT solved the full model of
ta29, ta30 and ta23 with our best stored schedule as a complete hint: 14
workers, seed 1, 600 s each, on an empty machine. No improvement: 1625, 1584
and 1561 stay exactly as hinted, and the proven bound stays at the published
lower bound on all three. The telling case is ta23, four units above the best
known, where ten minutes of large-neighbourhood search around our schedule
find nothing better. Our best schedules are deep local optima for CP-SAT's
large relaxations too, beyond what B-12 showed for one or two machines and
windows of 60 operations.

## I-041 is rejected and rolled back: moving across the plateau is neutral

Filter and six waves, 1500 runs, no infeasible schedule (wave 5 ran after the
I-042 probe, never at the same time). Side minus control: +1.80 at the filter,
then +0.27, -0.05, +0.04, +0.27, +0.38, +0.22, final p = 0.768. The collapse
of the population onto one makespan is real, but letting sources drift across
it neither helps nor hurts; with I-015, two ways of acting on the plateau and
both neutral. The plateau is a symptom of stagnation, not its cause.

No records and no matches. The code is rolled back and the rebuilt solver
reproduces the reference exactly.

## I-043: the ABC -> CP-SAT hybrid matches ta23 at 1557

The 20 lowest distinct ta23 schedules of the I-032 hunt (1561 to 1568), each
given 60 s of CP-SAT with the I-042 model (14 workers, seed 1). Nine of the 20
improve. Hint 6, a 1566 schedule, goes to 1564, 1563, 1562 in its first second,
then 1559 at 30 s, 1558 at 37.5 s and 1557 at 39.6 s: the best known value,
verified by verify_certificate.py against the original OR-Library data. Our
best on ta23 had been 1561 after 800 hunting runs.

The best ABC schedule is not the best starting point: the four lowest hints
(1561 to 1565) do not move, as in I-042, while a 1566 one drops nine units.
Spreading CP-SAT's time over distinct regions pays where ten minutes on the
single best did nothing; the ABC supplies exactly what CP-SAT needs, many
distinct near-optimal starting points. Next: the same hybrid on ta29 and ta30,
where a record is one unit away, and a continuation of hint 6.

## I-044: 1584 and 1625 behave as floors

ta23 from its 1557 schedule does not improve in 600 s. On ta29, none of the ten
lowest distinct schedules (five at 1625, five at 1627) moves in 60 s. On ta30,
all four 1587 schedules drop to 1584, the best known (four certificates
verified), and none goes below. CP-SAT brings any nearby ta30 schedule of ours
to 1584 within a minute and never past it. With B-12, 1584 and 1625 behave as
floors: whatever lies below is not near any region the ABC visits. Not a proof
of optimality (CP-SAT's bound stays at the published lower bound), but reason
to move the hybrid to the open instances where it still has room.

An incident: parts b and c did not run the first time because some old
certificates have truncated rows; the hint selector now skips such runs and
the two parts were rerun exactly as fixed.

## I-045: three new own bests, and the improvements come early

The hybrid on the other open 20x20 instances, 6 hints each, 75 s: ta25 1616 to
1603 (best known 1595), ta26 1660 to 1653 (1643), ta27 1694 to 1689 (1680), all
verified; ta22 stays at 1613 (1600), its six hints all 1613. Again the best
hint is not the one that drops most (on ta25 a 1617 hint reaches 1603 while the
1616 one stops at 1613), and almost every improvement comes in the first 10 to
40 s. More hints, less time each.

## I-046: with many hints nearly all improve, and ta27 drops to 1685

40 hints each at 20 s. ta25: 34 of 40 improve, but the best is again 1603.
ta27: 31 of 40 improve, best 1685, a new own best (was 1689; best known 1680).
Short time suffices, but each instance's minimum stalls at a value more hints
and more time do not move: the hybrid takes every hint to its CP-SAT floor,
and those floors have a minimum of their own above the best known. Going lower
needs hints from other regions.

## I-047: closing the loop finds nothing new around the floors

56 seeded ABC runs of 40 s, no infeasible schedule. The seeded ABC never goes
below its best seed: its best runs return exactly 1603 (ta25) and 1685 (ta27).
CP-SAT on the best distinct schedules of those runs does not pass 1603 or 1685
either. They are floors shared by both methods; the ABC <-> CP-SAT loop, as it
stands, converges to them. Own verified bests stand at ta23 1557 (= best
known), ta25 1603, ta26 1653, ta27 1685.

## I-048: four new own bests on the open 30x15 instances

6 hints each at 60 s, all verified: ta32 1815 to 1811 (best known 1784), ta33
1820 to 1802 (1791), ta34 1843 to 1834 (1829), ta40 1695 to 1688 (1669). On
ta34 five of six hints end exactly at 1834, another shared floor. Next: many
hints on ta33 and ta34.

## I-049: on 30x15, 20 s is too short

40 hints each at 20 s: ta33 best 1802 (the same as I-048), ta34 best 1835 (worse
than the 1834 of I-048's 60 s). Unlike 20x20, CP-SAT keeps improving past 20 s
on 30x15. Next: long continuations from the best schedules.

## I-050: long continuations pay little

From I-048's best schedules: ta33 (1802) and ta34 (1834) do not move in 600 s;
ta32 goes from 1811 to 1809 and ta40 from 1688 to 1686, all verified. As on
20x20, insisting on one schedule pays little; the hybrid's reach is in spreading
CP-SAT over distinct hints with just enough time each. Own verified bests now:
ta23 1557 (= best known), ta25 1603, ta26 1653, ta27 1685, ta32 1809, ta33
1802, ta34 1834, ta40 1686, and the matched ta29 1625 and ta30 1584.

## An apparent ta45 record that was not one

Listing the 30x20 bests showed a 1998 on ta45, below the best known 2000. Run
through verify_certificate.py before anything was said: it comes from
I-002_voidfilter_n8, the cell of the old N8 bug, whose schedules overlap on a
machine; the best verified schedule in that certificate is 2027. No record.
The hint selector did not check feasibility; it now skips any run with
unreadable rows, job-order violations or machine overlaps. The hints of I-043
to I-050 did not come from that cell, and every schedule CP-SAT writes is
verified against OR-Library, so the own bests reported stand.

## I-051: four small new own bests on 30x20

6 feasible hints each at 60 s, 52 new schedules verified: ta45 2015 to 2011
(best known 2000), ta49 1987 to 1980 (1961), ta44 2011 to 2009 (1979), ta42
1974 to 1971 (1937). Gains of 2 to 7 units against gaps of 11 to 34.

## I-052: floors belong to the hint, not to the seed

With CP-SAT seeds 2 to 13 at 45 s, each hint always falls into the same two or
three floors: ta25 1603 ten times of twelve (and 1615, 1616), ta26 1653 three
times and 1660 nine, ta27 1685 eight times and 1689 four. The floor is almost a
property of the hint; the solver's randomness only picks among a few. The one
lever left in the hybrid is more distinct hints.

## I-053: 1603 is an attractor on ta25

Hints ranked 41 to 120 (1623 to 1631), 20 s each: 70 of 80 improve, and the best
again ends exactly at 1603, from a different schedule. With 160 distinct hints,
twelve seeds and continuations of up to 600 s, ta25 does not pass 1603, ta27
1685, ta26 1653, and ta29/ta30 their best known values. Distinct starting
points end at the same value: attractors of the ABC + CP-SAT pair, which the
hybrid has already found.

## I-054: no mix of two floors beats the attractor, and it is proven

30 exact recombinations (15 on ta25, 15 on ta27), agreeing machine pairs fixed:
all 30 end OPTIMAL in 0 to 2 s at the better parent, 1603 and 1685. The
parents, floors of distinct hints, agree on 86.6 to 96.8 per cent of the 3800
machine pairs on ta25 and 93.3 to 95.8 on ta27. Next: fix only what every
parent of a group shares, with ever larger groups.

## I-055: there is a common core, and the attractor is optimal inside it

Consensus recombination over nested groups of CP-SAT floors, only pairs every
member orders alike fixed, 120 s each. On ta25, 1603 is proven optimal in the
consensus of 4, 8, 16 and 32 floors (1051 free machine pairs); with 64 and 107
floors CP-SAT does not close in 120 s (bounds 1598, 1591) and finds nothing
better. On ta27, 1685 is proven optimal in the consensus of all 37 floors (652
free). Unlike the IJSP diagnosis of June, the classic JSP floors share a core,
two thirds of the machine decisions across 107 distinct hints, and every
schedule better than the attractor must break a decision the floors share.

## I-056: 1603 is optimal in the consensus of all 107 ta25 floors

With 600 s the two large consensus subspaces close: K = 64 OPTIMAL 1603 in 163
s, K = 107 (1263 free machine pairs, a third of 3800) OPTIMAL 1603 in 565 s.
Every ta25 schedule better than 1603 breaks at least one of the 2537 machine
decisions all 107 floors share. I-057 frees them one machine at a time.

## I-057: breaking the core one machine at a time is inconclusive

Freeing one machine's consensus pairs on top of the 107-floor consensus (1361 to
1406 free pairs), CP-SAT finds nothing better than 1603 on any of the 20
machines, and closes none in 60 s (bounds 1584 to 1588). No improvement and no
proof. Closing each would take on the order of ten minutes per machine.

## I-058: 1613 on ta22 is the strongest attractor

40 hints each at 20 s: on ta22, 38 of 40 end exactly at 1613 and two at 1614;
on ta26 the best is 1656, the own best stays at 1653. No new bests.

## I-059: the core is not a bias of the ABC

Cold CP-SAT on ta22, no hint, seeds 1 to 10 at 120 s: makespans 1629 to 1648
(worse than the ABC's 1613), and 90.0 to 93.5 per cent agreement with the ABC
attractor on machine pairs, the same order as the ABC's own floors. Two
generators with nothing in common land in the same core: a structure of the
instance, not of our algorithm. The best known 1600 must break decisions both
make. I-060 tests their joint consensus.

## I-060: inconclusive, with the bound below the best known

The joint consensus of 20 ABC-side 1613 schedules and 10 cold CP-SAT schedules
fixes 77.7 per cent of machine pairs, leaving 847 free. In 600 s CP-SAT finds
nothing better than 1613 and does not close: the bound stays at 1598, below
the best known 1600, so the subspace could hold schedules of 1600 to 1612.
I-061 gives it 1500 s.

## I-061: ta22's best known schedule lies outside the core, proven

With 1500 s the joint-consensus subspace of I-060 still does not close (best
1613), but the bound rises to 1607: no ta22 schedule with makespan at most 1606
respects the 2953 machine decisions shared by the 30 schedules of both
generators. The best known 1600 lies outside that core. With I-055 and I-056
(ta25: 1603 optimal in the 107-floor core, best known 1595; ta27: 1685 optimal
in the 37-floor core, best known 1680), on three instances the best known
schedule breaks decisions all our good schedules share. Everything we do works
inside the core; the next step searches against it.

## I-062: searching against the core is inconclusive

Asking CP-SAT for a ta22 schedule of makespan at most 1612 that flips at least
10, 50 or 150 core decisions: nothing found and nothing proven in 480 s each.
Without a hint that meets the target, this is as hard as solving the instance
near-optimally from scratch. Searching against the core needs a starting point
outside it, and none of our generators provides one. The most informative
would be the published best-known schedules themselves, which would show which
core decisions they break; downloading them needs the PI's permission.

## I-063: five new own bests on 30x20

6 feasible hints each at 60 s, all verified: ta41 2059 to 2031 (best known
2005), ta43 1890 to 1883 (1846), ta46 2043 to 2032 (2004), ta47 1926 to 1922
(1889), ta48 1996 to 1990 (1937).

## I-064: ta18 to 1405, ta50 to 1950

ta18, never polished before: 10 of 20 hints improve, 1414 to 1405 (best known
1396), reached from two distinct hints. ta50: 5 of 6, 1966 to 1950 (1923). All
verified. I-065 takes ta18's hints 21 to 80 at 20 s.

## I-065: no new best on ta18; I-066 extends the core test

ta18 hints 21 to 80 (all 1420): 16 of 60 improve, best 1409; the own best stays
at 1405. I-066 takes the consensus core of ta18 and ta26 with 600 s each.

## I-066: five instances with the best known schedule outside the core

ta18: the consensus of 27 schedules is solved to optimality at 1405 in 16 s
(best known 1396); ta26: 26 schedules, optimal 1653 in 332 s (best known 1643).
With ta22 (bound 1607 against 1600), ta25 (1603 against 1595) and ta27 (1685
against 1680), on five open instances every schedule that matches or beats
the best known breaks a machine decision all our good schedules share.

## I-067: going below 1405 on ta18 needs at least two machines of the core broken

Freeing one machine's consensus pairs at a time, all 15 machines close OPTIMAL
at 1405 in 13 to 31 s. Every ta18 schedule below 1405, the best known 1396
included, breaks core decisions on two machines or more. I-068 to I-070 free
pairs of machines, 105 pairs in three batches.

## I-068 to I-070: 93 of 105 machine pairs of ta18 proven at 1405

Freeing the core pairs of two machines at a time, 45 s each: 93 of the 105
pairs close OPTIMAL at 1405, 12 do not close (bounds 1400 to 1403), and none
gives anything below 1405. The open ones revolve around machines 3, 4, 5, 7,
8, 10 and 13, machine 13 in eight of them. I-071 reruns them at 120 s.

## I-071: the first escape from the core, ta18 to 1404

With 120 s the 12 open machine pairs of ta18 all close OPTIMAL: eleven at 1405
and the pair (5, 8) at 1404, verified against OR-Library, a new own best (was
1405) and the first schedule of the whole line outside the core. Of the 105
pairs, 104 have optimum 1405 and only (5, 8) allows going lower. This opens a
descent: break the core, find a better schedule outside it, recompute the core
with that schedule, repeat. I-072 is the first round.

## I-072: round 1 of the descent, 1404 holds on 14 machines

With the 1404 schedule in the group, 14 of the 15 single-machine relaxations
close OPTIMAL at 1404; machine 13 does not close in 60 s (bound 1400). I-073
runs machine 13 at 180 s and the 105 pairs in three batches.

## I-073: the descent reaches 1401 on ta18

Round 2 of the core-breaking descent: machine 13 closes at 1404; of the machine
pairs 1 to 70, 43 close at 1404, 26 stay open, and the pair (4, 14) closes
OPTIMAL at 1401, verified against OR-Library, a new own best five units from
the best known 1396. The descent so far: 1405, 1404, 1401. Round 3 starts from
1401.

## I-074 to I-076: round 3 from 1401, 98 of 105 pairs proven

With 1401 in the group, all 15 single machines and 98 of the 105 machine pairs
close OPTIMAL at 1401; seven stay open (bounds 1396 to 1399), machine 13 in five
of them. No improvement yet. I-077 reruns the seven at 180 s.

## I-077: 1401 holds against every single machine and every pair

The seven open pairs close OPTIMAL at 1401 with 180 s. With the 29-schedule
core, 1401 is optimal when any one machine or any of the 105 machine pairs is
freed; going lower needs three machines or more. I-078 samples 30 random
triples of the 455 at 50 s.

## I-078: machine triples give no signal; the method moves to ta27

Of 30 random machine triples of ta18, 25 close OPTIMAL at 1401 and 5 do not
close in 50 s, none improves. The ta18 descent rests at 1405, 1404, 1401. The
method moves to ta27, the instance with the shortest gap (1685 against 1680),
starting with its 20 single machines.

## I-079 and I-080: ta27 holds at 1685 on every machine; its pairs are hard

All 20 single-machine relaxations of ta27 close OPTIMAL at 1685 (machines 0 and
15 needed 180 s). The 37 pairs containing machine 0 or 15 do not close in 40 s
(bounds 1673 to 1682) and none improves; ta27 is much harder than ta18. The
lowest bounds, (15,17) 1673 and (0,15), (0,17) 1676, sit below the best known
1680. I-081 gives those three pairs 400 s each.

## I-082: ta27 drops to 1684

The ta27 machine-pair campaign, 187 pairs at 90 s: nearly all close OPTIMAL at
1685, four stay open without improvement, and the pair (17, 18) closes OPTIMAL
at 1684, verified against OR-Library, a new own best four units from the best
known 1680. After ta18, the second instance where breaking the core by machine
pairs moves an attractor nothing else moved. I-083 starts round 2 from 1684.

## I-084: ta27 matches the best known value, 1680

Round 2 of the ta27 core-breaking descent: freeing machines 15 and 18 of the
core, CP-SAT closes the subspace OPTIMAL at 1680 in 37 s. verify_certificate.py
against the original OR-Library data: "best verified makespan 1680 -- matches
the best known solution 1680". The second match of the line after ta23, and
the first of the descent: 1685, 1684, 1680. I-085 starts round 3 from the best
known value; anything below 1680 would be a record.

## I-086: 1680 holds against all 190 machine pairs of ta27

From the 1680 group, all 190 machine pairs close OPTIMAL at 1680. With the 20
single machines of I-085, every ta27 schedule below 1680, that is every
record, breaks the core of those 39 schedules on three machines or more.
I-087 samples 30 random machine triples at 60 s.

## I-087: machine triples give no signal on ta27 either; the descent moves to ta25

29 of 30 random ta27 triples close OPTIMAL at 1680, one stays open, none
improves. I-088 runs the core-breaking descent on ta25 (1603 against 1595) with
the consensus of its 8 lowest floors, which closes in 5 s.

## I-088: ta25 does not drop with the 8-floor core; I-089 sweeps ta18 triples

With the consensus of ta25's 8 lowest floors, all 20 machines and 190 pairs
close OPTIMAL at 1603 in at most 4 s each; that core fixes more pairs than the
107-floor one, so the proof is weaker than on ta18 or ta27. I-089 sweeps the
425 remaining machine triples of ta18 from 1401 at 50 s, in batches of 35.

## I-089: the ta18 machine-triple sweep

With I-078, 380 of the 455 machine triples of ta18 close OPTIMAL at 1401 in 50
s; 75 stay open, nearly all with machine 13, bounds 1392 to 1400; none goes
below 1401. I-090 reruns the open ones at 180 s, lowest bound first.
