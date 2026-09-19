# Draft research plan: Quantum approaches to the Interval Job Shop Scheduling Problem

**Status:** initial outline (drafted while waiting for Phase B bug-fix re-run to finish)
**Owner:** Hernán Díaz Rodríguez
**Last updated:** 3 June 2026

---

## 1. Working title

> *"Extreme Critical Paths Meet Quantum Computing: a QUBO formulation and a
> Grover-style neighbourhood for the Interval Job Shop Scheduling Problem"*

(Alternative shorter title: *"Towards Quantum-Assisted Local Search for the
Interval JSP."*)

---

## 2. Elevator pitch (one paragraph)

The Interval Job Shop Scheduling Problem (IJSP) extends classical JSP with
processing-time uncertainty represented as closed intervals. We have a fully
classical theoretical and experimental framework for IJSP (the COR_Tabu paper:
extreme critical paths, the neighbourhood $H(\sigma)$, irace-tuned tabu search
with the LEX2 ranking operator). The proposed quantum paper builds *on top of*
that framework by (i) giving the first QUBO/Ising formulation of IJSP that
respects the interval ranking operators and (ii) proving that $H(\sigma)$
admits an efficient oracle for amplitude amplification, yielding an
$O(\sqrt{|N(\sigma)|})$ search at the inner loop of tabu search. We validate
both contributions with a small ($n{\le}4$, $m{\le}3$) instance on real
IBM Quantum hardware using the platform's 10 free minutes, and complement with
extensive Qiskit-simulator runs.

The honest framing: this is *not* a "quantum advantage" paper; it is a
**formulation** + **structural** + **proof-of-concept** paper. That framing
is realistic for current NISQ hardware and is increasingly welcome in OR
venues.

---

## 3. Why this is a publishable angle

| Component | Novelty | Risk | Mitigation |
|---|---|---|---|
| QUBO for IJSP with interval ranking | High — no existing formulation for the interval extension | Medium — encoding is non-trivial for LEX2 | Start with EV ranking (averaging) which is straightforward; LEX2 as the headline novelty |
| Grover oracle for $H(\sigma)$ | High — exploits the structural result already proven in COR_Tabu | Low — the classical $H(\sigma)$ membership test is $O(n)$, so the oracle is cheap | — |
| 10-minute IBM Quantum demo | Required for credibility but not as quality benchmark | High — current hardware can't beat your TS classically | Treat as illustration, not benchmark; full benchmark on simulator |
| Comparison vs your TS-$N_2$ | Forced by COR_Tabu reviewers | Medium — quantum will lose on quality | Be explicit: this is about *scaling expectations*, not current performance |

The combined paper is publishable in a workshop or quantum-applied journal even
if the experimental quantum results are modest, because the contribution is
the **formulation** plus the **structural theorem**.

---

## 4. Paper structure (rough outline)

### §1 Introduction

- IJSP in two sentences (interval processing times, interval ranking, makespan).
- Why quantum: large search space, structured constraints, increasing interest
  in quantum optimization (cite Tian et al., Venturelli, Yarkoni et al.).
- Gap: no QUBO formulation exists for the *interval* extension, and no work
  exploits the rich neighbourhood structure of $H(\sigma)$ via quantum
  amplitude amplification.
- Contributions (numbered, 3–4 bullets).

### §2 Background

- §2.1 Classical IJSP. Cite COR_Tabu for extreme critical paths, $H(\sigma)$
  feasibility/connectivity/no-loss properties.
- §2.2 Quantum optimisation primer: QUBO/Ising, QAOA, amplitude
  amplification / Grover. Self-contained but compressed (1.5 pages).

### §3 QUBO formulation for IJSP

- §3.1 Binary variables $x_{j,o,t} \in \{0,1\}$ — operation $(j,o)$ starts at
  time slot $t$.
- §3.2 Hard constraints as penalties:
    - Each operation starts exactly once: $\sum_t x_{j,o,t} = 1$.
    - Job precedence: $\sum_t t \cdot x_{j,o+1,t} \ge \sum_t t \cdot x_{j,o,t} + p_{j,o}$.
    - Machine capacity: $\sum_{(j',o') \text{ on machine } m} x_{j',o',t} \le 1$ for all $t$, $m$.
- §3.3 **Novel:** interval objective. For each ranking operator one
  formulation:
    - **EV:** minimise $\frac{1}{2}(C^-_{\max} + C^+_{\max})$ — linear combination.
    - **LEX2:** lexicographic priority on $C^+_{\max}$ — use a large weight on $C^+_{\max}$ plus a smaller weight on $C^-_{\max}$ to encode the lexicographic order in QUBO.
    - **LEX1, YX:** parallel definitions.
- §3.4 Qubit-count analysis: $|x| = n \cdot m \cdot T_{\max}$. For 3 jobs ×
  3 machines × 30 time slots → 270 qubits — too many for current hardware.
  Discuss the $T_{\max}$-tightening trick (use a lower bound from
  $H(\sigma_0)$ to shrink the horizon).

### §4 Grover-style search in $H(\sigma)$

- §4.1 Restate the no-loss property from COR_Tabu: every improving move lies
  in $H(\sigma)$.
- §4.2 Build the quantum oracle $U_f$: $U_f|v\rangle|0\rangle = |v\rangle|\mathrm{isExtremeCritical}(v)\rangle$.
  Classical $\mathrm{isExtremeCritical}$ is $O(n)$ (one head/tail computation
  on $G^-$ and $G^+$), so the quantum oracle has the same gate complexity.
- §4.3 Amplitude amplification gives $O(\sqrt{|D(\sigma)|})$ samples vs.
  classical $O(|D(\sigma)|)$.
- §4.4 *Caveat:* the speedup is in the **query count**, not necessarily in
  wall-clock — the oracle has to be implemented as a quantum circuit, which
  on NISQ devices is the bottleneck.

### §5 Experimental setup

- §5.1 Instance: a small 3×3 IJSP instance derived from a classical Taillard
  benchmark with the same $\pm 7.5\%$ perturbation used in COR_Tabu. Encoded
  with $T_{\max} = $ tight lower bound, total $\le 30$ qubits.
- §5.2 Real hardware: IBM Quantum Platform (free tier) — 10 minutes of access
  to one of their public-access backends (e.g., `ibm_brisbane`, 127 qubits,
  superconducting). Used for the *one* headline experiment, not for
  full statistics.
- §5.3 Simulator: Qiskit's `AerSimulator` for all systematic studies (number
  of QAOA layers, mixer choice, classical baseline). Free, unlimited.
