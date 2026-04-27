#!/usr/bin/env Rscript
#
# run_statistical_tests_exp8.R
# Statistical analysis for Exp8: irace-tuned HC neighbourhoods on CRISP instances.
#
# Input:   statistical_results_exp8/runs_data.csv
# Outputs: statistical_results_exp8/{friedman, wilcoxon, rpd, summary}.*
#
# Design:
#   Compares 5 irace-tuned HC configs: n1, n2, n3, next, n8
#   Blocks: each (instance × run) pair  →  N = 82 × 30 = 2460
#   Metric: makespan (crisp integer — lower is better)
#   Test 1: Friedman + pairwise Wilcoxon (Holm) on raw makespan
#   Analysis 2: RPD = (best_found - BKS) / BKS × 100 per (config × instance)
#   BKS source: optimizizer.com (http://optimizizer.com/TA.php) + OR-Library
#
# BKS values (best known solutions / proven optima for JSSP benchmarks):
#   Classical: Fisher & Thompson 1963, Adams et al. 1988, Lawrence 1984
#   Taillard:  Taillard 1993 (updated by subsequent literature)

cat("=== IJSP Exp8 — irace-tuned HC on CRISP instances ===\n\n")

args       <- commandArgs(trailingOnly = FALSE)
f          <- grep("--file=", args, value = TRUE)
script_dir <- if (length(f)) dirname(normalizePath(sub("--file=", "", f[1]))) else getwd()
out_dir    <- file.path(script_dir, "statistical_results_exp8")
data_file  <- file.path(out_dir, "runs_data.csv")
if (!file.exists(data_file)) stop("runs_data.csv not found. Run build_stats_data_exp8.py first.")

# ── Load data ────────────────────────────────────────────────────────────────────
cat("Loading data...\n")
df <- read.csv(data_file, stringsAsFactors = FALSE)

valid_neigh <- c("n1", "n2", "n3", "next", "n8")
df <- df[df$neigh %in% valid_neigh, ]
df$neigh <- factor(df$neigh, levels = valid_neigh)
df$run   <- as.integer(df$run)

cat(sprintf("  %d rows | %d instances | %d configs\n",
            nrow(df), length(unique(df$instance)), length(unique(df$config))))

