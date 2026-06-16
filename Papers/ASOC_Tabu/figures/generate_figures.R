#!/usr/bin/env Rscript
# =============================================================================
# generate_figures.R
# Manuscript figures for:
#   "Neighbourhood Structures and Ranking Operators for the IJSP"
#   Target: Applied Soft Computing (Elsevier)
#
# Elsevier guidelines applied:
#   - Font: Arial (sans-serif), minimum 8 pt in figure, base 9 pt
#   - Line weight: >= 0.5 pt
#   - Width: 160 mm (full text), 78 mm (single column)
#   - Vector PDF via cairo_pdf (resolution-independent)
#   - Colours: Okabe-Ito palette (colorblind-safe, distinguishable in B&W)
#
# Figures generated:
#   fig3_quality_by_neighbourhood.pdf   -- Exp7: HC makespan by neighbourhood
#   fig4_width_by_operator.pdf          -- Exp1: interval width by ranking op
#   fig5_float_analysis.pdf             -- Exp1: float structure by ranking op
#   fig6_convergence.pdf                -- Exp1: convergence profiles
#   fig7_crisp_rpd_heatmap.pdf          -- Exp8: CRISP RPD% by group
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
  library(tidyr)
  library(scales)
  library(patchwork)
})

# ---- Paths ------------------------------------------------------------------
COR_DIR     <- "C:/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu"
OUT_DIR     <- file.path(COR_DIR, "figures")
DATA_EXP4   <- file.path(COR_DIR, "data/phaseB_ts/runs_data.csv")   # Exp4: irace-tuned TS
DATA_FLOATS <- file.path(COR_DIR, "data/phaseA/floats_data.csv")
DATA_CONV   <- file.path(COR_DIR, "data/phaseA/convergence_data.csv")
DATA_RPD_G  <- file.path(COR_DIR, "data/phaseC_ts/rpd_by_group.csv") # generated after exp9

# ---- Elsevier constants -----------------------------------------------------
MM2IN      <- 1 / 25.4
W_FULL_MM  <- 160    # full text width
W_HALF_MM  <- 78     # single column
BASE_PT    <- 9      # body font size in figure (Elsevier min 8 pt)
AXIS_PT    <- 8      # axis tick labels
TITLE_PT   <- 9

# ---- Okabe-Ito colour palette (colorblind-safe) -----------------------------
# 5 neighbourhoods (ordered best→worst in Exp4 TS: N2>N8>N_ext>N1>N3)
NEIGH_ORDER  <- c("N2", "N8", "N_ext", "N1", "N3")
NEIGH_COLORS <- c(
  "N_ext" = "#0072B2",   # Blue
  "N1"    = "#009E73",   # Bluish Green
  "N2"    = "#E69F00",   # Orange
  "N8"    = "#D55E00",   # Vermillion
  "N3"    = "#CC79A7"    # Reddish Purple
)
NEIGH_SHAPES <- c("N_ext"=16, "N1"=17, "N2"=15, "N8"=18, "N3"=4)

# 4 ranking operators (ordered best→worst on interval width: narrowest first)
OP_ORDER  <- c("LEX2", "YX", "EV", "LEX1")
OP_COLORS <- c(
  "LEX2" = "#0072B2",   # Blue     (narrowest interval)
  "YX"   = "#009E73",   # Green
  "EV"   = "#E69F00",   # Orange
  "LEX1" = "#D55E00"    # Vermillion (widest interval)
)
OP_SHAPES <- c("LEX2"=16, "YX"=17, "EV"=15, "LEX1"=4)

