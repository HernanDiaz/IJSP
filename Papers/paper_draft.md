# Neighbourhood Structures and Ranking Operators for the Interval Job Shop Scheduling Problem

*Draft — April 2026*

---

## Abstract

The Job Shop Scheduling Problem (JSP) with interval-valued processing times (IJSP) models real-world uncertainty where exact durations are unknown but bounded. In this setting, comparing schedules requires an interval ranking operator, and the classical notion of critical path must be extended to account for both extreme scenarios. We formalise the concept of **extreme critical paths** — paths that are critical in the best-case or worst-case scenario graph — and use them to define a neighbourhood H(σ) for local search. We prove that H(σ) satisfies the three key properties for a neighbourhood structure: feasibility, connectivity, and no-loss-of-improving-neighbours. We then adapt three neighbourhoods from the JSP literature (N1, N2, N3) and propose a new one (N_ext, inspired by Nowicki & Smutnicki) to the interval setting using the H(σ) framework. We evaluate all four neighbourhoods combined with four interval ranking operators (EV, LEX1, LEX2, YX) in a local-search-based metaheuristic on 82 benchmark instances. Experimental results show that N2 and N_ext offer the best makespan quality while dramatically reducing neighbourhood size relative to N1. A novel empirical finding is that the LEX2 operator (upper-bound-first lexicographic order) consistently produces solutions with ~5–11% narrower makespan intervals than the other operators. A structural float analysis of both extreme graphs confirms that this is a direct effect of the ranking mechanism — LEX2 drives the search toward tighter worst-case critical paths, rather than leaving more structural slack — providing a rigorous explanation for the interval-width advantage.

**Keywords:** Job shop scheduling, interval uncertainty, critical paths, neighbourhood structures, interval ranking, local search

---

## 1. Introduction

The Job Shop Scheduling Problem (JSP) is one of the most studied combinatorial optimisation problems in operations research. In many real-world applications, however, processing times are not known precisely at the time of planning: they depend on operator experience, machine wear, material quality, and other variable factors. A natural and tractable model for such uncertainty is to represent each processing time as a **closed interval** [p⁻, p⁺], where p⁻ and p⁺ are the minimum and maximum possible durations.

The resulting **Interval Job Shop Scheduling Problem (IJSP)** requires comparing schedules whose makespan is itself an interval, making the notion of "optimality" dependent on the choice of interval ranking operator. Furthermore, the classical concept of a critical path — fundamental to neighbourhood design in the deterministic JSP — must be reconsidered: which path is critical when processing times are uncertain?

### 1.1 Contributions

This paper makes the following contributions:

1. **Formal definition of extreme criticality for IJSP** (Section 3): We define a task/arc as *extreme critical* if it belongs to a critical path in the best-case graph G⁻(σ) or the worst-case graph G⁺(σ). We prove this definition encompasses all necessarily critical tasks and is contained within the set of possibly critical tasks, making it a computationally tractable and theoretically sound approximation.

2. **Neighbourhood H(σ) based on extreme critical arcs** (Section 4): We define H(σ) as all feasible processing orders reachable by reversing a single extreme critical disjunctive arc. We prove:
   - **Feasibility**: every neighbour in H(σ) is a feasible processing order.
   - **No-loss-of-improving-neighbours**: neighbours outside H(σ) can never improve the makespan under any of the considered rankings.
   - **Connectivity**: every non-optimal order can reach a global optimum via a finite sequence of moves in H(σ).

3. **Adaptation and comparison of four neighbourhood structures** (Section 5): We adapt N1, N2, N3, and N_ext to the IJSP using the H(σ) framework, characterising their theoretical size and the computational complexity of generating them.

4. **Extensive empirical study with 82 benchmark instances** (Section 6): We evaluate all 4 neighbourhoods × 4 ranking operators (16 configurations), with 30 independent runs per instance, measuring makespan quality, neighbourhood size per local search step, and — for the first time — the width of the makespan interval of returned solutions.

5. **Discovery and structural explanation of the LEX2–interval-width effect** (Sections 6.4, 6.6): LEX2, the upper-bound-first ranking, systematically produces solutions with ~5–11% narrower makespan intervals than other operators. We show via a float analysis of both extreme graphs that this effect is mechanistic — LEX2 directly reduces C_max⁺, tightening the G⁺ critical path (more critical operations, less per-operation slack) — rather than arising from a structurally looser worst-case graph.

### 1.2 Paper Organisation

Section 2 introduces the IJSP formally and reviews related work. Section 3 defines extreme criticality. Section 4 defines the neighbourhood H(σ) and proves its properties. Section 5 describes the four neighbourhood variants. Section 6 presents the experimental study. Section 7 discusses findings and implications. Section 8 concludes.

---

## 2. Background and Related Work

### 2.1 The Deterministic Job Shop Problem

The JSP consists of n jobs {J₁, ..., Jₙ} and m machines {M₁, ..., Mₘ}. Each job Jᵢ is a sequence of operations Oᵢ₁, ..., Oᵢₘ, where operation Oᵢⱼ must be processed on machine Mⱼ for exactly pᵢⱼ time units. Operations within a job must respect precedence, and each machine processes at most one operation at a time. The goal is to find a *feasible processing order* σ minimising makespan Cmax.

The *solution graph* G(σ) = (V, A ∪ D(σ)) represents a feasible order: A contains precedence arcs within jobs, and D(σ) contains disjunctive arcs imposed by the machine order σ. The makespan equals the length of the longest path in G(σ).