# ── BKS table ────────────────────────────────────────────────────────────────────
# Keys match the instance stem as it appears in the result file names.
# Source: optimizizer.com/TA.php (accessed April 2026) + OR-Library
bks_data <- c(
  # ── Classical instances (proven optimal) ──────────────────────────────────────
  "F0.15.0.ft10_10"   = 930,   # Fisher & Thompson 10×10
  "F0.15.0.ft20_05"   = 1165,  # Fisher & Thompson 20×5
  "F0.15.0.abz7_06"   = 656,   # Adams, Balas & Zawack 20×15
  "F0.15.0.abz8_05"   = 665,
  "F0.15.0.abz9_10"   = 679,
  "F0.15.0.la21_04"   = 1046,  # Lawrence 15×10
  "F0.15.0.la24_03"   = 935,
  "F0.15.0.la25_04"   = 977,
  "F0.15.0.la27_09"   = 1235,  # Lawrence 20×10
  "F0.15.0.la29_03"   = 1152,
  "F0.15.0.la38_06"   = 1196,  # Lawrence 15×15
  "F0.15.0.la40_05"   = 1222,
  # ── Taillard 15×15 (ta01–ta10) ───────────────────────────────────────────────
  "tai15_15_01.F.15_01" = 1231,
  "tai15_15_02.F.15_01" = 1244,
  "tai15_15_03.F.15_01" = 1218,
  "tai15_15_04.F.15_01" = 1175,
  "tai15_15_05.F.15_01" = 1224,
  "tai15_15_06.F.15_01" = 1238,
  "tai15_15_07.F.15_01" = 1227,
  "tai15_15_08.F.15_01" = 1217,
  "tai15_15_09.F.15_01" = 1274,
  "tai15_15_10.F.15_01" = 1241,
  # ── Taillard 20×15 (ta11–ta20) ───────────────────────────────────────────────
  "tai20_15_01.F.15_01" = 1357,
  "tai20_15_02.F.15_01" = 1367,
  "tai20_15_03.F.15_01" = 1342,
  "tai20_15_04.F.15_01" = 1345,
  "tai20_15_05.F.15_01" = 1339,
  "tai20_15_06.F.15_01" = 1360,
  "tai20_15_07.F.15_01" = 1462,
  "tai20_15_08.F.15_01" = 1396,
  "tai20_15_09.F.15_01" = 1332,
  "tai20_15_10.F.15_01" = 1348,
  # ── Taillard 20×20 (ta21–ta30) ───────────────────────────────────────────────
  "tai20_20_01.F.15_01" = 1642,
  "tai20_20_02.F.15_01" = 1600,
  "tai20_20_03.F.15_01" = 1557,
  "tai20_20_04.F.15_01" = 1644,
  "tai20_20_05.F.15_01" = 1595,
  "tai20_20_06.F.15_01" = 1645,
  "tai20_20_07.F.15_01" = 1680,
  "tai20_20_08.F.15_01" = 1603,
  "tai20_20_09.F.15_01" = 1625,
  "tai20_20_10.F.15_01" = 1584,
  # ── Taillard 30×15 (ta31–ta40) ───────────────────────────────────────────────
  "tai30_15_01.F.15_01" = 1764,
  "tai30_15_02.F.15_01" = 1784,
  "tai30_15_03.F.15_01" = 1791,
  "tai30_15_04.F.15_01" = 1828,
  "tai30_15_05.F.15_01" = 2007,
  "tai30_15_06.F.15_01" = 1819,
  "tai30_15_07.F.15_01" = 1771,
  "tai30_15_08.F.15_01" = 1673,
  "tai30_15_09.F.15_01" = 1795,
  "tai30_15_10.F.15_01" = 1670,
  # ── Taillard 30×20 (ta41–ta50) ───────────────────────────────────────────────
  "tai30_20_01.F.15_01" = 2006,
  "tai30_20_02.F.15_01" = 1939,
  "tai30_20_03.F.15_01" = 1846,
  "tai30_20_04.F.15_01" = 1979,
  "tai30_20_05.F.15_01" = 2000,
  "tai30_20_06.F.15_01" = 2006,
  "tai30_20_07.F.15_01" = 1889,
  "tai30_20_08.F.15_01" = 1937,
  "tai30_20_09.F.15_01" = 1960,
  "tai30_20_10.F.15_01" = 1923,
  # ── Taillard 50×15 (ta51–ta60) ───────────────────────────────────────────────
  "tai50_15_01.F.15_01" = 2760,
  "tai50_15_02.F.15_01" = 2756,
  "tai50_15_03.F.15_01" = 2717,
  "tai50_15_04.F.15_01" = 2839,
  "tai50_15_05.F.15_01" = 2679,
  "tai50_15_06.F.15_01" = 2781,
  "tai50_15_07.F.15_01" = 2943,
  "tai50_15_08.F.15_01" = 2885,
  "tai50_15_09.F.15_01" = 2655,
  "tai50_15_10.F.15_01" = 2723,
  # ── Taillard 50×20 (ta61–ta70) ───────────────────────────────────────────────
  "tai50_20_01.F.15_01" = 2868,
  "tai50_20_02.F.15_01" = 2869,
  "tai50_20_03.F.15_01" = 2755,
  "tai50_20_04.F.15_01" = 2702,
  "tai50_20_05.F.15_01" = 2725,
  "tai50_20_06.F.15_01" = 2845,
  "tai50_20_07.F.15_01" = 2825,
  "tai50_20_08.F.15_01" = 2784,
  "tai50_20_09.F.15_01" = 3071,
  "tai50_20_10.F.15_01" = 2995
)

bks_df <- data.frame(
  instance = names(bks_data),
  bks      = as.integer(bks_data),
  stringsAsFactors = FALSE
)