- §5.4 Classical baseline: TS-$N_2$ from COR_Tabu, same instance.
- §5.5 Metrics: time-to-optimum, solution quality (RE\,(\%) over LB), number
  of quantum circuit evaluations.

### §6 Results

- §6.1 QAOA on the simulator: how the objective lands as a function of
  $p$ (QAOA layers).
- §6.2 Real-vs-simulator agreement: noise model verification.
- §6.3 Headline experiment on IBM hardware: solution quality of the best
  QAOA configuration on the 3×3 instance.
- §6.4 Honest comparison vs classical TS: TS likely wins. Frame as a
  reference benchmark, not a competition. Identify the instance size at
  which simulators predict crossover.

### §7 Discussion

- §7.1 What we proved structurally (QUBO formulation for any interval
  ranking; oracle complexity for $H(\sigma)$).
- §7.2 What we observed empirically (NISQ is not yet competitive; encoding
  works as advertised).
- §7.3 Implications for the IJSP community.
- §7.4 Implications for the quantum optimisation community (the interval
  ranking obstacle and how to handle it).

### §8 Limitations & future work

- Scaling: tighter $T_{\max}$, smarter encodings (e.g., bucketed time,
  unary vs binary).
- Hybrid: warm-start QAOA from the classical TS solution.
- Quantum annealing alternative on D-Wave.
- More ranking operators / different uncertainty representations.

### §9 Conclusions

(Short — restate the two contributions and the most defensible empirical
takeaway.)

---

## 5. Experimental plan (detailed)

### Hardware budget

| Resource | Quota | Use |
|---|---|---|
| IBM Quantum free tier | 10 min total | One headline experiment, ~50 shots × 10 QAOA configs |
| Qiskit AerSimulator | unlimited | All systematic sweeps |
| Local CPU (your workstation) | unlimited | Classical TS baseline |

### Instance choice

- Start from a Taillard 3×3 nominal instance (smallest publicly available)
  or down-sample a $15{\times}15$ instance to $3{\times}3$ keeping the
  structure. Same $\pm 7.5\%$ perturbation as COR_Tabu so the two papers'
  instances are directly comparable.
- Resource estimate: $n\,m\,T_{\max}$ qubits. With $T_{\max} = 25$, the
  3×3 instance fits in $\le 75$ qubits with tight bounds. IBM's 127-qubit
  machines can run this, though circuit depth will hit noise limits.

### Software stack

- Qiskit ≥ 1.0 (current stable)
- `qiskit_optimization` (for QUBO ↔ Ising conversion)
- `qiskit_ibm_runtime` (for IBM cloud access)
- Python 3.11+

### Milestones

