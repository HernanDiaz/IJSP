#!/usr/bin/env Rscript
#
# run_statistical_tests.R
# Non-parametric statistical analysis for the IJSP neighbourhood study.
#
# Inputs:  statistical_results/runs_data.csv
# Outputs: statistical_results/{friedman,wilcoxon,summary}.*
#
# Design:
#   Each (instance, run) pair is a block (up to 82x30 = 2460 blocks).
#   Friedman test across k treatments (operators or neighbourhoods).
#   Post-hoc: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction.
#   Effect size: r = |Z| / sqrt(N).
#   Metric: midpoint of makespan interval.

cat("=== IJSP Statistical Analysis ===\n\n")

# ── Paths ──────────────────────────────────────────────────────────────────────
args <- commandArgs(trailingOnly = FALSE)
f    <- grep("--file=", args, value = TRUE)
script_dir <- if (length(f)) dirname(normalizePath(sub("--file=", "", f[1]))) else getwd()
out_dir   <- file.path(script_dir, "statistical_results")
data_file <- file.path(out_dir, "runs_data.csv")
if (!file.exists(data_file)) stop("runs_data.csv not found. Run build_stats_data.sh first.")

# ── Load ───────────────────────────────────────────────────────────────────────
cat("Loading data...\n")
df <- read.csv(data_file, stringsAsFactors = FALSE)
# Remove any rows with unexpected config names
valid_neigh <- c("n1","n2","n3","nh","next")
valid_comp  <- c("EV","LEX1","LEX2","YX")
df <- df[df$neigh %in% valid_neigh & df$comp %in% valid_comp, ]
df$neigh <- factor(df$neigh, levels = valid_neigh)
df$comp  <- factor(df$comp,  levels = valid_comp)
df$run   <- as.integer(df$run)
cat(sprintf("  %d rows | %d instances | %d configs\n",
            nrow(df), length(unique(df$instance)), length(unique(df$config))))

# ── Build wide matrix from long data ──────────────────────────────────────────
# Returns a data.frame with one column per treatment, rows = blocks
make_wide <- function(data, block_vars, treatment_var, value_var, treatments) {
  # Aggregate duplicates (mean) just in case
  agg <- aggregate(as.formula(paste(value_var, "~",
                   paste(c(block_vars, treatment_var), collapse="+"))),
                   data = data, FUN = mean)
  # Build list of vectors per treatment
  cols <- lapply(treatments, function(tr) {
    sub <- agg[agg[[treatment_var]] == tr, ]
    # Create a key from block_vars
    sub$key <- do.call(paste, c(sub[block_vars], sep="|"))
    sub[order(sub$key), ]
  })
  # Intersect keys present in all treatments
  keys_list <- lapply(cols, function(x) x$key)
  common_keys <- Reduce(intersect, keys_list)
  if (length(common_keys) == 0) stop("No common blocks across treatments")
  mat <- as.data.frame(lapply(cols, function(x) {
    x[x$key %in% common_keys, ][[value_var]][order(x$key[x$key %in% common_keys])]
  }))
  names(mat) <- treatments
  mat
}

# ── Helpers ───────────────────────────────────────────────────────────────────
pairwise_wilcoxon <- function(mat, treatments) {
  pairs <- combn(treatments, 2, simplify = FALSE)
  res <- do.call(rbind, lapply(pairs, function(p) {
    a <- p[1]; b <- p[2]
    d <- mat[[a]] - mat[[b]]
    ok <- !is.na(d)
    d  <- d[ok]; n <- length(d)
    wt <- wilcox.test(d, mu = 0, exact = FALSE, correct = TRUE)
    # Signed Z approximation from p-value + direction
    z  <- qnorm(wt$p.value / 2) * sign(mean(d))
    r  <- abs(z) / sqrt(n)
    data.frame(A=a, B=b, n_pairs=n,
               mean_A=round(mean(mat[[a]], na.rm=TRUE), 2),
               mean_B=round(mean(mat[[b]], na.rm=TRUE), 2),
               mean_diff=round(mean(d), 2),
               W=as.numeric(wt$statistic),
               p_raw=wt$p.value, Z=round(z,4), effect_r=round(r,4),
               stringsAsFactors=FALSE)
  }))
  res$p_adj <- p.adjust(res$p_raw, method="holm")
  res$sig   <- ifelse(res$p_adj < 0.001, "***",
               ifelse(res$p_adj < 0.01,  "**",
               ifelse(res$p_adj < 0.05,  "*", "")))
  res[order(res$p_adj), ]
}

