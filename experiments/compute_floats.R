#!/usr/bin/env Rscript
#
# compute_floats.R
# For each config/instance, reconstructs G-(σ) and G+(σ) from the best
# solution saved in _Sols.csv and the instance .txt file, then computes
# operation floats in both extreme graphs.
#
# Outputs: statistical_results/floats_data.csv
# Columns: config, neigh, comp, instance, cmax_m, cmax_p, width,
#          n_ops, n_crit_m, n_crit_p, n_crit_both, n_crit_only_p,
#          mean_float_m, mean_float_p, total_float_m, total_float_p
#
# Hypothesis: does LEX2 produce schedules with more slack in G+(σ)?

cat("=== Float / Slack Analysis ===\n\n")

# ── Paths ──────────────────────────────────────────────────────────────────────
args <- commandArgs(trailingOnly = FALSE)
f    <- grep("--file=", args, value = TRUE)
script_dir  <- if (length(f)) dirname(normalizePath(sub("--file=", "", f[1]))) else getwd()
ijsp_dir    <- dirname(script_dir)
inst_dir    <- file.path(ijsp_dir, "SelectosYTaillardIntervalos")
results_dir <- file.path(script_dir, "results")
out_dir     <- file.path(script_dir, "statistical_results")
out_file    <- file.path(out_dir, "floats_data.csv")

NEIGHBOURHOODS <- c("n1","n2","n3","nh","next")
COMPARATORS    <- c("EV","LEX1","LEX2","YX")

# ── Parse instance file ────────────────────────────────────────────────────────
# Handles two formats:
#   Format A (F0/abz/ft/la): numeric header — line1=n_jobs, line2=n_mach,
#             then n_jobs routing rows, then n_jobs duration rows (tab-separated)
#   Format B (tai*): Spanish text headers — "NUMERO DE TRABAJOS", n_jobs,
#             "NUMERO DE RECURSOS", n_mach, "SECUENCIA DE MAQUINAS",
#             n_jobs routing rows, "DURACIONES", n_jobs duration rows
parse_instance <- function(filename) {
  raw   <- readLines(filename)
  lines <- raw[nchar(trimws(raw)) > 0]   # strip blank lines

  # Detect format: Format B has a text header on line 1
  is_format_b <- grepl("^[A-Za-z]", trimws(lines[1]))

  if (!is_format_b) {
    # ── Format A ──────────────────────────────────────────────────────────────
    n_jobs <- as.integer(trimws(lines[1]))
    n_mach <- as.integer(trimws(lines[2]))
    route_start <- 3L          # lines[3 .. 2+n_jobs]
    dur_start   <- 3L + n_jobs # lines[(3+n_jobs) .. (2+2*n_jobs)]
  } else {
    # ── Format B ──────────────────────────────────────────────────────────────
    # Expected structure (after blank-line removal):
    #   1: "NUMERO DE TRABAJOS"
    #   2: n_jobs
    #   3: "NUMERO DE RECURSOS"
    #   4: n_mach
    #   5: "SECUENCIA DE MAQUINAS"
    #   6 .. 5+n_jobs: routing rows
    #   6+n_jobs: "DURACIONES"
    #   7+n_jobs .. 6+2*n_jobs: duration rows
    n_jobs <- as.integer(trimws(lines[2]))
    n_mach <- as.integer(trimws(lines[4]))
    route_start <- 6L
    dur_start   <- 7L + n_jobs
  }

  # Routing matrix: n_jobs rows of n_mach integers each
  routing <- matrix(0L, n_jobs, n_mach)
  for (i in seq_len(n_jobs))
    routing[i,] <- as.integer(strsplit(trimws(lines[route_start + i - 1L]),
                                       "\\s+")[[1]])

  # Processing times: next n_jobs rows, each with n_mach "(lo, hi)" intervals
  plo <- matrix(0L, n_jobs, n_mach)
  phi <- matrix(0L, n_jobs, n_mach)
  for (i in seq_len(n_jobs)) {
    nums <- as.integer(regmatches(lines[dur_start + i - 1L],
                                  gregexpr("[0-9]+",
                                           lines[dur_start + i - 1L]))[[1]])
    for (j in seq_len(n_mach)) {
      plo[i,j] <- nums[2*j-1]
      phi[i,j] <- nums[2*j]
    }
  }

  list(n_jobs=n_jobs, n_mach=n_mach, routing=routing, plo=plo, phi=phi)
}