A key insight from [van Laarhoven 1992] is that the neighbourhood for local search should restrict to reversals of arcs on critical paths, ensuring feasibility and providing the connectivity property [Nowicki & Smutnicki 1996].

### 2.2 Interval Arithmetic and Ranking Operators

When processing times are unknown but bounded, we represent each as **p** = [p⁻, p⁺]. Standard interval arithmetic extends real operations:

- **a** + **b** = [a⁻ + b⁻, a⁺ + b⁺]
- max(**a**, **b**) = [max(a⁻, b⁻), max(a⁺, b⁺)]

Since there is no natural total order on intervals, comparing schedules requires choosing a ranking operator. We consider four total orders:

| Ranking | Definition | Intuition |
|---------|-----------|-----------|
| **LEX1** | **a** ≤ **b** iff a⁻ < b⁻, or a⁻ = b⁻ and a⁺ ≤ b⁺ | Minimise lower bound first |
| **LEX2** | **a** ≤ **b** iff a⁺ < b⁺, or a⁺ = b⁺ and a⁻ ≤ b⁻ | Minimise upper bound first |
| **YX**  | **a** ≤ **b** iff a⁻+a⁺ < b⁻+b⁺, or equal sum and a⁺-a⁻ ≤ b⁺-b⁻ | Minimise sum, then width |
| **EV**  | **a** ≤ **b** iff (a⁻+a⁺)/2 ≤ (b⁻+b⁺)/2 | Minimise expected/midpoint value |

LEX1, LEX2, and YX are *admissible* total orders [Bustince 2013]: they refine the product order (if a⁻ ≤ b⁻ and a⁺ ≤ b⁺ then **a** ≤ᴿ **b** for all three). EV (midpoint) also has this property, though it is not a strict total order on intervals.

### 2.3 Related Work on IJSP Neighbourhoods

The first neighbourhood for the fuzzy/interval JSP was proposed in [González-Rodríguez et al., ICAPS 2008], extending the van Laarhoven neighbourhood to use possibly critical arcs. This corresponds to what we call N1 in our framework. Subsequent work defined N2 as a filtered version of N1 using a lower bound criterion [cite N2fjsp], reducing neighbourhood size ~64% with identical solution quality. N3 extends the neighbourhood to consider subsequence reversals [cite N3fjsp].

The Nowicki-Smutnicki N_ext neighbourhood [Nowicki & Smutnicki 2005] was originally proposed for the crisp JSP, providing strong empirical performance. Its adaptation to the IJSP, which we introduce here as N_ext, is one of the contributions of this paper.

---

## 3. Extreme Criticality for IJSP

### 3.1 Configurations and Extreme Scenario Graphs

A *configuration* ω = (p₁, ..., p_o) assigns a deterministic duration pᵢ ∈ **pᵢ** to each operation i. The set of all configurations is Ω. Given a feasible processing order σ, we define two *extreme solution graphs*:

- **G⁻(σ)**: identical to G(σ) but with each arc (x,y) weighted by p_x⁻ (best-case durations)
- **G⁺(σ)**: identical to G(σ) but with each arc (x,y) weighted by p_x⁺ (worst-case durations)

These correspond to the extreme scenarios ω⁺(∅) and ω⁺(V) respectively. The makespan interval satisfies:
- C_max⁻(σ) = length of longest path in G⁻(σ)
- C_max⁺(σ) = length of longest path in G⁺(σ)

### 3.2 Heads and Tails

For a solution graph G(σ), the *head* **r**_x = [r_x⁻, r_x⁺] of task x is its earliest start time interval, and the *tail* **q**_x = [q_x⁻, q_x⁺] is the length of the longest path from the completion of x to the end node. For each extreme graph G*(σ), r_x* is the length of the longest path from start to x, and q_x* + p_x* is the length of the longest path from x to end.

**Proposition 1** (Properties of critical arcs). *If arc (x,y) is extreme critical, then* r_x⁻ + p_x⁻ = r_y⁻ *or* r_x⁺ + p_x⁺ = r_y⁺. *A task x is extreme critical if and only if* r_x* + p_x* + q_x* = C_max*(σ) *for some* * ∈ {−, +}.

### 3.3 Definition and Relationship to Possibly/Necessarily Critical Tasks

**Definition 1** (Extreme critical path). *A path P in G(σ) is* extreme critical *if P is critical in G⁻(σ) or in G⁺(σ). A node, arc, or block is extreme critical if it lies on an extreme critical path.*

This definition connects to the general criticality literature as follows:

**Proposition 2.** *Every extreme critical task is possibly critical. Every necessarily critical task is extreme critical.*

*Proof sketch.* If task i is critical in G⁻(σ) or G⁺(σ), then F_i(ω⁺(∅)) = 0 or F_i(ω⁺(V)) = 0, so F_i⁻ = min_Ω F_i(ω) ≤ 0, making i possibly critical. Conversely, if i is necessarily critical, F_i⁺ = 0 = max_Ω F_i(ω) ≥ F_i(ω⁺(∅)), so i is critical in both extreme graphs. □

The extreme criticality concept provides a computationally tractable approximation: computing critical paths in G⁻(σ) and G⁺(σ) each takes O(|E|) via standard longest-path algorithms, while computing the full set of possibly critical tasks is NP-hard [Artigues 2015].