run_friedman <- function(mat, label) {
  cat(sprintf("\n--- Friedman: %s | blocks=%d treatments=%d ---\n",
              label, nrow(mat), ncol(mat)))
  ft <- friedman.test(as.matrix(mat))
  cat(sprintf("    chi2=%.4f  df=%d  p=%.2e\n",
              ft$statistic, ft$parameter, ft$p.value))
  # Mean ranks
  rnks <- t(apply(mat, 1, rank))
  mr   <- colMeans(rnks, na.rm=TRUE)
  cat("    Mean ranks (lower=better):", paste(names(mr), round(mr,3), sep="=", collapse="  "), "\n")
  list(test=ft, mean_ranks=sort(mr))
}

effect_mag <- function(r) {
  ifelse(r>=0.5,"large", ifelse(r>=0.3,"medium", ifelse(r>=0.1,"small","negligible")))
}

# ══════════════════════════════════════════════════════════════════════════════
# ANALYSIS 1 — OPERATORS  (average midpoint across neighbourhoods per block)
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 1: Ranking Operators ══\n")

ops_mat <- make_wide(df, c("instance","run"), "comp", "midpoint", valid_comp)
cat(sprintf("Matrix: %d blocks x %d treatments\n", nrow(ops_mat), ncol(ops_mat)))

fr_ops  <- run_friedman(ops_mat, "Operators")
wt_ops  <- pairwise_wilcoxon(ops_mat, valid_comp)

cat("\nPairwise Wilcoxon (Holm corrected):\n")
print(wt_ops[, c("A","B","n_pairs","mean_A","mean_B","mean_diff","p_adj","effect_r","sig")])

# ══════════════════════════════════════════════════════════════════════════════
# ANALYSIS 2 — NEIGHBOURHOODS  (average midpoint across operators per block)
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 2: Neighbourhoods ══\n")

nb_mat <- make_wide(df, c("instance","run"), "neigh", "midpoint", valid_neigh)
cat(sprintf("Matrix: %d blocks x %d treatments\n", nrow(nb_mat), ncol(nb_mat)))

fr_nb  <- run_friedman(nb_mat, "Neighbourhoods")
wt_nb  <- pairwise_wilcoxon(nb_mat, valid_neigh)

cat("\nPairwise Wilcoxon (Holm corrected):\n")
print(wt_nb[, c("A","B","n_pairs","mean_A","mean_B","mean_diff","p_adj","effect_r","sig")])

# ══════════════════════════════════════════════════════════════════════════════
# ANALYSIS 3 — OPERATORS PER NEIGHBOURHOOD
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 3: Operators per Neighbourhood ══\n")

wt_per_nb <- do.call(rbind, lapply(valid_neigh, function(nb) {
  sub  <- df[df$neigh == nb, ]
  mat  <- make_wide(sub, c("instance","run"), "comp", "midpoint", valid_comp)
  ft   <- friedman.test(as.matrix(mat))
  cat(sprintf("  %s: Friedman chi2=%.3f p=%.2e %s | blocks=%d\n",
              nb, ft$statistic, ft$p.value,
              ifelse(ft$p.value<0.05,"[sig]","[n.s.]"), nrow(mat)))
  wt  <- pairwise_wilcoxon(mat, valid_comp)
  wt$neigh <- nb
  wt
}))

# ══════════════════════════════════════════════════════════════════════════════
# SAVE
# ══════════════════════════════════════════════════════════════════════════════
cat("\n\nSaving results...\n")

# Friedman text files
for (info in list(list(fr_ops, "Operators", "friedman_operators.txt"),
                  list(fr_nb,  "Neighbourhoods", "friedman_neighbourhoods.txt"))) {
  sink(file.path(out_dir, info[[3]]))
  cat(sprintf("=== Friedman Test: %s ===\n\n", info[[2]]))
  print(info[[1]]$test)
  cat("\nMean ranks (lower = better):\n")
  print(info[[1]]$mean_ranks)
  sink()
}

# Wilcoxon CSVs
write.csv(wt_ops,     file.path(out_dir, "wilcoxon_operators.csv"),                    row.names=FALSE)
write.csv(wt_nb,      file.path(out_dir, "wilcoxon_neighbourhoods.csv"),               row.names=FALSE)
write.csv(wt_per_nb,  file.path(out_dir, "wilcoxon_operators_per_neighbourhood.csv"),  row.names=FALSE)

