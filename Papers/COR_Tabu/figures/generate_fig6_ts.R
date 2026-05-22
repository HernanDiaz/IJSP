#!/usr/bin/env Rscript
# =============================================================================
# generate_fig6_ts.R  (COR_Tabu — Tabu Search version)
# Figure 6: Per-generation convergence profiles, irace-tuned TS (Phase B, exp4)
# Data: experiments/statistical_results_exp4/convergence_data_ts.csv
# Metric: best_norm = best_midpoint[step] / best_midpoint[0]  (lower = better)
# Averaged over 82 instances × 30 runs per (neigh, step)
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
})

BASE_DIR  <- "C:/Users/diazhernan/CLionProjects/IJSP"
COR_DIR   <- file.path(BASE_DIR, "Papers/COR_Tabu")
OUT_DIR   <- file.path(COR_DIR, "figures")
DATA_CONV <- file.path(BASE_DIR,
               "experiments/statistical_results_exp4/convergence_data_ts.csv")

MM2IN   <- 1 / 25.4
W_FULL  <- 160 * MM2IN
H_FIG6  <-  90 * MM2IN
BASE_PT <- 9
AXIS_PT <- 8

# TS ordering: best → worst
NEIGH_ORDER  <- c("N2", "N8", "N_ext", "N1", "N3")
NEIGH_COLORS <- c("N2"    = "#E69F00", "N8"    = "#D55E00",
                  "N_ext" = "#0072B2", "N1"    = "#009E73",
                  "N3"    = "#CC79A7")
NEIGH_LINES  <- c("N2"    = "solid",   "N8"    = "dashed",
                  "N_ext" = "solid",   "N1"    = "dotdash",
                  "N3"    = "dotted")
neigh_map <- c(n1 = "N1", n2 = "N2", n3 = "N3", n8 = "N8",
               "next" = "N_ext", next. = "N_ext")

ijsp_theme <- function(base = BASE_PT) {
  theme_bw(base_size = base, base_family = "sans") +
    theme(
      panel.grid.minor  = element_blank(),
      panel.grid.major  = element_line(colour = "grey88", linewidth = 0.3),
      panel.border      = element_rect(colour = "grey40", linewidth = 0.5),
      axis.title        = element_text(size = base),
      axis.text         = element_text(size = AXIS_PT),
      legend.text       = element_text(size = AXIS_PT),
      legend.title      = element_text(size = AXIS_PT),
      plot.margin       = margin(1, 1, 1, 1, "mm")
    )
}

# ── Load data ─────────────────────────────────────────────────────────────────
dc <- read.csv(DATA_CONV, stringsAsFactors = FALSE)
dc$neigh_f <- factor(neigh_map[dc$neigh], levels = NEIGH_ORDER)
dc <- dc[!is.na(dc$neigh_f), ]

# ── Trim to steps with reasonable coverage ────────────────────────────────────
# Keep only steps where n_obs >= 10% of step-0 observations (filter sparse tail)
step0_n <- max(dc$n_obs[dc$step == 0])
dc <- dc[dc$n_obs >= step0_n * 0.10, ]

# Cap x-axis at generation 50
dc <- dc[dc$step <= 50, ]

# Compute SE ribbon
dc$se <- dc$sd_best_norm / sqrt(dc$n_obs)

cat("=== Convergence data check ===\n")
dc |>
  group_by(neigh_f) |>
  summarise(
    max_step     = max(step),
    final_mean   = mean_best_norm[which.max(step)],
    .groups = "drop"
  ) |>
  arrange(final_mean) |>
  print()

# ── Plot ──────────────────────────────────────────────────────────────────────
p6 <- ggplot(dc, aes(x = step, y = mean_best_norm,
                      colour = neigh_f, linetype = neigh_f,
                      fill = neigh_f, group = neigh_f)) +
  geom_ribbon(aes(ymin = mean_best_norm - se,
                  ymax = mean_best_norm + se),
              alpha = 0.12, colour = NA) +
  geom_line(linewidth = 0.7) +
  scale_colour_manual(name = "Neighbourhood", values = NEIGH_COLORS) +
  scale_fill_manual(  name = "Neighbourhood", values = NEIGH_COLORS) +
  scale_linetype_manual(name = "Neighbourhood", values = NEIGH_LINES) +
  scale_x_continuous(name = "Generation", expand = expansion(mult = c(0.01, 0.03))) +
  scale_y_continuous(
    name   = "Normalised best midpoint (lower = better)",
    labels = scales::percent_format(accuracy = 1),
    expand = expansion(mult = c(0.01, 0.03))
  ) +
  ijsp_theme() +
  theme(
    legend.position  = "right",
    legend.key.width = unit(1.2, "cm")
  )

# ── Save ──────────────────────────────────────────────────────────────────────
fpath <- file.path(OUT_DIR, "fig6_convergence.pdf")
cairo_pdf(filename = fpath, width = W_FULL, height = H_FIG6,
          family = "sans", antialias = "subpixel")
print(p6)
dev.off()
cat(sprintf("[OK] %s\n", fpath))