---

## 4. Neighbourhood H(σ) and Its Properties

### 4.1 Definition

**Definition 2** (Neighbourhood H(σ)). *Given a feasible processing order σ, let σ_(v) denote the processing order obtained by reversing disjunctive arc v ∈ D(σ). The neighbourhood of σ is:*

> H(σ) = { σ_(v) : v ∈ D(σ) is extreme critical }

For comparison, the larger neighbourhood based on possibly critical arcs is:

> H'(σ) = { σ_(v) : v ∈ D(σ) is possibly critical }

H(σ) ⊂ H'(σ) always, and the inclusion is strict in general.

### 4.2 No Loss of Improving Neighbours

**Proposition 3.** *Let σ ∈ Σ and let σ_(v) be obtained by reversing an arc v that is not extreme critical in G(σ). Then C_max⁻(σ) ≤ C_max⁻(σ_(v)) and C_max⁺(σ) ≤ C_max⁺(σ_(v)), and therefore* **C_max**(σ) ≤_R **C_max**(σ_(v)) *for all rankings R.*

*Proof sketch.* Reversing a non-critical arc (x,y) replaces it with the reverse arc (y,x). The critical arcs of G⁻(σ) and G⁺(σ) remain unchanged or are replaced by longer alternative paths, so the makespan can only increase in both extreme scenarios. Since all considered rankings refine the product order, the interval makespan does not improve. □

**Corollary 1.** *Neighbours in H'(σ) \ H(σ) can never improve the makespan under any ranking R.*

This justifies using H(σ) instead of H'(σ): we reduce the neighbourhood to extreme critical arcs at no cost in solution quality.

### 4.3 Feasibility

**Theorem 1** (Feasibility). *For any feasible σ ∈ Σ, the reversal of any extreme critical arc v = (x,y) produces a feasible processing order σ_(v) ∈ Σ. Hence H(σ) ⊂ Σ.*

*Proof sketch.* If (x,y) is critical in G*(σ) for some * ∈ {−,+}, then r_y* = r_x* + p_x*. Suppose G(σ_(v)) contains a cycle; then there exists an alternative path from x to y in G(σ). Combining this with the criticality condition yields r_x* + p_x* ≥ r_x* + p_x* + p_SJ_x* + p_PJ_y*, which is impossible since all durations are strictly positive. □

Feasibility is essential for practical local search: it eliminates the need for feasibility checks on neighbours, avoiding both computational overhead and the loss of feasible solutions.

### 4.4 Connectivity

**Theorem 2** (Connectivity). *For every non-optimal σ ∈ Σ, there exists a finite sequence of moves in H(σ) leading to a globally optimal order σ₀.*

*Proof sketch.* Define V_σ(σ₀) = {(x,y) ∈ D(σ) : (x,y) is extreme critical, (y,x) ∈ D̄(σ₀)}, where D̄ denotes transitive closure. First, if V_σ(σ₀) = ∅ then σ is optimal (proved via the product order refinement property of the rankings). The sequence λ₀ = σ, λ_{k+1} = reversal of some arc in V_{λ_k}(σ₀), decreases |M̄_σ(σ₀)| by 1 at each step and terminates at an optimal order. □

---

## 5. Neighbourhood Variants for IJSP

All four variants apply the H(σ) framework of Section 4 — they differ only in which subset of extreme critical arcs they consider. We describe each variant, together with the computational cost of generating the neighbour set.

### 5.1 N1: All Extreme Critical Arcs

N1 includes every disjunctive arc (x,y) such that x immediately precedes y on the machine in G(σ) and the arc lies on a critical path in G⁻(σ) or G⁺(σ). No positional filter is applied within the block: both boundary arcs (first and last arc of a critical block) and interior arcs are candidates. Formally, N1 evaluates all consecutive arc pairs on all extreme critical paths in G⁻(σ) and G⁺(σ).

Neighbourhood size: ~61 candidate moves per step (empirical average across 82 instances). Generation cost: O(|E|) for each extreme graph.

### 5.2 N2: Block-Boundary Filter

N2 restricts N1 to arcs at the **boundary** of each extreme critical block. A block B = {o₁, ..., oₖ} is a maximal sequence of consecutive operations on the same machine all belonging to a critical path. N2 retains only the two boundary arcs of each such block:
- The arc (o_{k−1}, oₖ): last arc of the block (o_{k−1} has no machine predecessor on the critical path, or oₖ has no machine successor on the critical path).
- The arc (o₁, o₂): first arc of the block (symmetrically).

Interior arcs (oᵢ, oᵢ₊₁), 2 ≤ i ≤ k−2, are excluded. This is a direct adaptation of the result of [Nowicki & Smutnicki 1996] to the interval setting: reversing an interior arc of a critical block cannot improve the makespan in either extreme graph G⁻(σ) or G⁺(σ), so N2 ⊂ N1 with no loss of improving neighbours.

Neighbourhood size: ~22 candidate moves per step (~64% reduction vs N1). Solution quality: statistically identical to N1 across all 82 instances and all 4 operators. This empirically confirms the theoretical guarantee: the interior arcs excluded by N2 carry no improving moves, consistent with the Nowicki-Smutnicki result extended to both extreme graphs simultaneously.

### 5.3 N3: Subsequence Reversals

