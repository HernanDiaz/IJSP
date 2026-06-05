#!/usr/bin/env Rscript
# =============================================================================
# combine_fig34.R
# Combined 3-panel figure merging current Figure 3 (width by operator) and
# Figure 4 (float analysis, 2 panels):
#   (a) Makespan interval width by operator
#   (b) Critical ops in G+(sigma) by operator
#   (c) Per-operation slack in G+(sigma) by operator
# Layout: (a) on top full-width, (b) and (c) side-by-side below
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
  library(scales)
  library(patchwork)
})

COR_DIR     <- "C:/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu"
OUT_DIR     <- file.path(COR_DIR, "figures")
DATA_FLOATS <- file.path(COR_DIR, "data/phaseA/floats_data.csv")

MM2IN    <- 1 / 25.4
W_FULL   <- 160 * MM2IN
BASE_PT  <- 9
AXIS_PT  <- 8

OP_ORDER  <- c("LEX2", "YX", "EV", "LEX1")
OP_COLORS <- c(LEX2="#0072B2", YX="#009E73", EV="#E69F00", LEX1="#D55E00")

ijsp_theme <- function(base = BASE_PT) {
  theme_bw(base_size = base, base_family = "sans") +
    theme(
      panel.grid.minor  = element_blank(),
      panel.grid.major  = element_line(colour = "grey88", linewidth = 0.3),
      panel.border      = element_rect(colour = "grey40", linewidth = 0.5),
      strip.background  = element_rect(fill = "grey92", colour = "grey50",
                                       linewidth = 0.4),
      strip.text        = element_text(size = base, face = "bold"),
      axis.title        = element_text(size = base),
      axis.text         = element_text(size = AXIS_PT),
      legend.text       = element_text(size = AXIS_PT),
      legend.title      = element_text(size = base, face = "bold"),
      legend.key.size   = unit(0.35, "cm"),
      legend.background = element_rect(fill = "white", colour = "grey70",
                                       linewidth = 0.3),
      plot.margin       = margin(0, 1, 0, 1, "mm")   # márgenes mínimos
    )
}

# ---- Load data ---------------------------------------------------------------
df <- read.csv(DATA_FLOATS, stringsAsFactors = FALSE)
df$op_f <- factor(df$comp, levels = OP_ORDER)

# ---- Panel (a): width by operator --------------------------------------------
op_summary <- df %>%
  group_by(op_f) %>%
  summarise(mean_w = mean(width, na.rm = TRUE), .groups = "drop")

ev_mean <- op_summary$mean_w[op_summary$op_f == "EV"]

pa <- ggplot(df, aes(x = op_f, y = width, fill = op_f)) +
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
    name   = expression("Interval width  ["*C[max]^"+"*" − "*C[max]^"−"*"]"),
    labels = number_format(big.mark = ","),
    expand = expansion(mult = c(0.02, 0.05))
  ) +
  xlab("Ranking operator") +
  ijsp_theme() +
  theme(legend.position = "none")

# ---- Panels (b) and (c): float analysis in G^+(sigma) -----------------------
float_agg <- df %>%
  group_by(op_f) %>%
  summarise(
    mean_ncrit = mean(n_crit_p,     na.rm = TRUE),
    se_ncrit   = sd(n_crit_p,       na.rm = TRUE) / sqrt(n()),
    mean_float = mean(mean_float_p, na.rm = TRUE),
    se_float   = sd(mean_float_p,   na.rm = TRUE) / sqrt(n()),
    .groups    = "drop"
  )

lex2_ncrit <- float_agg$mean_ncrit[float_agg$op_f == "LEX2"]
lex2_seN   <- float_agg$se_ncrit[  float_agg$op_f == "LEX2"]
lex1_ncrit <- float_agg$mean_ncrit[float_agg$op_f == "LEX1"]
lex2_float <- float_agg$mean_float[float_agg$op_f == "LEX2"]
lex2_seF   <- float_agg$se_float[  float_agg$op_f == "LEX2"]
lex1_float <- float_agg$mean_float[float_agg$op_f == "LEX1"]

pb <- ggplot(float_agg, aes(x = op_f, y = mean_ncrit, fill = op_f)) +
  geom_col(width = 0.65, colour = "grey30", linewidth = 0.3) +
  geom_errorbar(
    aes(ymin = mean_ncrit - se_ncrit, ymax = mean_ncrit + se_ncrit),
    width = 0.22, linewidth = 0.5, colour = "grey20"
  ) +
  annotate("segment",
           x = 4, xend = 1,
           y = lex1_ncrit + 4, yend = lex2_ncrit + lex2_seN + 1.5,
           arrow = arrow(length = unit(2, "mm"), type = "closed"),
           colour = "grey25", linewidth = 0.4) +
  annotate("text",
           x = 2.5, y = max(float_agg$mean_ncrit) + 6,
           label = "more critical ops",
           size = 3.0, colour = "grey15", fontface = "italic") +
  scale_fill_manual(values = OP_COLORS) +
  scale_y_continuous(
    name   = expression("Critical ops in "*G^"+"*"("*sigma*")  [mean]"),
    expand = expansion(mult = c(0.02, 0.18))
  ) +
  xlab("Ranking operator") +
  ijsp_theme() +
  theme(legend.position = "none")

pc <- ggplot(float_agg, aes(x = op_f, y = mean_float, fill = op_f)) +
  geom_col(width = 0.65, colour = "grey30", linewidth = 0.3) +
  geom_errorbar(
    aes(ymin = mean_float - se_float, ymax = mean_float + se_float),
    width = 0.22, linewidth = 0.5, colour = "grey20"
  ) +
  annotate("segment",
           x = 4, xend = 1,
           y = lex1_float + 2.5, yend = lex2_float + lex2_seF + 1.0,
           arrow = arrow(length = unit(2, "mm"), type = "closed"),
           colour = "grey25", linewidth = 0.4) +
  annotate("text",
           x = 2.5, y = max(float_agg$mean_float) + 4,
           label = "less slack per op",
           size = 3.0, colour = "grey15", fontface = "italic") +
  scale_fill_manual(values = OP_COLORS) +
  scale_y_continuous(
    name   = expression("Mean float in "*G^"+"*"("*sigma*")  [time units]"),
    expand = expansion(mult = c(0.02, 0.18))
  ) +
  xlab("Ranking operator") +
  ijsp_theme() +
  theme(legend.position = "none")

# ---- Combine: 3 panels side by side, each 1/3 of the width ------------------
p_combined <- (pa | pb | pc) +
  plot_annotation(
    tag_levels  = "a",
    tag_prefix  = "(",
    tag_suffix  = ")",
    theme       = theme(
      plot.tag    = element_text(size = BASE_PT, face = "bold"),
      plot.margin = margin(0, 0, 0, 0, "mm")
    )
  ) +
  plot_layout(widths = c(1, 1, 1), guides = "collect") &
  theme(plot.margin = margin(0, 1, 0, 1, "mm"))

# ---- Save --------------------------------------------------------------------
fpath <- file.path(OUT_DIR, "fig34_combined.pdf")
cairo_pdf(filename = fpath, width = W_FULL, height = 60 * MM2IN,
          family = "sans", antialias = "subpixel")
print(p_combined)
dev.off()
cat(sprintf("[OK] %s\n", fpath))
