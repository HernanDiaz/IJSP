# Cover letter — Applied Soft Computing (transfer submission)

> Draft for Hernán to adapt. Plain text below; convert to letterhead as needed.

Dear Editors,

Please consider the enclosed manuscript, "Neighbourhood Structures and Ranking
Operators for the Interval Job Shop Scheduling Problem", for publication in
Applied Soft Computing. The manuscript is transferred from Computers &
Operations Research, where the handling editor assessed the results as
interesting but considered the interval-scheduling topic a better fit for a
venue closer to the soft computing community — which is, indeed, the community
this work belongs to: it extends a line of research on fuzzy and interval job
shop scheduling developed largely in this journal and its sister venues.

The paper studies the job shop scheduling problem with interval-valued
processing times (IJSP) and makes four contributions: (i) a formal notion of
*extreme criticality* (criticality in the best-case or worst-case scenario
graph) with proofs that it is computationally tractable and theoretically
sound; (ii) a local-search neighbourhood H(σ) built on extreme critical arcs,
proved feasible, connected, and lossless with respect to improving moves under
all admissible interval rankings; (iii) five neighbourhood variants and four
ranking operators evaluated through an irace-tuned tabu search on 82 benchmark
instances of up to 1,000 operations; and (iv) a structural explanation, via
float analysis, of why the LEX2 ranking yields consistently narrower makespan
intervals. The tuned solver improves the previous best published results
(fEABC, ESABC — both ABC-based soft computing methods) on every one of the 82
instances, reducing mean relative error by 56.6% on the classical instances
and 57.7% on the Taillard set.

Relative to the version examined at Computers & Operations Research, this
revision makes the operational semantics of the interval makespan explicit: a
new lemma shows the interval computed by propagation is exactly the range of
realizable makespans — its bounds are the best- and worst-case makespans — so
the LEX2 ranking corresponds to lexicographic worst-case (robust)
optimisation. A new related-work paragraph positions the approach with respect
to the min–max regret literature on interval data and explains why
regret-based criteria do not scale to job shop instances of this size.

The raw experimental data, analysis scripts and instance files are openly
available on Zenodo (DOIs in the manuscript), and per-instance results for the
70 Taillard instances are provided as supplementary material.

This manuscript is original, not under consideration elsewhere, and all data
availability and authorship statements are included. I declare no competing
interests.

Thank you for your consideration.

Sincerely,
Hernán Díaz Rodríguez
Universidad de Oviedo