# ---- Shared theme -----------------------------------------------------------
ijsp_theme <- function(base = BASE_PT) {
  theme_bw(base_size = base, base_family = "sans") +
    theme(
      panel.grid.minor      = element_blank(),
      panel.grid.major      = element_line(colour = "grey88", linewidth = 0.3),
      panel.border          = element_rect(colour = "grey40", linewidth = 0.5),
      strip.background      = element_rect(fill = "grey92", colour = "grey50",
                                           linewidth = 0.4),
      strip.text            = element_text(size = base, face = "bold"),
      axis.title            = element_text(size = base),
      axis.text             = element_text(size = AXIS_PT),
      legend.text           = element_text(size = AXIS_PT),
      legend.title          = element_text(size = base, face = "bold"),
      legend.key.size       = unit(0.35, "cm"),
      legend.background     = element_rect(fill = "white", colour = "grey70",
                                           linewidth = 0.3),
      plot.margin           = margin(3, 4, 2, 4, "mm")
    )
}

# ---- Save helper ------------------------------------------------------------
save_fig <- function(p, name, w_mm = W_FULL_MM, h_mm = 90) {
  fpath <- file.path(OUT_DIR, paste0(name, ".pdf"))
  cairo_pdf(filename = fpath,
            width    = w_mm * MM2IN,
            height   = h_mm * MM2IN,
            family   = "sans",
            antialias = "subpixel")
  print(p)
  dev.off()
  cat(sprintf("  [OK] %s  (%.0f x %.0f mm)\n", basename(fpath), w_mm, h_mm))
}

# ---- Significance bracket helper --------------------------------------------
# Draw a single bracket: from x1 to x2 at height y, with label above
sig_bracket <- function(x1, x2, y, label, tick_h = NULL, label_size = 2.2) {
  if (is.null(tick_h)) tick_h <- (y - 0) * 0.008
  list(
    annotate("segment", x = x1, xend = x2,    y = y, yend = y,
             linewidth = 0.4, colour = "grey20"),
    annotate("segment", x = x1, xend = x1,    y = y - tick_h, yend = y,
             linewidth = 0.4, colour = "grey20"),
    annotate("segment", x = x2, xend = x2,    y = y - tick_h, yend = y,
             linewidth = 0.4, colour = "grey20"),
    annotate("text", x = (x1 + x2) / 2, y = y + tick_h * 0.6,
             label = label, size = label_size, colour = "grey10")
  )
}

# =============================================================================
# FIGURE 3 — Solution quality by neighbourhood (Exp4, irace-tuned TS)
# Data:  data/phaseB_ts/runs_data.csv
# 82 instances × 30 runs × 5 neighbourhoods = 12300 obs
# Wilcoxon (Holm): ALL pairs significant; Friedman chi2=4120.6, p<0.001
# TS ranking: N2 > N8 > N_ext > N1 > N3  (N2 vs N1: r=0.64 large effect)
# =============================================================================
cat("=== Figure 3: Quality by neighbourhood (Exp4, TS) ===\n")

d7 <- read.csv(DATA_EXP4, stringsAsFactors = FALSE)

# Remap neigh labels and factor
neigh_map <- c(n1 = "N1", n2 = "N2", n3 = "N3", n8 = "N8", next. = "N_ext",
               "next" = "N_ext")
d7$neigh_f <- factor(neigh_map[d7$neigh], levels = NEIGH_ORDER)

# Per-instance RPD% normalisation: removes scale variance between instances
# (raw midpoints span 686-3196, swamping the ~15-unit between-neighbourhood signal)
d7 <- d7 %>%
  group_by(instance) %>%
  mutate(rpd_pct = (midpoint - min(midpoint)) / min(midpoint) * 100) %>%
  ungroup()

p3 <- ggplot(d7, aes(x = neigh_f, y = rpd_pct, fill = neigh_f)) +
  geom_boxplot(
    outlier.size   = 0.4,
    outlier.alpha  = 0.35,
    outlier.colour = "grey50",
    linewidth      = 0.45,
    fatten         = 1.6,
    width          = 0.65
  ) +
  scale_fill_manual(values = NEIGH_COLORS, name = "Neighbourhood") +
  scale_y_continuous(
    name   = expression("Per-instance RPD on midpoint  [%]"),
    labels = function(x) sprintf("%.1f", x),
    expand = expansion(mult = c(0.02, 0.06))
  ) +
  xlab("Neighbourhood") +
  ijsp_theme() +
  theme(legend.position = "none")

