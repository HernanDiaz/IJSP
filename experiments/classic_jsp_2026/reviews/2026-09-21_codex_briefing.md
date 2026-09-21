You are being consulted as an expert in metaheuristics for scheduling. Do not
write code and do not inspect this machine: answer from the briefing and from
the literature. Be concrete and falsifiable, and say plainly when an idea is a
known negative result.

# The problem

Classic (deterministic) Job Shop Scheduling, Taillard benchmark, sizes 20x15,
20x20, 30x15, 30x20. The objective of the research line is to beat a best
known solution (BKS) on an open instance, not to publish another comparison.

The solver is a C++ hybrid: an Artificial Bee Colony / PSO population with a
tabu search as its local search, on the N2 neighbourhood (swaps restricted to
the ends of critical blocks, Nowicki-Smutnicki). Every makespan reported below
is recomputed from the emitted schedule against OR-Library data, never taken
from the solver's own report.

# The configuration, frozen (tuned by irace on 300 s runs)

- population 247, elite 86, elite selection 1, abandonment after 35 trials
- crossover JOX p = 0.982, mutation swap p = 0.1801
- local search = tabu, applied every generation to a **random 46 %** of the
  population (`target = 0.4645`, `period = 1`)
- each tabu call: stops after **15 consecutive non-improving iterations**,
  capped at **2 s** of CPU; tabu list minimum length 3; aspiration on the
  global best; the immediate reversal of the last move is forbidden
- neighbourhood N2, estimator "heads & tails" (a lower bound on the makespan
  after the swap), with exact pruning: the neighbourhood is sorted by the
  bound and the sweep stops once the bound cannot beat the best real value
  found, so the move taken is the exact best in the neighbourhood while only a
  prefix is evaluated
- Lamarckian (the improved solution is written back into the genotype)

Parameters are frozen by decision of the principal investigator: retuning per
iteration makes each research loop too long. A new idea may introduce a
parameter, but it must be testable at one value declared in advance.

# What we have measured (our own data, all verified)

1. **Distance of our best verified makespan to the BKS**, per open instance:
   ta29 0 (BKS matched), ta30 4, ta22 13, ta23 14, ta27 16, ta26 17, ta18 18,
   ta45 19. The remaining 30x20 instances are 47 to 65 away.
2. **Time of the last improvement**, 440 traces: median at 0.32 of the
   budget. 61.8 % of every 300 s run is spent after the last improvement. In
   20x20 the median is 36 s of 300; in 30x20 it is 0.48 of the budget.
3. **Restart beats one long run.** Simulating restarts from truncated traces
   (valid because the solver does not know the horizon), k x L beats 1 x 300 s
   on 21 of 22 instances. Best fixed L per class: 20x15 30 s, 20x20 40 s,
   30x15 100 s, 30x20 150 s.
4. **Throughput is not the binding constraint.** A memetic algorithm with its
   own irace-tuned configuration performs 6.5x more generations than the ABC
   in the same wall clock and loses: ABC better on 18 of 22, Wilcoxon
   p = 0.002, mean 5.4 makespan units (0.29 %).
5. **Run-to-run spread** of the final makespan at the short budget is about 8
   makespan units (sd), on makespans of 1580 to 2050. The effects we are
   chasing are around 3 units, i.e. 0.2 %.
6. **Seeding the initial population** from a bank of precomputed job
   permutations (generators: a deep RL policy, a GP dispatching rule,
   Giffler-Thompson with epsilon, GRASP with Most Operations Remaining, and a
   mixed pool), k = 25 of 247: on four filter instances, seeded minus control
   is between -2.4 and -3.6 units. A separate 58,560-run study of ours on the
   fuzzy/interval version of the same problem found seeding worth 0.1-1.0 % at
   full budget and 1.3-5.2 % at a tenth of the budget, and that what matters
   is the **composition** of the pool rather than the quality of its seeds: a
   mixed pool starts worse and ends better. A full 21-instance confirmation of
   this in the crisp short-budget regime is running now.

# What has failed, with numbers

- **Tabu back-jump** (returning to the best solution after stagnation,
  Nowicki-Smutnicki style): 22 instances x 5 runs x 300 s, better on 10 of 22,
  p = 1.000; on the 12 instances held out of development, plain tabu was
  better, p = 0.022. Discarded.
- **Elite pool + path relinking** (IPRTS-style): about 500 path-relinking
  calls produced 0 improvements; a quality-neutral plateau. Discarded.
- **Memetic algorithm** instead of the ABC: item 4 above. Discarded.

# How anything new gets evaluated

21 open instances with a seed bank, 30 runs per cell, budgets per size class
(40 / 100 / 150 s), a control cell run in the same batch, paired Wilcoxon over
the 21 instances on per-instance mean makespan. Run as 6 accumulating waves of
5 runs with Pocock's constant boundary (nominal 0.0142 per look). The machine
is 14 threads; one research step is about 7 CPU-hours.

# The current backlog

- **B-1** Confirm the restart finding with real runs rather than truncated
  traces. Costs no compute, only analysis of data we already have.
- **B-2** Optimise the *recipe* of the seed pool (quality quantiles, distance
  threshold, fraction, mixture of generators) against the final makespan at a
  short budget, on generated instances, then evaluate on Taillard. Depends on
  the seeding confirmation showing signal.
- **B-3** Train the RL generator with reward equal to the ABC's final result
  rather than the seed's own makespan. Only if B-2 shows a large lever.
- **B-4** First-improvement with random order in the tabu instead of
  best-of-all. Now low priority: the sweep already takes the exact best move
  via lower-bound pruning, so ordering cannot change the trajectory.
- **B-5** A learned move-selection policy in the tabu (RL or hyperheuristic).
  Only if B-4 shows that the selection rule matters.
- **B-6** Record attempts: many short runs of the current configuration on the
  shortlist above. This is the objective rather than an idea.
- **B-7** Adaptive run length: end a run after S seconds without improving the
  global best instead of exhausting a fixed budget, and let the harness start
  the next one. The class budgets are the best *fixed* L; an adaptive one
  should beat them.
- **B-8** Where to spend the tabu, at equal total cost. Today it falls on a
  random 46 % of the population every generation, 15 non-improving iterations
  and 2 s per call, and the incumbent gets nothing special except by chance.
  Cells: all the effort on the best; or the best always plus a random
  fraction.

# What I am asking you

1. Rank the backlog. Which would you run first, and which would you drop?
2. The failures are all intensification-and-recombination devices (back-jump,
   path relinking, a memetic); what has worked is restart and, slightly,
   seeding. Does that pattern tell you something about where this solver's
   deficiency actually is?
3. Propose new ideas. Constraints: a small diff to an existing C++ codebase, a
   hypothesis measurable as a makespan difference of the order of units, and
   at most one new parameter fixed in advance. I am particularly interested in
   anything that could close a gap of 4 to 19 units rather than the 3 units
   that seeding buys.
4. Given N2 plus tabu plus a population, what in the literature since
   Nowicki-Smutnicki (1996) are we plausibly missing? I am thinking of
   things like the i-TSAB / TSAB family, Balas-Vazacopoulos guided local
   search with insertion neighbourhoods, Zhang's TS/SA hybrids, Goncalves'
   biased random-key genetic algorithms, Peng-Lu-Cheng's path-relinking tabu,
   or constraint-programming hybrids, but tell me what you actually think
   matters for closing single-digit gaps on 20x20 and 30x20 Taillard
   instances, and which of those are known not to help.
5. Be explicit about anything in my measurements that you think is
   misinterpreted, especially the restart result and the claim that
   throughput is not the constraint.