# Summary markdown
p_str <- function(p) {
  if (p < 0.001) "< 0.001 ***"
  else if (p < 0.01) sprintf("= %.4f **", p)
  else if (p < 0.05) sprintf("= %.4f *",  p)
  else sprintf("= %.4f (n.s.)", p)
}

md <- c(
  "# Statistical Analysis Results — IJSP Neighbourhood Study", "",
  sprintf("Generated: %s", Sys.time()),
  sprintf("Data: %d observations (%d instances × ≤30 runs × 20 configs)",
          nrow(df), length(unique(df$instance))),
  "Metric: midpoint of makespan interval  (Cmax⁻ + Cmax⁺) / 2", "",
  "## Methodology",
  "- **Test**: Friedman (non-parametric repeated-measures ANOVA)",
  "- **Blocks**: each (instance × run) pair — up to 2460 blocks",
  "- **Post-hoc**: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction",
  "- **Effect size**: r = |Z| / √N  (small ≥ 0.1, medium ≥ 0.3, large ≥ 0.5)",
  "", "---", "",
  "## 1. Ranking Operators",
  sprintf("**Friedman**: χ²=%.4f, df=%d, p %s",
          fr_ops$test$statistic, fr_ops$test$parameter, p_str(fr_ops$test$p.value)),
  "",
  "Mean ranks (lower = better ranked on average):",
  paste(sprintf("- %s: %.3f", names(fr_ops$mean_ranks), fr_ops$mean_ranks), collapse="\n"),
  "",
  "| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |",
  "|---|---|---|--------|--------|------|-------|---|-----------|-----|"
)
for (i in seq_len(nrow(wt_ops))) {
  r <- wt_ops[i,]
  md <- c(md, sprintf("| %s | %s | %d | %.1f | %.1f | %.1f | %.4f | %.3f | %s | %s |",
    r$A, r$B, r$n_pairs, r$mean_A, r$mean_B, r$mean_diff,
    r$p_adj, r$effect_r, effect_mag(r$effect_r),
    ifelse(r$p_adj<0.05,"✓","✗")))
}

md <- c(md, "", "---", "",
  "## 2. Neighbourhoods",
  sprintf("**Friedman**: χ²=%.4f, df=%d, p %s",
          fr_nb$test$statistic, fr_nb$test$parameter, p_str(fr_nb$test$p.value)),
  "",
  "Mean ranks (lower = better):",
  paste(sprintf("- %s: %.3f", names(fr_nb$mean_ranks), fr_nb$mean_ranks), collapse="\n"),
  "",
  "| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |",
  "|---|---|---|--------|--------|------|-------|---|-----------|-----|"
)
for (i in seq_len(nrow(wt_nb))) {
  r <- wt_nb[i,]
  md <- c(md, sprintf("| %s | %s | %d | %.1f | %.1f | %.1f | %.4f | %.3f | %s | %s |",
    r$A, r$B, r$n_pairs, r$mean_A, r$mean_B, r$mean_diff,
    r$p_adj, r$effect_r, effect_mag(r$effect_r),
    ifelse(r$p_adj<0.05,"✓","✗")))
}

md <- c(md, "", "---", "",
  "## 3. Operator Consistency Per Neighbourhood",
  "",
  "| Neighbourhood | χ² | p | Significant |",
  "|---|---|---|---|"
)
for (nb in valid_neigh) {
  sub <- df[df$neigh==nb,]
  mat <- make_wide(sub, c("instance","run"), "comp", "midpoint", valid_comp)
  ft  <- friedman.test(as.matrix(mat))
  md  <- c(md, sprintf("| %s | %.3f | %.2e | %s |",
    nb, ft$statistic, ft$p.value, ifelse(ft$p.value<0.05,"Yes ***","No")))
}

writeLines(md, file.path(out_dir, "summary.md"))

cat("Files written to", out_dir, ":\n")
cat("  friedman_operators.txt\n  friedman_neighbourhoods.txt\n")
cat("  wilcoxon_operators.csv\n  wilcoxon_neighbourhoods.csv\n")
cat("  wilcoxon_operators_per_neighbourhood.csv\n  summary.md\n")
cat("\nDone.\n")