# Check coverage
instances_in_data <- unique(df$instance)
missing_bks <- setdiff(instances_in_data, bks_df$instance)
if (length(missing_bks) > 0) {
  cat(sprintf("WARNING: no BKS for %d instances: %s\n",
              length(missing_bks), paste(missing_bks, collapse=", ")))
}

# ── Helpers ──────────────────────────────────────────────────────────────────────
p_str <- function(p) {
  if (p < 0.001)     "< 0.001 ***"
  else if (p < 0.01) sprintf("= %.4f **",     p)
  else if (p < 0.05) sprintf("= %.4f *",      p)
  else               sprintf("= %.4f (n.s.)", p)
}

effect_mag <- function(r)
  ifelse(r >= 0.5, "large", ifelse(r >= 0.3, "medium", ifelse(r >= 0.1, "small", "negligible")))

make_wide <- function(data, block_vars, treatment_var, value_var, treatments) {
  agg  <- aggregate(as.formula(paste(value_var, "~",
                    paste(c(block_vars, treatment_var), collapse = "+"))),
                    data = data, FUN = mean)
  cols <- lapply(treatments, function(tr) {
    sub     <- agg[agg[[treatment_var]] == tr, ]
    sub$key <- do.call(paste, c(sub[block_vars], sep = "|"))
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
  cat("    Mean ranks (lower=better):", paste(names(mr), round(mr, 3), sep="=", collapse="  "), "\n")
  list(test = ft, mean_ranks = sort(mr))
}

# ══════════════════════════════════════════════════════════════════════════════════
# ANALYSIS 1 — WILCOXON COMPARISON OF HC CONFIGS (raw makespan)
# ══════════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 1: HC Config Comparison (all 82 instances, raw makespan) ══\n")

nb_mat <- make_wide(df, c("instance", "run"), "neigh", "makespan", valid_neigh)
cat(sprintf("Matrix: %d blocks x %d treatments\n", nrow(nb_mat), ncol(nb_mat)))

fr_nb <- run_friedman(nb_mat, "HC Configs — makespan")
wt_nb <- pairwise_wilcoxon(nb_mat, valid_neigh)

cat("\nPairwise Wilcoxon (Holm corrected):\n")
print(wt_nb[, c("A","B","n_pairs","mean_A","mean_B","mean_diff","p_adj","effect_r","sig")])

# ══════════════════════════════════════════════════════════════════════════════════
# ANALYSIS 2 — RPD (Relative Percentage Deviation from BKS)
# ══════════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 2: RPD from Best Known Solution ══\n")
cat("RPD = (best_of_30_runs - BKS) / BKS × 100\n\n")

# Compute best makespan over 30 runs per (config, instance)
best_per_ci <- aggregate(makespan ~ config + neigh + instance, data = df, FUN = min)
names(best_per_ci)[names(best_per_ci) == "makespan"] <- "best_found"

# Merge BKS
best_per_ci <- merge(best_per_ci, bks_df, by = "instance", all.x = TRUE)
best_per_ci$rpd <- (best_per_ci$best_found - best_per_ci$bks) / best_per_ci$bks * 100

# Instance group label
best_per_ci$group <- ifelse(
  grepl("^F0\\.15\\.0\\.", best_per_ci$instance),
  sub("^F0\\.15\\.0\\.([a-z]+[0-9]+).*", "\\1", best_per_ci$instance),
  sub("^(tai[0-9]+_[0-9]+).*", "\\1", best_per_ci$instance)
)

# ── Summary by config ─────────────────────────────────────────────────────────
rpd_by_config <- do.call(rbind, lapply(valid_neigh, function(n) {
  sub <- best_per_ci[best_per_ci$neigh == n & !is.na(best_per_ci$rpd), ]
  data.frame(
    neigh         = n,
    config        = unique(sub$config),
    n_instances   = nrow(sub),
    rpd_mean      = round(mean(sub$rpd), 3),
    rpd_median    = round(median(sub$rpd), 3),
    rpd_sd        = round(sd(sub$rpd), 3),
    rpd_max       = round(max(sub$rpd), 3),
    n_optimal     = sum(sub$rpd <= 0.001),   # found BKS (RPD ≈ 0)
    pct_optimal   = round(mean(sub$rpd <= 0.001) * 100, 1),
    stringsAsFactors = FALSE
  )
}))

cat("RPD summary per config:\n")
print(rpd_by_config[, c("neigh","rpd_mean","rpd_median","rpd_sd","rpd_max",
                          "n_optimal","pct_optimal")])

# ── Summary by instance group and config ──────────────────────────────────────
groups <- sort(unique(best_per_ci$group))
rpd_by_group <- do.call(rbind, lapply(groups, function(g) {
  sub_g <- best_per_ci[best_per_ci$group == g & !is.na(best_per_ci$rpd), ]
  do.call(rbind, lapply(valid_neigh, function(n) {
    sub <- sub_g[sub_g$neigh == n, ]
    if (nrow(sub) == 0) return(NULL)
    data.frame(
      group       = g,
      neigh       = n,
      n_inst      = nrow(sub),
      rpd_mean    = round(mean(sub$rpd), 3),
      rpd_median  = round(median(sub$rpd), 3),
      rpd_max     = round(max(sub$rpd), 3),
      n_optimal   = sum(sub$rpd <= 0.001),
      stringsAsFactors = FALSE
    )
  }))
}))

cat("\nRPD by instance group:\n")
print(rpd_by_group)

# ── Instance-level table: best config per instance ────────────────────────────
inst_wide <- reshape(
  best_per_ci[, c("instance","group","neigh","rpd")],
  idvar    = c("instance","group"),
  timevar  = "neigh",
  direction = "wide"
)
names(inst_wide) <- gsub("^rpd\\.", "RPD_", names(inst_wide))
# Add BKS column
inst_wide <- merge(inst_wide, bks_df, by = "instance")
# Best config column
rpd_cols <- paste0("RPD_", valid_neigh)
rpd_cols_present <- intersect(rpd_cols, names(inst_wide))
inst_wide$best_config <- apply(inst_wide[, rpd_cols_present], 1,
                                function(r) valid_neigh[which.min(r)])
inst_wide <- inst_wide[order(inst_wide$group, inst_wide$instance), ]

# ══════════════════════════════════════════════════════════════════════════════════
# ANALYSIS 3 — WILCOXON ON RPD (per-instance best makespan)
# ══════════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 3: Wilcoxon on Best-of-30 Makespan (per instance) ══\n")

best_wide <- reshape(
  best_per_ci[, c("instance","neigh","best_found")],
  idvar    = "instance",
  timevar  = "neigh",
  direction = "wide"
)
names(best_wide) <- gsub("^best_found\\.", "", names(best_wide))
avail_neigh <- intersect(valid_neigh, names(best_wide))

if (length(avail_neigh) >= 2) {
  fr_best <- run_friedman(best_wide[, avail_neigh, drop=FALSE], "best-of-30 makespan")
  wt_best <- pairwise_wilcoxon(best_wide[, avail_neigh, drop=FALSE], avail_neigh)
  cat("\nPairwise Wilcoxon on best-of-30 (Holm corrected):\n")
  print(wt_best[, c("A","B","n_pairs","mean_A","mean_B","mean_diff","p_adj","effect_r","sig")])
} else {
  cat("Insufficient neighbourhoods for comparison.\n")
  fr_best <- NULL; wt_best <- NULL
}

# ══════════════════════════════════════════════════════════════════════════════════
# ANALYSIS 4 — PER INSTANCE GROUP (all-runs makespan)
# ══════════════════════════════════════════════════════════════════════════════════
cat("\n\n══ ANALYSIS 4: Friedman per Instance Group (all runs) ══\n")

df$group <- ifelse(
  grepl("^F0\\.15\\.0\\.", df$instance),
  sub("^F0\\.15\\.0\\.([a-z]+[0-9]+).*", "\\1", df$instance),
  sub("^(tai[0-9]+_[0-9]+).*", "\\1", df$instance)
)

wt_per_group <- do.call(rbind, lapply(sort(unique(df$group)), function(g) {
  sub <- df[df$group == g, ]
  if (length(unique(sub$instance)) < 2) return(NULL)
  mat <- tryCatch(make_wide(sub, c("instance","run"), "neigh", "makespan", valid_neigh),
                  error = function(e) NULL)
  if (is.null(mat) || nrow(mat) < 5) return(NULL)
  ft   <- friedman.test(as.matrix(mat))
  rnks <- t(apply(mat, 1, rank))
  mr   <- colMeans(rnks, na.rm = TRUE)
  best <- names(which.min(mr))
  cat(sprintf("  %-12s  n=%2d  chi2=%6.2f  p=%.2e %s  best=%s\n",
              g, length(unique(sub$instance)), ft$statistic, ft$p.value,
              ifelse(ft$p.value < 0.05, "[sig]", "[n.s.]"), best))
  data.frame(group = g, n_instances = length(unique(sub$instance)),
             chi2 = round(ft$statistic, 3), p = ft$p.value,
             sig  = ft$p.value < 0.05, best_neigh = best,
             stringsAsFactors = FALSE)
}))

# ══════════════════════════════════════════════════════════════════════════════════
# SAVE
# ══════════════════════════════════════════════════════════════════════════════════
cat("\n\nSaving results...\n")

sink(file.path(out_dir, "friedman_neighbourhoods.txt"))
cat("=== Friedman Test: HC Configs — Exp8 CRISP ===\n\n")
cat("[ All runs, raw makespan ]\n")
print(fr_nb$test)
cat("\nMean ranks (lower = better):\n")
print(fr_nb$mean_ranks)
if (!is.null(fr_best)) {
  cat("\n\n[ Best-of-30 makespan per instance ]\n")
  print(fr_best$test)
  cat("\nMean ranks (lower = better):\n")
  print(fr_best$mean_ranks)
}
sink()

write.csv(wt_nb,         file.path(out_dir, "wilcoxon_all_runs.csv"),        row.names = FALSE)
if (!is.null(wt_best))
  write.csv(wt_best,     file.path(out_dir, "wilcoxon_best_of_30.csv"),      row.names = FALSE)
write.csv(rpd_by_config, file.path(out_dir, "rpd_by_config.csv"),            row.names = FALSE)
write.csv(rpd_by_group,  file.path(out_dir, "rpd_by_group.csv"),             row.names = FALSE)
write.csv(inst_wide,     file.path(out_dir, "rpd_per_instance.csv"),         row.names = FALSE)
if (!is.null(wt_per_group))
  write.csv(wt_per_group, file.path(out_dir, "friedman_by_group.csv"),       row.names = FALSE)

# ── Summary markdown ──────────────────────────────────────────────────────────
md <- c(
  "# Statistical Analysis — Exp8: irace-tuned HC on CRISP Instances", "",
  sprintf("Generated: %s", Sys.time()),
  sprintf("Data: %d observations (%d instances × ≤30 runs × 5 HC configs)",
          nrow(df), length(unique(df$instance))),
  "Metric: makespan (crisp integer, lower is better)",
  "BKS source: optimizizer.com/TA.php + OR-Library (accessed April 2026)", "",
  "## Methodology",
  "- **Test**: Friedman (non-parametric repeated-measures ANOVA)",
  "- **Blocks**: each (instance × run) pair",
  "- **Post-hoc**: pairwise Wilcoxon signed-rank, Holm-Bonferroni correction",
  "- **Effect size**: r = |Z| / √N  (small ≥ 0.1, medium ≥ 0.3, large ≥ 0.5)",
  "- **RPD**: (best_of_30_runs − BKS) / BKS × 100  (per instance per config)",
  "", "---", "",
  "## 1. HC Config Comparison — All Runs (raw makespan)",
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
  r  <- wt_nb[i, ]
  md <- c(md, sprintf("| %s | %s | %d | %.1f | %.1f | %.1f | %.4f | %.3f | %s | %s |",
    r$A, r$B, r$n_pairs, r$mean_A, r$mean_B, r$mean_diff,
    r$p_adj, r$effect_r, effect_mag(r$effect_r),
    ifelse(r$p_adj < 0.05, "✓", "✗")))
}

md <- c(md, "", "---", "",
  "## 2. RPD from Best Known Solutions",
  "",
  "RPD = (best\\_of\\_30\\_runs − BKS) / BKS × 100. Lower is better; 0 = matched BKS.",
  "",
  "| Config | Mean RPD% | Median RPD% | SD | Max RPD% | #Optimal | %Optimal |",
  "|--------|-----------|-------------|----|----------|----------|----------|"
)
for (i in seq_len(nrow(rpd_by_config))) {
  r  <- rpd_by_config[i, ]
  md <- c(md, sprintf("| %s | %.3f | %.3f | %.3f | %.3f | %d | %.1f%% |",
    r$neigh, r$rpd_mean, r$rpd_median, r$rpd_sd, r$rpd_max,
    r$n_optimal, r$pct_optimal))
}

md <- c(md, "", "---", "",
  "## 3. Best-of-30 Makespan Comparison"
)
if (!is.null(fr_best)) {
  md <- c(md,
    sprintf("**Friedman**: χ²=%.4f, df=%d, p %s",
            fr_best$test$statistic, fr_best$test$parameter, p_str(fr_best$test$p.value)),
    "",
    "Mean ranks (lower = better):",
    paste(sprintf("- %s: %.3f", names(fr_best$mean_ranks), fr_best$mean_ranks), collapse="\n"),
    "",
    "| A | B | n | Mean A | Mean B | Diff | p-adj | r | Magnitude | Sig |",
    "|---|---|---|--------|--------|------|-------|---|-----------|-----|"
  )
  for (i in seq_len(nrow(wt_best))) {
    r  <- wt_best[i, ]
    md <- c(md, sprintf("| %s | %s | %d | %.1f | %.1f | %.1f | %.4f | %.3f | %s | %s |",
      r$A, r$B, r$n_pairs, r$mean_A, r$mean_B, r$mean_diff,
      r$p_adj, r$effect_r, effect_mag(r$effect_r),
      ifelse(r$p_adj < 0.05, "✓", "✗")))
  }
}

md <- c(md, "", "---", "",
  "## 4. Results per Instance Group",
  "",
  "| Group | Instances | χ² | p | Significant | Best Config |",
  "|---|---|---|---|---|---|"
)
if (!is.null(wt_per_group) && nrow(wt_per_group) > 0) {
  for (i in seq_len(nrow(wt_per_group))) {
    r  <- wt_per_group[i, ]
    md <- c(md, sprintf("| %s | %d | %.3f | %.2e | %s | **%s** |",
      r$group, r$n_instances, r$chi2, r$p,
      ifelse(r$sig, "Yes", "No"), r$best_neigh))
  }
}

md <- c(md, "", "---", "",
  "## 5. RPD by Instance Group",
  "",
  "| Group | Config | n | Mean RPD% | Median RPD% | Max RPD% | #Optimal |",
  "|---|---|---|---|---|---|---|"
)
if (!is.null(rpd_by_group) && nrow(rpd_by_group) > 0) {
  for (i in seq_len(nrow(rpd_by_group))) {
    r  <- rpd_by_group[i, ]
    md <- c(md, sprintf("| %s | %s | %d | %.3f | %.3f | %.3f | %d |",
      r$group, r$neigh, r$n_inst, r$rpd_mean, r$rpd_median, r$rpd_max, r$n_optimal))
  }
}

writeLines(md, file.path(out_dir, "summary.md"))

cat("Files written to", out_dir, ":\n")
cat("  friedman_neighbourhoods.txt\n")
cat("  wilcoxon_all_runs.csv\n")
cat("  wilcoxon_best_of_30.csv\n")
cat("  rpd_by_config.csv\n")
cat("  rpd_by_group.csv\n")
cat("  rpd_per_instance.csv\n")
cat("  friedman_by_group.csv\n")
cat("  summary.md\n")
cat("\nDone.\n")
