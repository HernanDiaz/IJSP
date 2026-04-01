# Statistical Analysis Results — IJSP Neighbourhood Study

Generated: 2026-04-01 11:16:18.949257
Data: 49900 observations (82 instances × ≤30 runs × 20 configs)
Metric: midpoint of makespan interval  (Cmax⁻ + Cmax⁺) / 2

## Methodology
- **Test**: Friedman (non-parametric repeated-measures ANOVA)
- **Blocks**: each (instance × run) pair — up to 2460 blocks
- **Post-hoc**: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction
- **Effect size**: r = |Z| / √N  (small ≥ 0.1, medium ≥ 0.3, large ≥ 0.5)

---

## 1. Ranking Operators
**Friedman**: χ²=694.9605, df=3, p < 0.001 ***

Mean ranks (lower = better ranked on average):
- YX: 2.149
- EV: 2.362
- LEX2: 2.422
- LEX1: 3.067

| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |
|---|---|---|--------|--------|------|-------|---|-----------|-----|
| LEX1 | YX | 2460 | 1909.5 | 1901.5 | 8.0 | 0.0000 | 0.509 | large | ✓ |
| EV | LEX1 | 2460 | 1903.3 | 1909.5 | -6.2 | 0.0000 | 0.415 | medium | ✓ |
| LEX1 | LEX2 | 2460 | 1909.5 | 1903.9 | 5.6 | 0.0000 | 0.349 | medium | ✓ |
| LEX2 | YX | 2460 | 1903.9 | 1901.5 | 2.4 | 0.0000 | 0.159 | small | ✓ |
| EV | YX | 2460 | 1903.3 | 1901.5 | 1.8 | 0.0000 | 0.131 | small | ✓ |
| EV | LEX2 | 2460 | 1903.3 | 1903.9 | -0.6 | 0.1966 | 0.026 | negligible | ✗ |

---

## 2. Neighbourhoods
**Friedman**: χ²=5970.1461, df=4, p < 0.001 ***

Mean ranks (lower = better):
- next: 2.159
- n2: 2.186
- n1: 2.196
- n3: 3.515
- nh: 4.944

| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |
|---|---|---|--------|--------|------|-------|---|-----------|-----|
| n1 | nh | 2460 | 1890.1 | 1944.8 | -54.7 | 0.0000 | Inf | large | ✓ |
| n2 | nh | 2460 | 1890.1 | 1944.8 | -54.8 | 0.0000 | Inf | large | ✓ |
| n3 | nh | 2460 | 1908.7 | 1944.8 | -36.1 | 0.0000 | Inf | large | ✓ |
| nh | next | 2460 | 1944.8 | 1889.9 | 54.9 | 0.0000 | Inf | large | ✓ |
| n3 | next | 2460 | 1908.7 | 1889.9 | 18.8 | 0.0000 | 0.729 | large | ✓ |
| n1 | n3 | 2460 | 1890.1 | 1908.7 | -18.6 | 0.0000 | 0.725 | large | ✓ |
| n2 | n3 | 2460 | 1890.1 | 1908.7 | -18.6 | 0.0000 | 0.723 | large | ✓ |
| n1 | n2 | 2460 | 1890.1 | 1890.1 | 0.1 | 1.0000 | 0.004 | negligible | ✗ |
| n1 | next | 2460 | 1890.1 | 1889.9 | 0.2 | 1.0000 | 0.018 | negligible | ✗ |
| n2 | next | 2460 | 1890.1 | 1889.9 | 0.2 | 1.0000 | 0.009 | negligible | ✗ |

---

## 3. Operator Consistency Per Neighbourhood

| Neighbourhood | χ² | p | Significant |
|---|---|---|---|
| n1 | 170.746 | 8.78e-37 | Yes *** |
| n2 | 223.671 | 3.23e-48 | Yes *** |
| n3 | 251.521 | 3.07e-54 | Yes *** |
| nh | 102.009 | 5.75e-22 | Yes *** |
| next | 130.575 | 4.07e-28 | Yes *** |
