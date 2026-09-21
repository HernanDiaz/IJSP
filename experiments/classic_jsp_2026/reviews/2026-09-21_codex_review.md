Your solver’s main deficiency is probably not population throughput or initial-solution quality. It is that nearly all intensification uses the same very shallow N2 trajectory. Restarts help because they repeatedly sample new basins; seeding helps modestly because it changes which basins are sampled. Back-jumping and path relinking fail because they mostly recycle basins already represented.

The most promising route to gains of 4–19 units is therefore to change either:

1. the depth of tabu trajectories, or  
2. the connectivity of the local-search neighbourhood.

More population recombination is unlikely to be the answer.

## 1. Backlog ranking

My order would be:

| Rank | Item | Decision |
|---:|---|---|
| 1 | **B-1 real restart confirmation** | Do immediately |
| 2 | **B-6 record attempts** | Start immediately after B-1; this directly serves the stated objective |
| 3 | **B-7 adaptive run length** | Test, but use a carefully defined stopping statistic |
| 4 | **B-8 tabu allocation** | Test “best always + random remainder,” not “best only” |
| 5 | **New: occasional deep tabu call** | Higher priority than B-2 if 15 iterations really is the normal trajectory depth |
| 6 | **New: expanded/insertion neighbourhood** | The strongest structural experiment, though a somewhat larger diff |
| 7 | **B-2 seed-recipe optimisation** | Continue only if the current confirmation is convincingly positive |
| 8 | **B-4 first improvement** | Diagnostic experiment at most |
| 9 | **B-3 end-to-end RL seeding** | Defer indefinitely unless B-2 produces a much larger effect |
| 10 | **B-5 learned tabu policy** | Drop for now |

### B-1

This is essentially free and tests the premise underlying the whole record-attempt strategy. Do it before spending more compute.

But “real-run confirmation” must mean independent runs actually stopped and restarted at the proposed horizons—not merely another aggregation of prefixes. Report the distribution of the best-of-\(k\), not only its mean.

### B-6

This is not scientifically glamorous, but it is correctly aligned with the objective. On ta30, where the gap is four and the short-run standard deviation is about eight, independent attempts may be a better use of the machine than developing a mechanism whose expected effect is three units.

Prioritise the shortlist approximately by both gap and empirical hit probability, not gap alone. In particular: ta30 first, then ta22/23/27/26/18/45 after estimating each instance’s lower-tail distribution. A 19-unit gap with a heavy improving tail can be a better target than a 13-unit gap whose runs have already collapsed to the same value.

### B-7

Promising, but “seconds since global-best improvement” has a trap: early runs that happen to find a strong incumbent quickly will then receive more fruitless time than weak runs that stop early. The rule can therefore allocate effort in the wrong direction.

Predeclare one parameter:

> Stop after \(S=0.25L\) seconds without a global-best improvement, subject to a minimum runtime of \(0.25L\).

Use \(L\) equal to the class-specific fixed horizon. The minimum protects runs whose useful improvements arrive after a slow start. Compare it against fixed-\(L\) restarts at equal total wall time, including restart overhead.

I would also log the conditional probability of another improvement as a function of stagnation age. If that hazard does not decline materially, a stagnation cutoff has no basis.

### B-8

Test:

> Always improve the current population best; allocate all remaining calls randomly, keeping exactly the same total number of tabu calls.

Do not begin with “all effort on the best.” With Lamarckian replacement and elite selection, that risks repeatedly polishing one lineage while destroying the diversity that your restart and mixed-seeding results say matters.

This experiment measures whether local-search allocation is currently wasting calls on poor individuals. It does not test whether the tabu search itself is deep or expressive enough.

### B-2

The observed effect is credible enough to finish the confirmation, but I would not launch a high-dimensional recipe optimiser merely because four development instances show \(-2.4\) to \(-3.6\). That is almost exactly the scale at which adaptive selection and winner’s curse become dangerous.

If confirmed, reduce B-2 to a small, prespecified factorial question:

- mixed versus single generator;
- diversity-filtered versus unfiltered;
- perhaps 10% versus the present 25/247 fraction.

Optimising quality quantile, distance threshold, fraction, and mixture jointly on generated instances is very likely to learn a generator/decoder-specific recipe whose Taillard transfer is weak.

### Drop or freeze