save_fig(p3, "fig3_quality_by_neighbourhood", w_mm = W_FULL_MM, h_mm = 100)

# =============================================================================
# FIGURE 4 — Makespan interval width by ranking operator (Exp1 float data)
# Data:  experiments/statistical_results/floats_data.csv
# 1640 obs: 5 neigh × 4 operators × 82 instances (best solution per config)
# Metric: width = C_max^+ − C_max^−
# Key finding: LEX2 produces intervals ~11% narrower than LEX1
# =============================================================================
cat("=== Figure 4: Interval width by operator ===\n")

df <- read.csv(DATA_FLOATS, stringsAsFactors = FALSE)

op_map  <- c(EV = "EV", LEX1 = "LEX1", LEX2 = "LEX2", YX = "YX")
df$op_f <- factor(df$comp, levels = OP_ORDER)

# Per-operator summary
op_summary <- df %>% group_by(op_f) %>%
  summarise(
    mean_w  = mean(width, na.rm = TRUE),
    sd_w    = sd(width,   na.rm = TRUE),
    n       = n(),
    se_w    = sd_w / sqrt(n),
    .groups = "drop"
  )

# Percentage change vs EV (reference)
ev_mean <- op_summary$mean_w[op_summary$op_f == "EV"]
op_summary$pct_vs_ev <- round((op_summary$mean_w - ev_mean) / ev_mean * 100, 1)

p4 <- ggplot(df, aes(x = op_f, y = width, fill = op_f)) +
  geom_boxplot(
    outlier.size   = 0.4,
    outlier.alpha  = 0.35,
    outlier.colour = "grey50",
    linewidth      = 0.45,
    fatten         = 1.6,
    width          = 0.65
  ) +
  geom_hline(yintercept = ev_mean, linetype = "dashed",
             colour = "grey50", linewidth = 0.4) +
  annotate("text", x = 4.4, y = ev_mean + 3,
           label = "EV mean", size = 2.4, colour = "grey40", hjust = 1) +
  scale_fill_manual(values = OP_COLORS) +
  scale_y_continuous(
    name   = expression("Makespan interval width  ["*C[sigma]^"+"*" \u2212 "*C[sigma]^"\u2212"*"]"),
    labels = number_format(big.mark = ","),
    expand = expansion(mult = c(0.02, 0.05))
  ) +
  xlab("Ranking operator") +
  ijsp_theme() +
  theme(legend.position = "none")

save_fig(p4, "fig4_width_by_operator", w_mm = W_HALF_MM + 20, h_mm = 95)

# =============================================================================
# FIGURE 5 — Structural float analysis: why LEX2 narrows intervals (Exp1)
# Data:  experiments/statistical_results/floats_data.csv
# Two-panel: (a) critical ops in G^+(sigma) by operator
#            (b) per-op slack in G^+(sigma) by operator
# Key message: LEX2 has MORE critical ops and LESS slack in G^+ —
#              it tightens the worst-case path directly (not "more room")
# =============================================================================
cat("=== Figure 5: Float analysis ===\n")

# Aggregate by operator (averaging over all 5 neigh and 82 instances)
float_agg <- df %>%
  group_by(op_f) %>%
  summarise(
    mean_ncrit  = mean(n_crit_p,     na.rm = TRUE),
    se_ncrit    = sd(n_crit_p,       na.rm = TRUE) / sqrt(n()),
    mean_float  = mean(mean_float_p, na.rm = TRUE),
    se_float    = sd(mean_float_p,   na.rm = TRUE) / sqrt(n()),
    n           = n(),
    .groups     = "drop"
  )

# Pre-compute annotation positions so they sit ABOVE the error bar
# (vs. inside the bar — which was unreadable in the previous version)
lex2_ncrit <- float_agg$mean_ncrit[float_agg$op_f == "LEX2"]
lex2_seN   <- float_agg$se_ncrit[  float_agg$op_f == "LEX2"]
lex1_ncrit <- float_agg$mean_ncrit[float_agg$op_f == "LEX1"]
lex2_float <- float_agg$mean_float[float_agg$op_f == "LEX2"]
lex2_seF   <- float_agg$se_float[  float_agg$op_f == "LEX2"]
lex1_float <- float_agg$mean_float[float_agg$op_f == "LEX1"]