# ── Compute heads, tails and floats in one extreme graph ─────────────────────
compute_floats_one <- function(inst, perm, use_plus) {
  nj   <- inst$n_jobs; nm <- inst$n_mach; nops <- nj * nm
  # 0-based op → job, position, machine, processing time
  op_job  <- (0:(nops-1)) %/% nm
  op_pos  <- (0:(nops-1)) %% nm
  op_mach <- mapply(function(j,p) inst$routing[j+1L, p+1L], op_job, op_pos)
  op_p    <- if (use_plus)
    mapply(function(j,p) inst$phi[j+1L, p+1L], op_job, op_pos)
  else
    mapply(function(j,p) inst$plo[j+1L, p+1L], op_job, op_pos)

  # Job predecessor/successor (0-based indices, -1 = none)
  pred_job <- ifelse(op_pos > 0L,       0:(nops-1) - 1L, -1L)
  succ_job <- ifelse(op_pos < nm - 1L,  0:(nops-1) + 1L, -1L)

  # Machine predecessor from permutation
  pred_mach <- rep(-1L, nops)
  mach_last <- rep(-1L, nm)
  for (k in perm) {          # k is 0-based
    m <- op_mach[k+1L]
    if (mach_last[m+1L] >= 0L) pred_mach[k+1L] <- mach_last[m+1L]
    mach_last[m+1L] <- k
  }

  # Machine successor (inverse of pred_mach)
  succ_mach <- rep(-1L, nops)
  for (k in seq_len(nops)-1L) {
    pm <- pred_mach[k+1L]
    if (pm >= 0L) succ_mach[pm+1L] <- k
  }

  # Forward pass → heads r
  r <- rep(0L, nops)
  for (k in perm) {
    rk <- 0L
    pj <- pred_job[k+1L];  if (pj >= 0L) rk <- max(rk, r[pj+1L] + op_p[pj+1L])
    pm <- pred_mach[k+1L]; if (pm >= 0L) rk <- max(rk, r[pm+1L] + op_p[pm+1L])
    r[k+1L] <- rk
  }
  Cmax <- max(r + op_p)

  # Backward pass → tails q
  q <- rep(0L, nops)
  for (k in rev(perm)) {
    qk <- 0L
    sj <- succ_job[k+1L];  if (sj >= 0L) qk <- max(qk, op_p[sj+1L] + q[sj+1L])
    sm <- succ_mach[k+1L]; if (sm >= 0L) qk <- max(qk, op_p[sm+1L] + q[sm+1L])
    q[k+1L] <- qk
  }

  F <- Cmax - r - op_p - q
  list(Cmax=Cmax, F=F, n_crit=sum(F==0L),
       mean_float=mean(F), total_float=sum(F))
}

# ── Main loop ─────────────────────────────────────────────────────────────────
rows <- list()
total_done <- 0L

for (nb in NEIGHBOURHOODS) {
  for (comp in COMPARATORS) {
    config  <- paste0(nb, "_", comp)
    res_dir <- file.path(results_dir, config)

    # Collect latest _Sols.csv per instance (same logic as build_stats_data.sh)
    sols_files <- list.files(res_dir, pattern="_[0-9]{14}_Sols\\.csv$",
                             full.names=TRUE)
    # Strip timestamp to get stem
    stems <- sub("_[0-9]{14}_Sols\\.csv$", "", basename(sols_files))
    # Keep latest per stem
    df_files <- data.frame(stem=stems, path=sols_files, stringsAsFactors=FALSE)
    df_files <- df_files[order(df_files$path, decreasing=TRUE), ]
    df_files <- df_files[!duplicated(df_files$stem), ]

    for (i in seq_len(nrow(df_files))) {
      stem     <- df_files$stem[i]
      # Skip tai100
      if (startsWith(stem, "tai100")) next
      sols_path <- df_files$path[i]
      inst_path <- file.path(inst_dir, paste0(stem, ".txt"))
      if (!file.exists(inst_path)) next

      # Read Sols: pick best run (lowest midpoint)
      sols <- tryCatch(read.csv(sols_path, sep=";", stringsAsFactors=FALSE),
                       error=function(e) NULL)
      if (is.null(sols) || nrow(sols)==0) next

      # Parse objective values "(lo, hi)"
      nums <- regmatches(sols[["Objective.Value"]],
                         gregexpr("[0-9]+", sols[["Objective.Value"]]))
      lo_vals <- sapply(nums, function(x) as.numeric(x[1]))
      hi_vals <- sapply(nums, function(x) as.numeric(x[2]))
      mid_vals <- (lo_vals + hi_vals) / 2

      best_row <- which.min(mid_vals)
      best_sol_str <- sols[best_row, "Solution"]

      # Parse permutation
      perm <- as.integer(strsplit(trimws(best_sol_str), "\\s+")[[1]])

      # Parse instance
      inst <- tryCatch(parse_instance(inst_path), error=function(e) NULL)
      if (is.null(inst)) next

      # Sanity check
      n_ops <- inst$n_jobs * inst$n_mach
      if (length(perm) != n_ops) next

      # Compute floats in G- and G+
      fm <- compute_floats_one(inst, perm, use_plus=FALSE)
      fp <- compute_floats_one(inst, perm, use_plus=TRUE)

      n_crit_both   <- sum(fm$F == 0L & fp$F == 0L)
      n_crit_only_p <- sum(fm$F > 0L  & fp$F == 0L)

      rows[[length(rows)+1]] <- data.frame(
        config        = config,
        neigh         = nb,
        comp          = comp,
        instance      = stem,
        cmax_m        = fm$Cmax,
        cmax_p        = fp$Cmax,
        width         = fp$Cmax - fm$Cmax,
        n_ops         = n_ops,
        n_crit_m      = fm$n_crit,
        n_crit_p      = fp$n_crit,
        n_crit_both   = n_crit_both,
        n_crit_only_p = n_crit_only_p,
        mean_float_m  = round(fm$mean_float, 4),
        mean_float_p  = round(fp$mean_float, 4),
        total_float_m = fm$total_float,
        total_float_p = fp$total_float,
        stringsAsFactors = FALSE
      )
      total_done <- total_done + 1L
    }
    cat(sprintf("  %s: %d instances processed\n", config, nrow(df_files)))
  }
}

result <- do.call(rbind, rows)
write.csv(result, out_file, row.names=FALSE)
cat(sprintf("\nDone. %d rows written to %s\n", nrow(result), out_file))

# ── Quick summary by comparator ───────────────────────────────────────────────
cat("\n=== Summary by ranking operator (mean across all neighbourhoods) ===\n\n")
for (comp in COMPARATORS) {
  sub <- result[result$comp == comp, ]
  cat(sprintf("%-5s  mean_float_p=%.1f  n_crit_p=%.1f  n_crit_only_p=%.1f  width=%.1f\n",
    comp,
    mean(sub$mean_float_p),
    mean(sub$n_crit_p),
    mean(sub$n_crit_only_p),
    mean(sub$width)))
}