- **B-4:** exact best move and first acceptable move produce different trajectories even if the sweep ordering previously did not matter. Therefore it is not logically true that “ordering cannot change the trajectory.” But first improvement mostly buys evaluations, and you already know raw evolutionary throughput is not decisive. It is low priority.
- **B-5:** no evidence yet that a learned policy can identify the rare useful non-greedy move. Effects of a few units, high stochasticity, and expensive labels make this an especially poor RL problem.
- **B-3:** the credit-assignment noise from an entire ABC run will be enormous. Unless B-2 demonstrates a much larger seeding lever, this is unlikely to pay.
- **Further path relinking/back-jump variants:** stop. Your negative results are direct and relevant.

## 2. What the success/failure pattern says

The evidence points to a **basin-access problem**, with an important qualification: it may actually be an **N2 trajectory-depth/connectivity problem** masquerading as a global-diversification problem.

Your observations are mutually consistent:

- Random restarts work because independent starts reach different N2 basins.
- Mixed seeds work better eventually despite worse initial makespan because they widen the basin distribution.
- Path relinking produces a quality-neutral plateau because paths between elite solutions stay inside, or quickly fall back into, the same broad attraction region.
- Back-jumping fails because revisiting earlier points on a short trajectory does not create genuinely new structure.
- The faster memetic algorithm loses because producing more population generations does not imply producing more independent or better local-search trajectories.

The critical detail is the **15 consecutive non-improving iterations**. For tabu search, 15 non-improving moves is extremely shallow. A tabu search is useful precisely because it crosses worsening regions for tens, hundreds, or thousands of moves. With a 15-move stagnation cutoff, your component may behave more like a mildly non-monotone descent method than the TSAB/i-TSAB search studied in the literature.

That makes this interpretation falsifiable:

> If the deficiency is inadequate trajectory depth, occasional long tabu calls should increase the probability of improvements occurring more than 15 moves after the last incumbent improvement and should improve final makespan even when the total number of evaluated moves is held approximately constant.

Instrument this before changing anything. For every call, record:

- number of iterations;
- iterations since the last call-local best;
- minimum worsening depth before the next improvement;
- whether termination was caused by 15 iterations or 2 seconds;
- starting and ending makespan;
- number of distinct solutions or move attributes visited.

If nearly every call ends at 15–30 iterations and improvements after a worsening excursion are rare only because excursions are truncated, that is the smoking gun.

## 3. New experiments

### A. Occasional deep tabu burst — highest priority

Small diff, one new parameter:

> Every time the current run has gone \(0.1L\) seconds without improving its global best, apply one tabu call to the incumbent best with a limit of **1,000 consecutive non-improving iterations**; retain the existing 2-second cap.

The single new value is 1,000; the trigger can be tied to the already fixed run horizon rather than tuned.

Why it could produce more than three units: it changes the reachable part of the search graph rather than merely selecting a better start. If 2 seconds binds first, the effective experiment becomes “use the whole allowed local-search call on the incumbent,” which is still informative.

A cleaner equal-cost variant is to replace, not add, random tabu calls until the same measured CPU cost has been spent.

### B. A broader critical-path neighbourhood

Add a critical-operation insertion/relocation neighbourhood and alternate it with N2. Predeclare:

> Use the insertion neighbourhood on every tenth tabu iteration; N2 on the other nine.

The one new parameter is frequency \(1/10\).

N2 is deliberately small and efficient, but its edges do not provide all useful transitions. The post-TSAB literature’s most meaningful advances are often not another population metaphor; they use richer block moves, insertion/ejection moves, or a second search representation.

This has a plausible 4–19-unit mechanism: a relocation can effect several adjacent exchanges at once and cross barriers that a short N2 tabu trajectory cannot cross. It is much more credible than learning to rank the same N2 moves.

Feasibility and exact evaluation must be validated carefully. Do not compare it at equal iteration count; compare at equal wall time or neighbour-evaluation cost.

### C. Multi-critical-path sampling

Your neighbourhood may depend on a single reconstructed critical path even when the schedule has many. If so, enumerate moves from the union of critical blocks on several distinct critical paths.

One fixed parameter:

> Generate at most **four** critical paths, obtained by randomized predecessor tie-breaking.

This is a small change if critical-path extraction already exists. The hypothesis is concrete: improvements found through moves absent from the canonical single-path N2 set should occur often enough to reduce final mean makespan.

This is especially attractive when many operations have zero slack and path multiplicity is high. If your implementation already considers the full critical graph rather than one path, this proposal is inapplicable.

### D. Perturb-and-reoptimise the incumbent

When stagnation reaches \(0.1L\), make a structured perturbation of exactly:

> **Three** feasible critical-block moves, chosen randomly without evaluating makespan, clear tabu memory, then run the normal tabu search.

This is an iterated-local-search “kick,” not back-jumping. Back-jumping goes to an already visited solution; the kick deliberately exits the current basin. It also differs from a full restart because it retains useful backbone structure.

