# Statistical Analysis — Exp7: HC Tuned Neighbourhoods

Generated: 2026-04-26 10:03:37.568513
Data: 12300 observations (82 instances × ≤30 runs × 5 HC configs)
Metric: midpoint = (lower_bound + upper_bound) / 2

## Methodology
- **Test**: Friedman (non-parametric repeated-measures ANOVA)
- **Blocks**: each (instance × run) pair
- **Post-hoc**: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction
- **Effect size**: r = |Z| / √N  (small ≥ 0.1, medium ≥ 0.3, large ≥ 0.5)

---

## 1. HC Neighbourhood Comparison (all instances)
**Friedman**: χ²=1113.0304, df=4, p < 0.001 ***

Mean ranks (lower = better):
- next: 2.465
- n1: 2.667
- n2: 2.739
- n8: 3.479
- n3: 3.651

| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |
|---|---|---|--------|--------|------|-------|---|-----------|-----|
| n3 | next | 2460 | 1898.8 | 1882.1 | 16.7 | 0.0000 | 0.530 | large | ✓ |
| n1 | n3 | 2460 | 1884.0 | 1898.8 | -14.7 | 0.0000 | 0.469 | medium | ✓ |
| next | n8 | 2460 | 1882.1 | 1896.9 | -14.9 | 0.0000 | 0.458 | medium | ✓ |
| n1 | n8 | 2460 | 1884.0 | 1896.9 | -12.9 | 0.0000 | 0.407 | medium | ✓ |
| n2 | n3 | 2460 | 1887.6 | 1898.8 | -11.2 | 0.0000 | 0.385 | medium | ✓ |
| n2 | n8 | 2460 | 1887.6 | 1896.9 | -9.4 | 0.0000 | 0.320 | medium | ✓ |
| n2 | next | 2460 | 1887.6 | 1882.1 | 5.5 | 0.0000 | 0.171 | small | ✓ |
| n1 | next | 2460 | 1884.0 | 1882.1 | 2.0 | 0.0000 | 0.097 | negligible | ✓ |
| n1 | n2 | 2460 | 1884.0 | 1887.6 | -3.5 | 0.0003 | 0.076 | negligible | ✓ |
| n3 | n8 | 2460 | 1898.8 | 1896.9 | 1.8 | 0.0009 | 0.067 | negligible | ✓ |

---

## 2. Results per Instance Group

| Group | Instances | χ² | p | Significant | Best |
|---|---|---|---|---|---|
| tai15_15 | 10 | 117.118 | 2.20e-24 | Yes *** | **next** |
| tai20_15 | 10 | 146.779 | 9.97e-31 | Yes *** | **next** |
| tai20_20 | 10 | 133.913 | 5.67e-28 | Yes *** | **next** |
| tai30_15 | 10 | 175.126 | 8.30e-37 | Yes *** | **next** |
| tai30_20 | 10 | 151.693 | 8.83e-32 | Yes *** | **next** |
| tai50_15 | 10 | 194.262 | 6.43e-41 | Yes *** | **n1** |
| tai50_20 | 10 | 153.701 | 3.28e-32 | Yes *** | **n1** |
