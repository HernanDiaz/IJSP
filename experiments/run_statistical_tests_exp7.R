#!/usr/bin/env Rscript
#
# run_statistical_tests_exp7.R
# Statistical analysis for Exp7: HC tuned neighbourhoods comparison.
#
# Input:   statistical_results_exp7/runs_data.csv
# Outputs: statistical_results_exp7/{friedman,wilcoxon,summary}.*
#
# Design:
#   Compares 5 HC neighbourhood operators: n1, n2, n3, next, n8
#   Each (instance, run) pair is a block (up to 82x30 = 2460 blocks).
#   Friedman test across the 5 neighbourhoods.
#   Post-hoc: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction.
#   Effect size: r = |Z| / sqrt(N).
#   Metric: midpoint of makespan interval = (lower + upper) / 2

cat("=== IJSP Exp7 — HC Neighbourhood Statistical Analysis ===\n\n")

args       <- commandArgs(trailingOnly = FALSE)
f          <- grep("--file=", args, value = TRUE)
script_dir <- if (length(f)) dirname(normalizePath(sub("--file=", "", f[1]))) else getwd()
out_dir    <- file.path(script_dir, "statistical_results_exp7")
data_file  <- file.path(out_dir, "runs_data.csv")
if (!file.exists(data_file)) stop("runs_data.csv not found. Run build_stats_data_exp7.py first.")

# ── Load ────────────────────────────────────────────────────────────────────────
cat("Loading data...\n")
df <- read.csv(data_file, stringsAsFactors = FALSE)

valid_neigh <- c("n1", "n2", "n3", "next", "n8")
df <- df[df$neigh %in% valid_neigh, ]
df$neigh <- factor(df$neigh, levels = valid_neigh)
df$run   <- as.integer(df$run)

cat(sprintf("  %d rows | %d instances | %d configs\n",
            nrow(df), length(unique(df$instance)), length(unique(df$config))))

# ── Helpers ─────────────────────────────────────────────────────────────────────
make_wide <- function(data, block_vars, treatment_var, value_var, treatments) {
  agg  <- aggregate(as.formula(paste(value_var, "~",
                    paste(c(block_vars, treatment_var), collapse = "+"))),
                    data = data, FUN = mean)
  cols <- lapply(treatments, function(tr) {
    sub      <- agg[agg[[treatment_var]] == tr, ]
    sub$key  <- do.call(paste, c(sub[block_vars], sep = "|"))
    sub[order(sub$key), ]
  })
  common_keys <- Reduce(intersect, lapply(cols, function(x) x$key))
  if (length(common_keys) == 0) stop("No common blocks across treatments")
  mat <- as.data.frame(lapply(cols, function(x) {
    x[x$key %in% common_keys, ][[value_var]][order(x$key[x$key %in% common_keys])]
  }))
  names(mat) <- treatments
  mat
}

pairwise_wilcoxon <- function(mat, treatments) {
  pairs <- combn(treatments, 2, simplify = FALSE)
  res <- do.call(rbind, lapply(pairs, function(p) {
    a <- p[1]; b <- p[2]
    d <- mat[[a]] - mat[[b]]
    ok <- !is.na(d); d <- d[ok]; n <- length(d)
    wt <- wilcox.test(d, mu = 0, exact = FALSE, correct = TRUE)
    z  <- qnorm(wt$p.value / 2) * sign(mean(d))
    r  <- abs(z) / sqrt(n)
    data.frame(A = a, B = b, n_pairs = n,
               mean_A    = round(mean(mat[[a]], na.rm = TRUE), 2),
               mean_B    = round(mean(mat[[b]], na.rm = TRUE), 2),
               mean_diff = round(mean(d), 2),
               W         = as.numeric(wt$statistic),
               p_raw     = wt$p.value,
               Z         = round(z, 4),
               effect_r  = round(r, 4),
               stringsAsFactors = FALSE)
  }))
  res$p_adj <- p.adjust(res$p_raw, method = "holm")
  res$sig   <- ifelse(res$p_adj < 0.001, "***",
               ifelse(res$p_adj < 0.01,  "**",
               ifelse(res$p_adj < 0.05,  "*", "")))
  res[order(res$p_adj), ]
}

run_friedman <- function(mat, label) {
  cat(sprintf("\n--- Friedman: %s | blocks=%d  treatments=%d ---\n",
              label, nrow(mat), ncol(mat)))
  ft   <- friedman.test(as.matrix(mat))
  cat(sprintf("    chi2=%.4f  df=%d  p=%.2e\n",
              ft$statistic, ft$parameter, ft$p.value))
  rnks <- t(apply(mat, 1, rank))
  mr   <- colMeans(rnks, na.rm = TRUE)
  cat("    Mean ranks (lower=better):", paste(names(mr), round(mr, 3), sep = "=", collapse = "  "), "\n")
  list(test = ft, mean_ranks = sort(mr))
}

effect_mag <- function(r)
  ifelse(r >= 0.5, "large", ifelse(r >= 0.3, "medium", ifelse(r >= 0.1, "small", "negligible")))

p_str <- function(p) {
  if (p < 0.001)      "< 0.001 ***"
  else if (p < 0.01)  sprintf("= %.4f **",     p)
  else if (p < 0.05)  sprintf("= %.4f *",      p)
  else                sprintf("= %.4f (n.s.)", p)
}

# ══════════════════════════════════════════════════════════════════════════════
# ANALYSIS 1 — HC NEIGHBOURHOODS  (all instances × runs)
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 1: HC Neighbourhoods (all instances) ══\n")