Three is large enough not to be an immediate reversal and small enough not to become random reconstruction. Measure machine-order distance before and after the kick and whether the subsequent local optimum differs from the pre-kick optimum.

### E. Exact or CP large-neighbourhood repair

Freeze most machine sequences and release a small bottleneck subproblem—e.g. all operations on the two machines contributing most critical arcs—then give a CP solver a short fixed repair window.

One parameter:

> **0.5 seconds** per repair.

This is not necessarily a small dependency change, but the conceptual integration is small. CP/local-search hybrids have shown that propagation can complement tabu search and reduce variability, rather than simply duplicate it. Beck, Feng, and Watson reported significant improvement over a state-of-the-art tabu search from such a combination ([INFORMS paper](https://pubsonline.informs.org/doi/10.1287/ijoc.1100.0388)).

For your narrow objective, target repair at the incumbent only and invoke it late in a run. This is one of the few approaches with a credible mechanism for finding a single-digit improvement that N2 repeatedly misses.

### F. Heavy-tailed, not adaptive, restart lengths

Before implementing B-7, a simpler experiment is to randomise run length around the class optimum:

> Draw each run limit uniformly from \(\{L/2,L,2L\}\), truncated by remaining campaign time.

No continuous parameter needs tuning. This protects against instances where most useful runs are short but rare breakthroughs require longer trajectories. It is particularly relevant for 30×20, whose last-improvement median is already 0.48 of budget.

## 4. What the literature says you may be missing

### i-TSAB / TSAB

The important lesson is not “implement back-jump.” Watson, Howe, and Whitley’s controlled deconstruction found that the effectiveness of i-TSAB must be attributed to particular interacting components rather than to a generic tabu label or folklore about long-term memory ([paper](https://www.sciencedirect.com/science/article/pii/S030505480500242X)).

Your back-jump result is a known negative result **for your solver** and should be respected. It does not show that TSAB-style deep trajectories are useless, because your calls terminate after only 15 consecutive non-improvements. Back-jumping around shallow searches and running a genuinely deep tabu trajectory are different interventions.

What matters from that family:

- N2’s efficient critical-block restriction;
- sustained non-improving trajectories;
- appropriate recency memory and cycling avoidance;
- restart/diversification between trajectories.

What does not deserve another test now: bolt-on back-jump alone.

### Balas–Vazacopoulos GLS and shifting bottleneck

This is structurally interesting because it searches with neighbourhood trees and combines a different neighbourhood structure with shifting bottleneck; the authors explicitly attribute the hybrid’s effectiveness to the complementarity of those structures ([Management Science paper](https://doi.org/10.1287/mnsc.44.2.262)).

The transferable lesson is **neighbourhood complementarity**, not necessarily implementing the full historical algorithm. An insertion/ejection or bottleneck-machine repair operator is worth testing. A full GLS/shifting-bottleneck reimplementation is not a small diff.

There is no known general negative result saying these moves do not help Taillard. Your own evidence has not tested them.

### TS/SA hybrids

Zhang et al.’s TS/SA method used SA to find different elite solutions in the big valley and tabu search to re-intensify from them; it reported 17 then-new upper bounds ([Computers & Operations Research paper](https://www.sciencedirect.com/science/article/pii/S0305054806000670)).

But your population plus restarts already supplies a temperature-free form of basin resampling. Adding SA acceptance around the same N2 moves is therefore not my first choice. The historical result is positive, but its mechanism overlaps with something your solver already does successfully.

I would test structured kicks or heavy-tailed restarts before SA.

### Peng–Lü–Cheng path relinking

Their algorithm was not simply “take two elites and walk between them.” It used a specialised path-construction method, reference-solution mechanism, and embedded tabu search, and reported 49 improved upper bounds over 205 instances ([paper](https://www.sciencedirect.com/science/article/pii/S0305054814002160)).

Nevertheless, your approximately 500 calls with zero improvements are decisive for the implementation and population geometry you have. This is a **known negative result in your setting**. Unless distance traces show that your relinking paths were degenerate or your implementation omitted the embedded reoptimisation that defines the published method, drop it. Do not assume a more elaborate reference-set policy will turn a zero-hit operator into a 10-unit lever.

### BRKGA / random keys

A new representation can provide smoother recombination and implicit diversity, but it would replace substantial parts of the solver and conflict with your “small diff” constraint. More importantly, your faster memetic alternative already indicates that changing the evolutionary shell is not the obvious bottleneck.

I would not pursue BRKGA for this objective. It is not known universally to fail, but your evidence makes it a poor bet.

### CP/local-search hybrids

This is the most relevant literature direction after neighbourhood expansion. The CP/local-search hybrid of Beck, Feng, and Watson improved state-of-the-art tabu search and reduced solution-quality variability ([INFORMS](https://pubsonline.informs.org/doi/10.1287/ijoc.1100.0388)). That is unusually aligned with your needs: low-variance improvement of near-best schedules, not merely faster construction.

The right form is a restricted large-neighbourhood repair seeded by your incumbent, not “hand the entire Taillard instance to CP for 150 seconds.”

### Guided ejection / partial-schedule search

Later guided local search using iterative ejections works in partial-solution space, solves restricted feasibility problems under a tighter makespan bound, and uses tabu search for reoptimisation ([Computers & Operations Research](https://www.sciencedirect.com/science/article/pii/S0305054817302460)). Again, the important feature is a move outside the complete-schedule N2 graph.

This is probably more relevant than a new crossover or learned move ranker, though it is a larger implementation effort.

## 5. Measurements I would reinterpret

### “Restart beats one long run”

The conclusion is strongly suggestive, but not yet fully established.

Truncated traces are valid for estimating the outcome of a run stopped at \(L\), since the solver does not know the horizon. Combining prefixes from **independent seeds** can estimate \(k\) independent restarts. However:

- choosing the best \(L\) per class on the same traces introduces selection optimism;
- repeatedly recombining the same finite set of traces understates uncertainty;
- “21 of 22” ignores effect magnitude and dependence across instances;
- actual restart overhead and RNG/initialisation behaviour are absent;
- a best-of-\(k\) comparison is a lower-tail comparison, for which 20 runs or similar may be quite imprecise.

So the correct current claim is:

> The traces provide strong evidence that fixed restarts dominate 300-second runs under the tested configuration; real independent runs are needed to estimate the magnitude and validate the selected horizons.

Also, your evaluation budgets are listed as 40/100/150 seconds while the trace optimum for 20×15 is 30 seconds. Make the distinction explicit; otherwise the restart prescription and experimental budget appear inconsistent.

### “61.8% of time is after the last improvement”

This is descriptive, not proof that the time was wasted. Every successful stochastic search has a long post hoc interval after its final improvement, because “final” is defined using the future. Stopping exactly at that time is impossible.

What matters is the conditional improvement hazard after \(s\) seconds of stagnation. B-7 should be justified from that hazard, not from the retrospective fraction alone.

### “Throughput is not the binding constraint”

The memetic result establishes only:

> Generation count in that alternative evolutionary architecture is not predictive of solution quality.

It does **not** establish that computational throughput is irrelevant. The ABC may spend more computation per generation on better local-search calls. Faster evaluation could still buy:

- more independent restarts;
- deeper tabu trajectories;
- richer neighbourhoods;
- more exact/CP repairs.

The useful throughput units are therefore independent basin entries, tabu moves, exactly evaluated neighbours, and improvements per CPU second—not generations.

### Seeding effect

The four-instance \(-2.4\) to \(-3.6\) result is encouraging but is not yet evidence of a general three-unit effect. It is development-set evidence, and the effect is below half a run-level standard deviation. Wait for the 21-instance confirmation.

Your fuzzy/interval study supports the hypothesis that diversity/composition matters, but it does not establish the crisp-Taillard effect size. Treat it as prior evidence, not confirmation.

### Statistical target

A paired Wilcoxon over 21 per-instance means tests a consistent instance-level shift. It is poorly aligned with a pure record objective, where one breakthrough on ta30 is worth more than small improvements on 15 instances.

Keep that test for algorithm selection, but add prespecified record-oriented endpoints:

- probability of matching or beating BKS on ta30;
- probability of reaching BKS+\(d\) for each shortlist instance;
- best verified makespan after a fixed total campaign budget;
- empirical expected shortfall in the lowest 5% or 10% of runs.

For BKS hunting, the lower tail is the product.

## Bottom line

I would spend the next cycles as follows:

1. Validate real fixed restarts.
2. Start record attempts, especially ta30.
3. Instrument tabu trajectory depth.
4. Test incumbent-always allocation.
5. Test an occasional 1,000-stagnation deep tabu burst.
6. Add a complementary insertion or multi-critical-path neighbourhood.
7. Try adaptive or heavy-tailed restart lengths.
8. Only then invest further in seed recipes or CP repair.

I would stop work on back-jumping, ordinary path relinking, a replacement memetic shell, learned N2 selection, and end-to-end RL seeding. The central experiment is whether your “tabu search” is simply being terminated before tabu search’s characteristic mechanism has time to operate.
