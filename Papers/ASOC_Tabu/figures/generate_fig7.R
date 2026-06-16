#!/usr/bin/env Rscript
# =============================================================================
# generate_fig7.R  (COR_Tabu — Tabu Search version, exp9)
# Figure 7: CRISP validation — RPD% vs BKS heatmap by neighbourhood × group
# Data:  experiments/statistical_results_exp9/rpd_by_group.csv
# Exp9:  82 instances (CRISP p-=p+), irace-tuned TS from Phase B
# RPD = (best_of_30_makespan − BKS) / BKS × 100
# Layout: full Elsevier column width (160 mm), height 130 mm
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
  library(scales)
})

BASE_DIR <- "C:/Users/diazhernan/CLionProjects/IJSP"
COR_DIR  <- file.path(BASE_DIR, "Papers/COR_Tabu")
OUT_DIR  <- file.path(COR_DIR, "figures")
DATA_RPD <- file.path(BASE_DIR,
               "experiments/statistical_results_exp9/rpd_by_group.csv")

MM2IN   <- 1 / 25.4
W_FULL  <- 160 * MM2IN
H_FIG7  <- 130 * MM2IN
BASE_PT <- 9
AXIS_PT <- 8

# TS neighbourhood ordering: best → worst
NEIGH_ORDER  <- c("N2", "N8", "N_ext", "N1", "N3")
NEIGH_COLORS <- c("N_ext" = "#0072B2", "N1" = "#009E73",
                  "N2"    = "#E69F00", "N8" = "#D55E00", "N3" = "#CC79A7")

neigh_map <- c(n1 = "N1", n2 = "N2", n3 = "N3", n8 = "N8",
               "next" = "N_ext", next. = "N_ext", nh = "N8")

# Instance group ordering (small classical → large Taillard)
group_order <- c(
  "ft10", "ft20",
  "abz7", "abz8", "abz9",
  "la21", "la24", "la25", "la27", "la29", "la38", "la40",
  "tai15_15", "tai20_15", "tai20_20",
  "tai30_15", "tai30_20",
  "tai50_15", "tai50_20"
)

group_labels <- c(
  "ft10"     = "ft10 (10×10)",
  "ft20"     = "ft20 (20×5)",
  "abz7"     = "abz7 (10×10)",
  "abz8"     = "abz8 (10×10)",
  "abz9"     = "abz9 (10×10)",
  "la21"     = "la21 (15×4)",
  "la24"     = "la24 (15×5)",
  "la25"     = "la25 (15×5)",
  "la27"     = "la27 (20×10)",
  "la29"     = "la29 (20×10)",
  "la38"     = "la38 (15×15)",
  "la40"     = "la40 (15×15)",
  "tai15_15" = "tai (15×15) avg",
  "tai20_15" = "tai (20×15) avg",
  "tai20_20" = "tai (20×20) avg",
  "tai30_15" = "tai (30×15) avg",
  "tai30_20" = "tai (30×20) avg",
  "tai50_15" = "tai (50×15) avg",
  "tai50_20" = "tai (50×20) avg"
)

ijsp_theme <- function(base = BASE_PT) {
  theme_bw(base_size = base, base_family = "sans") +
    theme(
      panel.grid.minor = element_blank(),
      panel.grid.major = element_line(colour = "grey88", linewidth = 0.3),
      panel.border     = element_rect(colour = "grey40", linewidth = 0.5),
      axis.title       = element_text(size = base),
      axis.text        = element_text(size = AXIS_PT),
      plot.margin      = margin(1, 1, 1, 1, "mm")
    )
}

# ── Load and prepare data ──────────────────────────────────────────────────────
dg <- read.csv(DATA_RPD, stringsAsFactors = FALSE)

# Map raw neigh codes → display labels
dg$neigh_f <- factor(neigh_map[dg$neigh], levels = NEIGH_ORDER)

dg_plot <- dg %>%
  filter(group %in% group_order, !is.na(neigh_f)) %>%
  mutate(
    group_f = factor(group, levels = rev(group_order)),
    label_f = factor(group_labels[as.character(group_f)],
                     levels = rev(group_labels[group_order]))
  )

# Print data check
cat("=== Fig 7 data check (mean RPD by neigh) ===\n")
dg_plot %>%
  group_by(neigh_f) %>%
  summarise(mean_rpd = mean(rpd_mean, na.rm = TRUE)) %>%
  arrange(mean_rpd) %>%
  print()

# ── Build heatmap ─────────────────────────────────────────────────────────────
RPD_LIM <- c(0, 15)   # TS has slightly higher RPD on harder instances

p7 <- ggplot(dg_plot, aes(x = neigh_f, y = group_f, fill = rpd_mean)) +
  geom_tile(colour = "white", linewidth = 0.5) +
  geom_text(
    aes(label = sprintf("%.1f", rpd_mean)),
    size   = 2.3,
    colour = ifelse(dg_plot$rpd_mean > 7.5, "white", "grey15")
  ) +
  scale_fill_gradientn(
    name    = "RPD (%)",
    colours = c("#1a9850", "#91cf60", "#d9ef8b", "#fee08b", "#fc8d59", "#d73027"),
    values  = rescale(c(0, 2.5, 5, 7.5, 11, 15)),
    limits  = RPD_LIM,
    oob     = squish,
    guide   = guide_colourbar(
      barwidth       = unit(3.5, "cm"),
      barheight      = unit(0.35, "cm"),
      title.position = "top",
      title.hjust    = 0.5
    )
  ) +
  # Group separator lines: after ft-group(2), after abz-group(5), after la-group(12),
  # after tai_small(14), after tai_medium(16)
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

# ── Save ──────────────────────────────────────────────────────────────────────
fpath <- file.path(OUT_DIR, "fig7_crisp_rpd_heatmap.pdf")
cairo_pdf(filename = fpath, width = W_FULL, height = H_FIG7,
          family = "sans", antialias = "subpixel")
print(p7)
dev.off()
cat(sprintf("[OK] %s\n", fpath))