nb_mat <- make_wide(df, c("instance", "run"), "neigh", "midpoint", valid_neigh)
cat(sprintf("Matrix: %d blocks x %d treatments\n", nrow(nb_mat), ncol(nb_mat)))

fr_nb <- run_friedman(nb_mat, "HC Neighbourhoods")
wt_nb <- pairwise_wilcoxon(nb_mat, valid_neigh)

cat("\nPairwise Wilcoxon (Holm corrected):\n")
print(wt_nb[, c("A","B","n_pairs","mean_A","mean_B","mean_diff","p_adj","effect_r","sig")])

# ══════════════════════════════════════════════════════════════════════════════
# ANALYSIS 2 — PER INSTANCE GROUP
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 2: Neighbourhoods per Instance Group ══\n")

# Derive group from instance name: prefix before first numeric part
df$group <- sub("^(F[0-9.]+\\.[^_]+|[a-z]+[0-9]+_[0-9]+).*", "\\1", df$instance)

groups <- sort(unique(df$group))
cat(sprintf("Groups found: %s\n", paste(groups, collapse=", ")))

wt_per_group <- do.call(rbind, lapply(groups, function(g) {
  sub <- df[df$group == g, ]
  if (length(unique(sub$instance)) < 2) return(NULL)
  mat <- tryCatch(make_wide(sub, c("instance","run"), "neigh", "midpoint", valid_neigh),
                  error = function(e) NULL)
  if (is.null(mat) || nrow(mat) < 5) return(NULL)
  ft  <- friedman.test(as.matrix(mat))
  rnks <- t(apply(mat, 1, rank))
  mr   <- colMeans(rnks, na.rm = TRUE)
  best <- names(which.min(mr))
  cat(sprintf("  %-30s  chi2=%6.2f  p=%.2e %s  best=%s\n",
              g, ft$statistic, ft$p.value,
              ifelse(ft$p.value < 0.05, "[sig]", "[n.s.]"), best))
  data.frame(group = g, n_instances = length(unique(sub$instance)),
             chi2 = round(ft$statistic, 3), p = ft$p.value,
             sig  = ft$p.value < 0.05, best_neigh = best,
             stringsAsFactors = FALSE)
}))

# ══════════════════════════════════════════════════════════════════════════════
# SAVE
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\nSaving results...\n")

sink(file.path(out_dir, "friedman_neighbourhoods.txt"))
cat("=== Friedman Test: HC Neighbourhoods ===\n\n")
print(fr_nb$test)
cat("\nMean ranks (lower = better):\n")
print(fr_nb$mean_ranks)
sink()

write.csv(wt_nb,        file.path(out_dir, "wilcoxon_neighbourhoods.csv"),     row.names = FALSE)
write.csv(wt_per_group, file.path(out_dir, "wilcoxon_neighbourhoods_by_group.csv"), row.names = FALSE)

# ── Summary markdown ──────────────────────────────────────────────────────────
md <- c(
  "# Statistical Analysis — Exp7: HC Tuned Neighbourhoods", "",
  sprintf("Generated: %s", Sys.time()),
  sprintf("Data: %d observations (%d instances × ≤30 runs × 5 HC configs)",
          nrow(df), length(unique(df$instance))),
  "Metric: midpoint = (lower_bound + upper_bound) / 2", "",
  "## Methodology",
  "- **Test**: Friedman (non-parametric repeated-measures ANOVA)",
  "- **Blocks**: each (instance × run) pair",
  "- **Post-hoc**: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction",
  "- **Effect size**: r = |Z| / √N  (small ≥ 0.1, medium ≥ 0.3, large ≥ 0.5)",
  "", "---", "",
  "## 1. HC Neighbourhood Comparison (all instances)",
  sprintf("**Friedman**: χ²=%.4f, df=%d, p %s",
          fr_nb$test$statistic, fr_nb$test$parameter, p_str(fr_nb$test$p.value)),
  "",
  "Mean ranks (lower = better):",
  paste(sprintf("- %s: %.3f", names(fr_nb$mean_ranks), fr_nb$mean_ranks), collapse = "\n"),
  "",
  "| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |",
  "|---|---|---|--------|--------|------|-------|---|-----------|-----|"
)
for (i in seq_len(nrow(wt_nb))) {
  r  <- wt_nb[i, ]
  md <- c(md, sprintf("| %s | %s | %d | %.1f | %.1f | %.1f | %.4f | %.3f | %s | %s |",
    r$A, r$B, r$n_pairs, r$mean_A, r$mean_B, r$mean_diff,
    r$p_adj, r$effect_r, effect_mag(r$effect_r),
    ifelse(r$p_adj < 0.05, "✓", "✗")))
}

md <- c(md, "", "---", "",
  "## 2. Results per Instance Group",
  "",
  "| Group | Instances | χ² | p | Significant | Best |",
  "|---|---|---|---|---|---|"
)
if (!is.null(wt_per_group) && nrow(wt_per_group) > 0) {
  for (i in seq_len(nrow(wt_per_group))) {
    r  <- wt_per_group[i, ]
    md <- c(md, sprintf("| %s | %d | %.3f | %.2e | %s | **%s** |",
      r$group, r$n_instances, r$chi2, r$p,
      ifelse(r$sig, "Yes ***", "No"), r$best_neigh))
  }
}

writeLines(md, file.path(out_dir, "summary.md"))

cat("Files written to", out_dir, ":\n")
cat("  friedman_neighbourhoods.txt\n")
cat("  wilcoxon_neighbourhoods.csv\n")
cat("  wilcoxon_neighbourhoods_by_group.csv\n")
cat("  summary.md\n")
cat("\nDone.\n")