N3 extends the search space beyond single arc swaps by also considering reversals of entire contiguous subsequences of operations within a critical block. For a block B = {o₁, ..., oₖ}, in addition to the single-arc swaps of N1, N3 considers every contiguous pair (oᵢ, oⱼ) with j > i+1 as a multi-step reversal candidate. This generates a strictly larger neighbourhood than N1.

Neighbourhood size: ~34 candidate moves per step. Makespan quality: worse than N1/N2/N_ext across all operators, despite the larger neighbourhood — a finding we analyse in Section 7.2.

### 5.4 N_ext: Extended Nowicki–Smutnicki Neighbourhood

N_ext adapts the extended neighbourhood of [Nowicki & Smutnicki 2005] to the IJSP. It starts from N2 (boundary arcs only, unconditionally included) and additionally considers interior block arcs that pass a heads-and-tails viability check `isViableSwap()`. This combines the guaranteed coverage of N2's boundary swaps with selective exploration of interior swaps that are estimated to improve the makespan.

Formally, for each extreme critical block B = {o₁, ..., oₖ}:
- The boundary arcs (o_{k−1}, oₖ) and (o₁, o₂) are always included (as in N2).
- Interior arcs (oᵢ, oᵢ₊₁), 2 ≤ i ≤ k−2, are included only if `isViableSwap(oᵢ, oᵢ₊₁)` returns true.

`isViableSwap(x, y)` computes an optimistic lower-bound estimate of the makespan after swapping x and y by recalculating only the heads and tails of x and y locally, without propagating changes through the full graph. Because the estimate is a lower bound, discarding a swap when `isViableSwap` returns false is theoretically safe: the real makespan after the swap cannot be better than the estimate.

Neighbourhood size: ~35 candidate moves per step. Makespan quality: comparable to N1/N2 — the best performing group.

---

## 6. Experimental Study

### 6.1 Setup

**Instances.** We use 82 benchmark instances constructed by perturbing classical JSP benchmarks (abz, ft, la, tai families) with uniform interval uncertainty at 15% width: p_ij = [p*_ij × 0.925, p*_ij × 1.075], where p*_ij is the nominal processing time. The Taillard instances cover sizes 15×15, 20×15, 20×20, 30×15, 30×20, 50×15, and 50×20 (10 instances each), with the largest having 1000 operations. The classical benchmarks (abz7–9, ft10, ft20, la21–40) range from 55 to 300 operations.

**Algorithm.** Each configuration (neighbourhood × ranking) runs an iterated local search (ILS) with:
- Neighbourhood evaluation as described in Section 5
- Best-improvement strategy within each local search call
- Perturbation: random block reversal when stuck in local optima
- 30 independent runs per instance, fixed time budget

**Configurations.** 4 neighbourhoods × 4 ranking operators = 16 configurations. Total: 16 × 82 × 30 = 39,360 runs.

**Metrics.** For each configuration and instance, we report:
- *Media Makespan*: average interval makespan over 30 runs, compared by midpoint
- *Mejor Makespan*: best interval makespan found over 30 runs
- *Neighbourhood size*: average number of moves evaluated per local search step
- *Improvement rate*: fraction of local search steps where at least one improving neighbour was found
- *Solution width*: average and best width (C_max⁺ − C_max⁻) of returned solutions

### 6.2 Makespan Quality

**Table 1.** Global average makespan (midpoint) over 82 instances.

| Neighbourhood | EV     | LEX1   | LEX2   | YX     |
|---------------|--------|--------|--------|--------|
| N1            | 1888.9 | 1894.6 | 1889.5 | 1887.0 |
| N2            | 1888.7 | 1895.2 | 1889.2 | 1887.1 |
| N3            | 1907.3 | 1914.2 | 1908.5 | 1904.9 |
| N_ext         | 1889.0 | 1891.0 | 1895.5 | 1887.2 |

**Key observations:**
- **N1, N2, and N_ext form the best group**, with differences of less than 0.3% between them.
- **N3 is ~1% worse** than the best group, despite a larger neighbourhood — additional moves explore non-improving regions.
- **YX produces the best average makespan** across all neighbourhoods, closely followed by EV.
- **LEX1 consistently produces the worst makespan**, suggesting that optimising the lower bound first is not an effective strategy for overall makespan minimisation.

**Table 2.** Best makespan over 30 runs (global average).

| Neighbourhood | EV     | LEX1   | LEX2   | YX     |
|---------------|--------|--------|--------|--------|
| N1            | 1856.2 | 1859.8 | 1855.5 | 1853.5 |
| N2            | 1856.4 | 1858.5 | 1855.7 | 1855.6 |
| N3            | 1872.3 | 1874.5 | 1872.2 | 1870.9 |
| N_ext         | 1855.1 | 1854.6 | 1862.6 | 1853.6 |

For *best* makespan, YX and N_ext together win the most instances. Notably, N_ext with LEX1 produces the best single result across all instances for this metric (1854.6), while N_ext with LEX2 is notably worse (1862.6) — LEX2 optimises the upper bound, which may diverge from the midpoint-based comparison used here.

**Win-loss analysis (N_ext vs other neighbourhoods, EV ranking):**

| Comparison   | N_ext wins | N_ext loses |
|-------------|-----------|------------|
| vs N1       | 42        | 40         |
| vs N2       | 33        | 49         |
| vs N3       | 80        | 2          |

N_ext and N2 are statistically indistinguishable in pairwise comparisons (~50/50), while both clearly dominate N3.

### 6.3 Neighbourhood Size and Improvement Rate