| Step | Duration | Deliverable |
|---|---|---|
| 1. Encode 3×3 IJSP as QUBO in Python | 1 week | Self-contained notebook |
| 2. Run QAOA on simulator with EV ranking | 1 week | Baseline plot: energy vs $p$ |
| 3. Add LEX2 encoding | 1 week | Encoding diff + ablation |
| 4. Implement Grover-style oracle on $H(\sigma)$ | 2 weeks | Theoretical lemma + quantum circuit |
| 5. Reserve IBM Quantum slot, run headline experiment | 2 days | Real-hardware data |
| 6. Classical TS baseline on same instance | 2 days | Reuse COR_Tabu code |
| 7. Write paper | 3 weeks | First draft |
| 8. Iterate / submit | 2 weeks | Submission |

Total realistic timeline: **~3 months** from first encoding to submission.

---

## 6. Target venues (in preference order)

1. **EPJ Quantum Technology** — applied quantum, OR-friendly editorial board.
2. **Quantum Information Processing** (Springer) — broader scope.
3. **IEEE Transactions on Quantum Engineering** (TQE) — newer venue, very
   open to NISQ-era applied work.
4. **IEEE Quantum Week (QCE) proceedings** — workshop track, fast.
5. **Optimization Letters** (Springer) — short paper, OR audience, would
   accept the *structural* contributions even with a thin experimental
   section.

Workshops to watch for special issues:

- *QCE 2026/2027 Workshop on Quantum-Inspired Algorithms*
- *ICAPS 2026* (sometimes has a "quantum and planning" track)
- INFORMS subdivision on Optimization Under Uncertainty has occasional
  quantum sessions.

---

## 7. Related-work checklist

To survey before writing:

- **JSP + quantum:**
    - Tian et al. — early QAOA for JSP
    - Venturelli et al. (NASA Ames) — D-Wave annealing for JSP
    - Carugno et al. (IBM Q + AIMMS) — hybrid solvers
    - Kurowski et al. — JSP QUBO formulations
- **Interval / robust scheduling under quantum:** likely thin literature —
  this is the gap.
- **Grover for combinatorial optimisation structure:** Baritompa et al.
  ("Grover's search applied to global optimisation"), Bulger et al.
- **Amplitude amplification for arbitrary search problems:** Brassard, Høyer,
  Mosca, Tapp.

---

## 8. Risks & mitigations

| Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|
| LEX2 encoding turns out to be NP-hard to express in QUBO | Medium | High — it's the headline contribution | Fall back to EV + a "future work" note on LEX2 |
| IBM Quantum free tier insufficient for any usable shot | Low | Medium | Use simulator for everything; treat hardware as "qualitative demonstration" |
| Reviewer demands quantum-advantage benchmark | Medium | Medium | Frame paper as "formulation + structural lemma" so the contribution is theoretical, not empirical |
| Existing prior art covers the same QUBO formulation | Medium | High | Aggressive literature search in Step 0; the interval angle is likely unique even if the JSP-QUBO basics are known |

---

## 9. Next concrete actions

1. **Account + access:**
    - Sign up for IBM Quantum Platform (free tier).
    - Install Qiskit locally (`pip install qiskit qiskit-optimization qiskit-ibm-runtime`).
2. **Mini-experiment to validate the path:**
    - Encode a *deterministic* 3×3 JSP as QUBO using existing
      `qiskit_optimization.applications.job_shop`.
    - Run on simulator with QAOA, verify it converges to a known optimum.
3. **Extend to interval:**
    - Replace deterministic durations with intervals.
    - Add the EV ranking objective.
    - Re-run on simulator.
4. **Literature scan** (1 day): confirm the gap, identify the 5–8 papers to
   cite as related work.
5. **Decision point:** after steps 1–4, evaluate whether the formulation
   really extends cleanly. If yes, commit to the full plan. If LEX2 turns
   out to be intractable, pivot to "QUBO for IJSP with EV ranking only,
   Grover speedup for $H(\sigma)$" — still a publishable contribution.

---

## 10. Open questions for me to think about

- Does the interval Cmax ($C^+_{\max}$, $C^-_{\max}$) admit a natural QUBO
  expression at all, given that it depends on a critical path that itself
  depends on the order? Probably needs auxiliary variables tracking job
  completion times explicitly.
- The Grover-style result needs a *uniform* oracle — does
  $\mathrm{isExtremeCritical}$ generalise across iterations of TS, or does
  it have to be rebuilt every iteration (which would kill any speedup)?
- For LEX2, can we avoid the giant penalty trick by using two QAOA layers
  in sequence (one for $C^+_{\max}$, one for $C^-_{\max}$)? Worth a short
  note.

---

*End of draft.*