# Panel (a): critical ops in G^+
pa <- ggplot(float_agg, aes(x = op_f, y = mean_ncrit, fill = op_f)) +
  geom_col(width = 0.65, colour = "grey30", linewidth = 0.3) +
  geom_errorbar(
    aes(ymin = mean_ncrit - se_ncrit, ymax = mean_ncrit + se_ncrit),
    width = 0.22, linewidth = 0.5, colour = "grey20"
  ) +
  # Curved arrow + label showing trend (LEX2 has the most critical ops)
  annotate("segment",
           x = 4, xend = 1,
           y = lex1_ncrit + 4, yend = lex2_ncrit + lex2_seN + 1.5,
           arrow = arrow(length = unit(2, "mm"), type = "closed"),
           colour = "grey25", linewidth = 0.4) +
  annotate("text",
           x = 2.5, y = max(float_agg$mean_ncrit) + 6,
           label = "more critical ops",
           size = 2.4, colour = "grey15",
           fontface = "italic") +
  scale_fill_manual(values = OP_COLORS) +
  scale_y_continuous(
    name   = expression("Critical ops in "*G^"+"*"("*sigma*")  [mean]"),
    expand = expansion(mult = c(0.02, 0.18))
  ) +
  xlab(NULL) +
  ggtitle(expression("(a) "*G^"+"*" critical-path saturation")) +
  ijsp_theme() +
  theme(
    legend.position = "none",
    plot.title      = element_text(size = BASE_PT, face = "plain")
  )

# Panel (b): per-op float in G^+
pb <- ggplot(float_agg, aes(x = op_f, y = mean_float, fill = op_f)) +
  geom_col(width = 0.65, colour = "grey30", linewidth = 0.3) +
  geom_errorbar(
    aes(ymin = mean_float - se_float, ymax = mean_float + se_float),
    width = 0.22, linewidth = 0.5, colour = "grey20"
  ) +
  # Curved arrow + label (LEX2 has the lowest slack)
  annotate("segment",
           x = 4, xend = 1,
           y = lex1_float + 2.5, yend = lex2_float + lex2_seF + 1.0,
           arrow = arrow(length = unit(2, "mm"), type = "closed"),
           colour = "grey25", linewidth = 0.4) +
  annotate("text",
           x = 2.5, y = max(float_agg$mean_float) + 4,
           label = "less slack per op",
           size = 2.4, colour = "grey15",
           fontface = "italic") +
  scale_fill_manual(values = OP_COLORS) +
  scale_y_continuous(
    name   = expression("Mean float in "*G^"+"*"("*sigma*")  [time units]"),
    expand = expansion(mult = c(0.02, 0.18))
  ) +
  xlab(NULL) +
  ggtitle(expression("(b) Per-operation slack in "*G^"+"*"("*sigma*")")) +
  ijsp_theme() +
  theme(
    legend.position = "none",
    plot.title      = element_text(size = BASE_PT, face = "plain")
  )

# Add shared x-axis label via patchwork
p5 <- (pa | pb) +
  plot_annotation(
    caption = "Ranking operator",
    theme   = theme(
      plot.caption = element_text(hjust = 0.5, size = BASE_PT,
                                  margin = margin(t = -2, b = 2, unit = "mm"))
    )
  )

save_fig(p5, "fig5_float_analysis", w_mm = W_FULL_MM, h_mm = 85)

# =============================================================================
# FIGURE 6 — Convergence profiles by neighbourhood (Exp1, all operators avg)
# Data:  experiments/statistical_results/convergence_data.csv
# Metric: best_norm = best_makespan[step] / best_makespan[0]  (lower = better)
# Average over all 4 operators and all 82 instances per (neigh, step)
# `nh` in this dataset = N8 neighbourhood
# =============================================================================
cat("=== Figure 6: Convergence profiles ===\n")

