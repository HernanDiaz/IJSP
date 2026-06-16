# ASOC transfer copy — changes relative to the frozen COR submission

Working copy created 2026-06-11 from `Papers/COR_Tabu/` (tag
`cor-tabu-2026-submission`). The frozen COR material is untouched.

## Applied (responding to the COR desk assessment)

1. **Lemma 1 "Exact range"** (end of §2.2 Interval Arithmetic and Ranking
   Operators): for a fixed processing order, the interval makespan computed by
   propagation equals exactly the set of realizable makespans (monotonicity +
   continuity over the scenario box). Direct answer to "I disagree that the
   makespan is an interval": it is not a postulate, it is the exact range.
2. **Robust reading paragraph** after the lemma: C⁺ = worst-case makespan,
   C⁻ = best case, LEX2 = lexicographic min–max (worst-case-first)
   optimisation. Answers "why are comparisons limited to interval ranking
   operators" — the rankings *contain* the robust criterion.
3. **Related work, new paragraph "Interval uncertainty in operations
   research"**: positions the paper against the min–max (regret) interval
   literature (Kouvelis & Yu 1997; Averbakh 2001; Kasperski 2008; Kasperski &
   Zieliński 2016) and gives the complexity argument: evaluating max regret of
   a single JSP schedule embeds NP-hard deterministic subproblems, so
   regret-based search cannot scale to 1,000-operation instances; the
   interval-ranking approach is the scalable counterpart.
4. **Abstract**: one sentence making the robust equivalence explicit.
5. **Keywords**: added "robust scheduling".
6. `\journal{}` changed to Applied Soft Computing; added
   `\newtheorem{lemma}` to the preamble; four BibTeX entries appended.

Compiles clean (latexmk, 0 errors, all citations resolve). Figure PDFs copied
from the original working repo (`CLionProjects/IJSP/figures/` and
`.../Papers/COR_Tabu/figures/`) — they are generated artifacts, kept out of
git.

## Suggested but NOT applied (Hernán's call)

- **Title**: consider signalling robustness, e.g. "... for the Robust
  (Interval) Job Shop Scheduling Problem" or keeping the current title.
- The COR cover letter highlights/zenodo README still mention COR in a couple
  of places if reused — check before submission.
- ASOC uses a soft-computing audience: the introduction could lean slightly
  more on the fuzzy-scheduling lineage (one sentence would do).