**Table 3.** Average neighbourhood size and improvement rate per local search step.

| Neighbourhood | Avg. moves/step | Improvement rate |
|---------------|----------------|-----------------|
| N1            | ~61            | 87.0%           |
| N2            | ~22            | 87.0%           |
| N_ext         | ~35            | 87.0%           |
| N3            | ~34            | 85.7%           |

N1, N2, and N_ext share identical improvement rates (87%), confirming the theoretical result of Proposition 3: the pruned neighbours (in N2/N_ext relative to N1) were never improving moves.

**Effect of ranking on neighbourhood size.** An unexpected finding is that LEX2 evaluates ~6–7% *more* neighbours per local search step than EV or YX, consistently across all four neighbourhoods:

| Ranking | Avg. moves/step (N_ext) |
|---------|------------------------|
| EV      | 34.1                   |
| LEX1    | 35.0                   |
| LEX2    | 36.6                   |
| YX      | 34.2                   |

This suggests that optimising the upper bound (LEX2) identifies more viable swaps in the extreme graph G⁺(σ), activating more interior block arcs in the isViableSwap check. This effect is theoretically connected to the tighter bound conditions that arise when the upper bound is not yet at its minimum.

### 6.4 Solution Interval Width

A novel dimension of analysis is the *width* of the returned makespan interval: C_max⁺ − C_max⁻. This measures the *uncertainty* of the solution — a narrower interval means the planner knows more precisely what the actual makespan will be, regardless of which processing time scenario occurs.

**Table 4.** Average makespan interval width by ranking operator (mean over all 4 neighbourhoods and 82 instances, best solution per run).

| Ranking  | Avg. Width | vs. EV    |
|----------|-----------|-----------|
| EV       | 269.3     | —         |
| LEX1     | 286.4     | +6.4%     |
| **LEX2** | **255.0** | **−5.3%** |
| YX       | 267.5     | −0.7%     |

**LEX2 produces intervals that are ~5% narrower than EV and YX, and ~11% narrower than LEX1.** This effect is remarkably consistent across all four neighbourhoods: widths range from 251 to 259 for LEX2, and from 267 to 272 for EV, with the ordering EV ≻ YX ≻ LEX1 ≻ **LEX2** holding in every neighbourhood (Table 5).

**Table 5.** Interval width by neighbourhood × ranking operator (best solution per run, 82 instances).

| Neighbourhood | EV  | LEX1 | LEX2 | YX  |
|--------------|-----|------|------|-----|
| N1           | 269 | 284  | 254  | 267 |
| N2           | 267 | 284  | 254  | 265 |
| N3           | 272 | 288  | 256  | 268 |
| N_ext        | 268 | 283  | 251  | 266 |

The neighbourhood choice has negligible effect on interval width (< 3 units variation within any operator); the width effect is entirely driven by the choice of ranking operator.

**Interpretation.** If a decision-maker's goal is to *reduce schedule uncertainty* rather than minimise the expected makespan, LEX2 is the superior choice. Conversely, if the goal is to minimise the expected value, YX slightly outperforms EV. These findings suggest that the choice of ranking operator should depend on the risk profile of the application.

### 6.5 Statistical Validation

The empirical comparisons in Sections 6.2–6.4 are validated with non-parametric statistical tests. Before presenting the results, an important methodological remark is necessary.

**Remark on parametric configuration.** All 16 configurations share an identical set of algorithm hyperparameters (population size, stopping criterion, perturbation strategy, etc.). No operator- or neighbourhood-specific tuning has been performed. This is a deliberate design choice that ensures *experimental control*: any observed difference in solution quality can be attributed solely to the neighbourhood structure or ranking operator, not to a configuration that was optimised in favour of one particular setting. The trade-off is that the results should not be interpreted as "operator X achieves the best possible makespan" — only as "operator X performs better than Y *under this common configuration*". A dedicated parametric study per operator, which lies outside the scope of this work, could reveal whether the ranking differences persist when each operator is allowed its own optimal hyperparameter setting.

**Test design.** Since all configurations use the same random seed sequence, each of the 30 runs per instance is paired across configurations. We treat each (instance, run) pair as a block, yielding up to 82 × 30 = 2460 paired blocks. We first apply the **Friedman test** (non-parametric equivalent of repeated-measures ANOVA) to detect overall differences among treatments, followed by **pairwise Wilcoxon signed-rank tests** with **Holm-Bonferroni** correction for multiple comparisons. Effect size is reported as r = |Z| / √N (thresholds: small ≥ 0.1, medium ≥ 0.3, large ≥ 0.5). The metric used is the midpoint of the makespan interval, (C_max⁻ + C_max⁺)/2, which provides a neutral common ground for all four operators.

**Operators.** The Friedman test across the four ranking operators yields χ²(3) = 694.96, p ≈ 10⁻¹⁵⁰, confirming that differences exist beyond chance. Pairwise results are shown in Table 6.

**Table 6.** Pairwise Wilcoxon tests for ranking operators (n = 2460 paired blocks, Holm-Bonferroni corrected).