dc <- read.csv(DATA_CONV, stringsAsFactors = FALSE)

# Map nh → N8 (Exp1 used "nh"; Exp7/8 use "n8" — accept both)
neigh_map2 <- c(n1 = "N1", n2 = "N2", n3 = "N3", next. = "N_ext",
                "next" = "N_ext", nh = "N8", n8 = "N8")
dc$neigh_f <- factor(neigh_map2[dc$neigh], levels = NEIGH_ORDER)

# Aggregate over operators and instances
conv_agg <- dc %>%
  group_by(neigh_f, step) %>%
  summarise(
    mean_norm = mean(best_norm, na.rm = TRUE),
    se_norm   = sd(best_norm,   na.rm = TRUE) / sqrt(n()),
    n_obs     = n(),
    .groups   = "drop"
  ) %>%
  filter(!is.na(neigh_f))

# Only plot up to step 100 (region where most improvement happens)
STEP_MAX <- 100
conv_plot <- conv_agg %>% filter(step <= STEP_MAX)

# Compute the "90% of improvement" step per neighbourhood
conv_90 <- conv_agg %>%
  group_by(neigh_f) %>%
  summarise(
    y0    = mean_norm[step == 0],
    y_fin = mean_norm[step == max(step)],
    .groups = "drop"
  ) %>%
  mutate(threshold = y0 - 0.90 * (y0 - y_fin))

p6 <- ggplot(conv_plot, aes(x = step, y = mean_norm,
                             colour = neigh_f, linetype = neigh_f)) +
  geom_ribbon(
    aes(ymin = mean_norm - se_norm, ymax = mean_norm + se_norm, fill = neigh_f),
    alpha = 0.10, colour = NA
  ) +
  geom_line(linewidth = 0.7) +
  scale_colour_manual(values = NEIGH_COLORS, name = "Neighbourhood") +
  scale_fill_manual(  values = NEIGH_COLORS, name = "Neighbourhood") +
  scale_linetype_manual(
    values = c("N_ext" = "solid", "N1" = "dashed", "N2" = "dotdash",
               "N8" = "longdash", "N3" = "dotted"),
    name = "Neighbourhood"
  ) +
  scale_x_continuous(
    name   = "Generation",
    breaks = c(0, 20, 40, 60, 80, 100),
    expand = expansion(mult = c(0.01, 0.02))
  ) +
  scale_y_continuous(
    name   = expression("Normalised best makespan  [best / "*"best"[0]*"]"),
    limits = c(0.81, 1.005),
    expand = expansion(mult = c(0.0, 0.01))
  ) +
  ijsp_theme() +
  theme(
    legend.position  = c(0.99, 0.99),
    legend.justification = c(1, 1),
    legend.key.width  = unit(1.0, "cm"),
    legend.spacing.y  = unit(0.05, "cm")
  )

save_fig(p6, "fig6_convergence", w_mm = W_FULL_MM, h_mm = 90)

