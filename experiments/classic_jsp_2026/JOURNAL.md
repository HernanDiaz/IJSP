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