| Comparison | Mean A | Mean B | Diff | p-adj | Effect r | Magnitude |
|-----------|--------|--------|------|-------|---------|-----------|
| LEX1 vs YX  | 1909.5 | 1901.5 | +8.0 | < 0.001 | 0.509 | large |
| EV vs LEX1  | 1903.3 | 1909.5 | −6.2 | < 0.001 | 0.415 | medium |
| LEX1 vs LEX2| 1909.5 | 1903.9 | +5.6 | < 0.001 | 0.349 | medium |
| LEX2 vs YX  | 1903.9 | 1901.5 | +2.4 | < 0.001 | 0.159 | small |
| EV vs YX    | 1903.3 | 1901.5 | +1.8 | < 0.001 | 0.131 | small |
| **EV vs LEX2**  | **1903.3** | **1903.9** | **−0.6** | **0.197** | **0.026** | **negligible** |

LEX1 is significantly worse than all other operators (medium-to-large effects). YX ranks first on average, but its advantage over EV (r = 0.131, small) and LEX2 (r = 0.159, small) is modest and, as noted above, may be partially explained by the common hyperparameter configuration. The comparison EV vs LEX2 is not statistically significant (p = 0.197, r = 0.026), confirming that these two operators are effectively equivalent in terms of average makespan under this setup. The operator ranking pattern — YX ≈ EV ≈ LEX2 ≻ LEX1 — is consistent across all four neighbourhoods (Friedman significant in all four, p < 10⁻²¹).

**Neighbourhoods.** The Friedman test across the four neighbourhoods yields χ²(3) = 4102.31, p ≈ 0, an overwhelming result. Pairwise comparisons (Table 7) reveal a clear partition into two groups.

**Table 7.** Pairwise Wilcoxon tests for neighbourhoods (n = 2460 blocks, Holm-Bonferroni corrected).

| Comparison | Mean A | Mean B | Diff | p-adj | Effect r | Magnitude |
|-----------|--------|--------|------|-------|---------|-----------|
| N1 vs N3    | 1890.1 | 1908.7 | −18.6 | < 0.001 | 0.725 | large |
| N2 vs N3    | 1890.1 | 1908.7 | −18.6 | < 0.001 | 0.723 | large |
| N_ext vs N3 | 1889.9 | 1908.7 | −18.8 | < 0.001 | 0.729 | large |
| **N1 vs N2**    | **1890.1** | **1890.1** | **+0.1** | **1.000** | **0.004** | **negligible** |
| **N1 vs N_ext** | **1890.1** | **1889.9** | **+0.2** | **1.000** | **0.018** | **negligible** |
| **N2 vs N_ext** | **1890.1** | **1889.9** | **+0.2** | **1.000** | **0.009** | **negligible** |

The results confirm — with overwhelming statistical certainty — the partition observed empirically: {N1, N2, N_ext} are statistically indistinguishable from each other (p = 1.0, r < 0.02 for all three pairs), while N3 is significantly worse. The practical implication is that N2 and N_ext achieve the same solution quality as N1 at a fraction of the computational cost (Section 6.3), without any statistical penalty.

### 6.6 Convergence Profiles

We extract per-generation best-solution quality from the "Evolution" section of every result file (30 runs × 1638 instance-config pairs), apply last-observation-carried-forward (LOCF) padding for runs that terminate early under the 25-generation-no-improvement stopping criterion, and average normalised traces across all 82 instances. Each trace is normalised to 1.000 at generation 0 (the initial random population) so instances of different sizes are commensurable; lower is better.

**Table 9.** Mean normalised best at key generation checkpoints (operator = EV; averaged over 82 instances). Normalisation: best[gen] / best[gen=0].

| Neighbourhood | gen 1  | gen 5  | gen 10 | gen 20 | gen 50 | final  | gen@90% |
|--------------|--------|--------|--------|--------|--------|--------|---------|
| N1           | 0.9104 | 0.8692 | 0.8523 | 0.8367 | 0.8230 | 0.8200 | 19      |
| N2           | 0.9106 | 0.8694 | 0.8524 | 0.8367 | 0.8227 | 0.8200 | 19      |
| N_ext        | 0.9101 | 0.8692 | 0.8525 | 0.8370 | 0.8230 | 0.8200 | 20      |
| N3           | 0.9151 | 0.8757 | 0.8598 | 0.8445 | 0.8304 | 0.8279 | 20      |

*gen@90% = first generation where 90% of the total quality improvement (gen0→final) has been achieved.*

**N1, N2, and N_ext have essentially identical convergence curves.** They reach the same final quality (0.820) via the same trajectory, and all achieve 90% of their improvement within 19–20 generations. N3 converges marginally slower and plateaus at a worse value (0.828).

**Table 10.** Mean normalised best by ranking operator (neighbourhood = N1).

| Operator | gen 1  | gen 5  | gen 10 | gen 20 | gen 50 | final  |
|---------|--------|--------|--------|--------|--------|--------|
| EV      | 0.9104 | 0.8692 | 0.8523 | 0.8367 | 0.8230 | 0.8200 |
| LEX1    | 0.9091 | 0.8684 | 0.8522 | 0.8373 | 0.8239 | 0.8209 |
| LEX2    | 0.9086 | 0.8681 | 0.8515 | 0.8363 | 0.8225 | 0.8189 |
| YX      | 0.9096 | 0.8683 | 0.8515 | 0.8358 | 0.8219 | 0.8190 |