# =============================================================================
# FIGURE 7 — CRISP validation: RPD% vs BKS by neighbourhood × instance group
# Data:  experiments/statistical_results_exp8/rpd_by_group.csv
# Exp8: 82 classical JSP instances (CRISP p^- = p^+), irace-tuned HC
# RPD = (best_of_30 - BKS) / BKS * 100  (BKS from optimizizer.com, Apr 2026)
# =============================================================================
if (!file.exists(DATA_RPD_G)) {
  cat("=== Figure 7: Skipped — Phase C TS data not yet available ===\n")
  cat("    Run exp9 and build_stats_data_exp9.py first.\n")
} else {
cat("=== Figure 7: CRISP RPD heatmap ===\n")

dg <- read.csv(DATA_RPD_G, stringsAsFactors = FALSE)
dg$neigh_f <- factor(neigh_map2[dg$neigh], levels = NEIGH_ORDER)

# Define a readable group order (small classical → large Taillard)
group_order <- c(
  "ft10", "ft20",
  "abz7", "abz8", "abz9",
  "la21", "la24", "la25", "la27", "la29", "la38", "la40",
  "tai15_15", "tai20_15", "tai20_20",
  "tai30_15", "tai30_20",
  "tai50_15", "tai50_20"
)
# Pretty labels
group_labels <- c(
  "ft10"     = "ft10 (10\u00d710)",
  "ft20"     = "ft20 (20\u00d75)",
  "abz7"     = "abz7 (10\u00d710)",
  "abz8"     = "abz8 (10\u00d710)",
  "abz9"     = "abz9 (10\u00d710)",
  "la21"     = "la21 (15\u00d74)",
  "la24"     = "la24 (15\u00d75)",
  "la25"     = "la25 (15\u00d75)",
  "la27"     = "la27 (20\u00d710)",
  "la29"     = "la29 (20\u00d710)",
  "la38"     = "la38 (15\u00d715)",
  "la40"     = "la40 (15\u00d715)",
  "tai15_15" = "tai (15\u00d715) avg",
  "tai20_15" = "tai (20\u00d715) avg",
  "tai20_20" = "tai (20\u00d720) avg",
  "tai30_15" = "tai (30\u00d715) avg",
  "tai30_20" = "tai (30\u00d720) avg",
  "tai50_15" = "tai (50\u00d715) avg",
  "tai50_20" = "tai (50\u00d720) avg"
)

# Filter to available groups and reorder
dg_plot <- dg %>%
  filter(group %in% group_order, !is.na(neigh_f)) %>%
  mutate(
    group_f = factor(group, levels = rev(group_order)),   # reverse for y-axis
    label_f = factor(group_labels[group], levels = rev(group_labels[group_order]))
  )

# For taib groups with multiple instances, rpd_mean is already the mean over n_inst
# Color scale: 0–12%, 0=green, 6=yellow, 12=red
RPD_LIM <- c(0, 12)

p7 <- ggplot(dg_plot, aes(x = neigh_f, y = group_f, fill = rpd_mean)) +
  geom_tile(colour = "white", linewidth = 0.5) +
  geom_text(
    aes(label = sprintf("%.1f", rpd_mean)),
    size   = 2.3,
    colour = ifelse(dg_plot$rpd_mean > 6, "white", "grey15")
  ) +
  scale_fill_gradientn(
    name   = "RPD (%)",
    colours = c("#1a9850", "#91cf60", "#d9ef8b", "#fee08b", "#fc8d59", "#d73027"),
    values  = rescale(c(0, 2, 4, 6, 9, 12)),
    limits  = RPD_LIM,
    oob     = squish,
    guide   = guide_colourbar(
      barwidth  = unit(3.5, "cm"),
      barheight = unit(0.35, "cm"),
      title.position = "top",
      title.hjust    = 0.5
    )
  ) +
  # Group separator lines
  geom_hline(yintercept = c(2.5, 5.5, 7.5, 12.5, 14.5, 16.5),
             colour = "grey55", linewidth = 0.6, linetype = "solid") +
  scale_x_discrete(
    name     = "Neighbourhood",
    position = "bottom",
    expand   = expansion(0)
  ) +
  scale_y_discrete(
    name   = NULL,
    labels = group_labels[levels(dg_plot$group_f)]
  ) +
  ijsp_theme() +
  theme(
    axis.text.y     = element_text(size = 7.5),
    legend.position = "top",
    legend.margin   = margin(0, 0, -1, 0, "mm"),
    panel.grid      = element_blank(),
    panel.border    = element_rect(colour = "grey40", linewidth = 0.5),
    axis.ticks      = element_line(linewidth = 0.3)
  )

save_fig(p7, "fig7_crisp_rpd_heatmap", w_mm = W_FULL_MM, h_mm = 130)
} # end if (file.exists(DATA_RPD_G))

# =============================================================================
# Summary
# =============================================================================
cat("\nAll figures saved to:", OUT_DIR, "\n")
cat(paste(list.files(OUT_DIR, pattern = "\\.pdf$"), collapse = "\n"), "\n")