The four operators converge at virtually the same rate — the differences across the entire convergence profile are below 0.002 in normalised units. This confirms that the operator choice affects *where* the search converges (the final solution quality with respect to each operator's objective) but not *how fast* it converges per generation. The convergence benefit of N2 and N_ext over N1 is therefore purely computational: they evaluate fewer candidate moves per generation at the same convergence speed, making each generation cheaper to compute.

### 6.7 Structural Analysis: Float Distribution in Extreme Graphs

The systematic interval-width advantage of LEX2 (Section 6.4) raises a natural question: *is the narrower interval a direct consequence of the ranking mechanism, or does LEX2 also produce solutions with more structural slack in G⁺(σ) — i.e., fewer operations on the critical path of the worst-case scenario?*

Formally, for a fixed permutation σ and extreme graph G*(σ), the **float** of operation Oᵢⱼ is:

> F*ᵢⱼ = C_max*(σ) − rᵢⱼ* − pᵢⱼ* − qᵢⱼ*

where rᵢⱼ* is the head (earliest start in G*), pᵢⱼ* the processing time (p⁻ or p⁺ depending on the scenario), and qᵢⱼ* the tail (earliest completion from Oᵢⱼ to the makespan arc). An operation is *extreme-critical* if F*ᵢⱼ = 0. We define two quantities:
- **n_crit_p**: number of critical operations in G⁺(σ) (F⁺ᵢⱼ = 0)
- **n_crit_only_p**: operations critical in G⁺ but not G⁻ — "newly critical" in the upper-bound graph

We reconstruct both G⁻(σ) and G⁺(σ) from the saved best-solution permutation and the instance file for each (config, instance) pair, and compute the float for all operations. Results for all 1312 records (82 instances × 16 configs) are summarised in Table 8.

**Table 8.** Mean float and critical-path structure in G⁻ and G⁺, by ranking operator (averages over all 5 neighbourhoods and 82 instances).

| Ranking | n_crit_m (%) | n_crit_p (%) | n_crit_only_p | mean_float_p |
|---------|-------------|-------------|---------------|-------------|
| EV      | 51.2 (11.4%) | 46.1 (10.3%) | 30.4          | 30.1        |
| LEX1    | 52.0 (11.7%) | 43.0  (9.9%) | 27.8          | 33.6        |
| **LEX2**| **46.7 (10.6%)** | **51.6 (11.4%)** | **35.4** | **28.4** |
| YX      | 48.4 (10.8%) | 48.0 (10.7%) | 32.4          | 29.9        |

*Percentages computed against mean n_ops = 483.5; n_crit_only_p counts operations critical in G⁺ but not G⁻.*

**The answer is unambiguous: LEX2 does NOT produce more structural slack in G⁺.** The opposite is true. LEX2 solutions have:
- **More** critical operations in G⁺ (51.6, vs 46.1 for EV and 43.0 for LEX1)
- **More** operations that are "newly" critical in the upper-bound graph (n_crit_only_p = 35.4 vs 30.4 for EV)
- **Less** per-operation float in G⁺ (mean_float_p = 28.4 vs 30.1 for EV and 33.6 for LEX1)

At the same time, LEX2 produces **fewer** critical operations in G⁻ (46.7 vs 51.2 for EV), indicating that the G⁻ critical path is relatively looser in LEX2 solutions. This is consistent with LEX2's ranking: it accepts moves that reduce C_max⁺ even when C_max⁻ does not improve, shifting optimisation effort entirely toward the upper-bound graph.

LEX1 exhibits the mirror image: it has the most critical operations in G⁻ (52.0, highest), the fewest in G⁺ (43.0, lowest), and the most per-operation float in G⁺ (33.6). LEX1 ignores the upper bound entirely, leaving G⁺ structurally loose — yet this produces the *widest* intervals (Table 4), not narrower ones.

**Conclusion.** The interval width reduction of LEX2 is a direct, mechanistic consequence of its ranking rule: by prioritising C_max⁺ reduction at every step, LEX2 drives the search toward configurations where the worst-case critical path is as short as possible. This tightens G⁺ — more operations are critical — but the overall C_max⁺ is smaller, yielding a narrower interval. The structural slack hypothesis (H2: "LEX2 is better because it leaves more room in G⁺") is falsified; the correct explanation (H1) is purely rank-driven: **narrower intervals emerge because LEX2 directly optimises the upper bound**.

---

## 7. Discussion

### 7.1 Summary of Findings

Our results lead to the following recommendations:

1. **Use N2 or N_ext** as the neighbourhood structure. Both achieve the same solution quality as N1 — and the same convergence speed (90% of improvement reached by generation 19–20, identical to N1) — while evaluating 64% (N2) or 43% (N_ext) fewer candidate moves per step. For N2, the saving follows directly from theory: interior block arcs cannot improve the makespan in either extreme graph, so excluding them loses nothing. For N_ext, the boundary arcs of N2 are always included, and interior arcs are only added when a fast local estimate (`isViableSwap`) suggests they may improve the makespan. The saving is purely computational: cheaper generations at no cost in convergence rate or final quality.

2. **Use YX if the objective is minimising expected makespan**, followed closely by EV. LEX1 is clearly inferior.

3. **Use LEX2 if schedule robustness (narrow intervals) is the primary concern.** The ~5–11% reduction in interval width (depending on neighbourhood) translates directly to a reduction in worst-case schedule uncertainty, at negligible cost in expected makespan (LEX2 midpoint is statistically equivalent to EV, p = 0.197). The Section 6.6 float analysis confirms this is a direct effect of the ranking mechanism, not a coincidental structural property.

4. **The ranking operator affects the number of neighbours evaluated** (LEX2 generates ~7% more), which has implications for computational cost. In practice, this is offset by LEX2's ability to find better solutions with respect to the upper bound.

### 7.2 Theoretical Connection: Why Does LEX2 Produce Narrower Intervals?

Proposition 3 guarantees that reversing a non-extreme-critical arc cannot improve either bound of the makespan interval. LEX2 specifically targets the upper bound C_max⁺, making moves that reduce C_max⁺ even if C_max⁻ does not improve. Over many local search steps, this creates a bias toward scheduling configurations where G⁺(σ) has a shorter critical path — which, by the structure of interval arithmetic, implies a tighter bound on the overall interval width.

More formally, a move that decreases C_max⁺ without affecting C_max⁻ is invisible to EV (which looks at the midpoint) but accepted by LEX2. Accumulating such moves produces solutions that are Pareto-dominated by LEX2 solutions in the (C_max⁺, width) space, even if their midpoints are similar.

The float analysis of Section 6.6 provides direct empirical confirmation: LEX2 solutions have more critical operations in G⁺ (11.4% vs 9.9–10.3% for others) and less per-operation slack (mean_float_p = 28.4 vs 30–34 for others). The G⁺ graph is structurally tighter — the algorithm has pushed the critical path of the worst-case scenario as short as it can go. Simultaneously, G⁻ has fewer critical operations (10.6% vs 11.4–11.7%), reflecting that LEX2 does not prioritise the best-case bound. The asymmetry between the two extreme graphs is a structural fingerprint of the LEX2 ranking rule.

### 7.3 On the Role of N3

N3's poor performance relative to N1 is counter-intuitive given that N3 ⊃ N1: N3 includes all single-arc swaps of N1 plus multi-arc subsequence reversals, yet it produces worse solutions. Note that N1 already includes all arcs on the critical path — both boundary and interior — so N3's additional candidates are the multi-step subsequence reversals, not the interior arcs (which N1 already covers). The heads-and-tails viability estimator `isViableSwap` is calibrated for single-arc swaps; for multi-arc reversals it becomes excessively optimistic, allowing many spurious candidates through into the evaluated pool. Combined with the first-improvement search strategy (which picks the first improving candidate at random), a larger pool diluted with poor candidates reduces the probability of selecting a genuinely improving move early, increasing wasted evaluations per step.

### 7.4 Limitations and Future Work

- **tai100 instances**: The current study covers instances up to tai50×20 (1000 operations). Results on tai100×20 instances (2000 operations) are pending and would further validate scalability at the upper end of the benchmark suite.
- **Alternative uncertainty models**: The 15% uniform interval width is a single noise level. Robustness of findings across different uncertainty levels should be examined.
- **Hybrid approaches**: The LEX2/N2 combination (narrow intervals, small neighbourhood) and LEX2/N_ext (larger neighbourhood, tight upper bound) could form the basis for a dedicated uncertainty-aware metaheuristic.

---

## 8. Conclusion

We have presented a theoretical framework for neighbourhood-based local search in the Interval Job Shop Scheduling Problem, grounded in the concept of *extreme critical paths* — arcs that are critical in either the best-case or worst-case scenario graph. We proved that the resulting neighbourhood H(σ) satisfies feasibility, connectivity, and the no-loss-of-improving-neighbours property for all admissible interval rankings simultaneously.

Four neighbourhood variants (N1, N2, N3, N_ext) and four ranking operators (EV, LEX1, LEX2, YX) were evaluated on 82 benchmark instances. The main findings are:

- **N2 and N_ext** achieve the same solution quality as N1 while being 43–64% smaller, and their per-generation convergence profiles are statistically indistinguishable from N1's (90% of improvement reached by generation 19–20 in all three). N3 produces inferior solutions and converges more slowly.
- **YX** is the best ranking for minimising expected makespan (marginally ahead of EV and LEX2, all statistically distinguishable from LEX1). **LEX2** is statistically equivalent to EV on midpoint makespan but produces solutions with **5–11% narrower makespan intervals** — a systematic effect explained by a float analysis of both extreme graphs: LEX2 directly tightens the G⁺ critical path rather than leaving more structural slack.
- The ranking operator has no measurable effect on neighbourhood improvement rates; neighbourhood size effects come entirely from the filtering strategy (N2 > N_ext > N3 > N1, in order of sparsity).

These results provide actionable guidance for practitioners implementing local search for uncertain job shop scheduling: the choice of ranking operator can be used not only to optimise makespan, but also to directly control solution uncertainty.

---

## References

*(to be completed)*

- Artigues, C., et al. (2015). *Temporal analysis of projects under interval uncertainty.* EJOR.
- Bustince, H., et al. (2013). *A class of admissible orders for interval-valued Atanassov intuitionistic fuzzy sets.* FSS.
- González-Rodríguez, I., et al. (2008). *A new neighbourhood for the fuzzy job shop.* ICAPS 2008.
- Nowicki, E., & Smutnicki, C. (1996). *A fast taboo search algorithm for the job shop problem.* Management Science.
- Nowicki, E., & Smutnicki, C. (2005). *An advanced tabu search algorithm for the job shop problem.* EJOR.
- Van Laarhoven, P., et al. (1992). *Job shop scheduling by simulated annealing.* Operations Research.
- Xu, Z.S., & Yager, R.R. (2006). *Some geometric aggregation operators based on intuitionistic fuzzy sets.* IJGIS.
