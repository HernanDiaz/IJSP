#!/usr/bin/env Rscript
# =============================================================================
# combine_fig5.R  (COR_Tabu  —  Tabu Search version, exp4)
# Combined Figure 5 for the TS paper:
#   (a) Phase B RPD by neighbourhood (boxplot, irace-tuned TS)
#   (b) Quality vs runtime trade-off (scatter, N3 excluded)
# Layout: (a) 3/5 width, (b) 2/5 width
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
  library(scales)
  library(patchwork)
})

COR_DIR  <- "C:/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu"
OUT_DIR  <- file.path(COR_DIR, "figures")
DATA_EXP4 <- file.path(COR_DIR, "data/phaseB_ts/runs_data.csv")
GROUP_B   <- file.path(COR_DIR, "data/phaseB_ts/phaseB_group_summary.csv")

MM2IN   <- 1 / 25.4
W_FULL  <- 160 * MM2IN
BASE_PT <- 9
AXIS_PT <- 8

# TS ordering: best → worst  (N2 ≈ N8 >> N_ext >> N1 >> N3)
NEIGH_ORDER  <- c("N2", "N8", "N_ext", "N1", "N3")
NEIGH_COLORS <- c("N_ext"="#0072B2", "N1"="#009E73", "N2"="#E69F00",
                  "N8"="#D55E00",   "N3"="#CC79A7")
neigh_map <- c(n1="N1", n2="N2", n3="N3", n8="N8",
               "next"="N_ext", next.="N_ext", nh="N8")

ijsp_theme <- function(base = BASE_PT) {
  theme_bw(base_size = base, base_family = "sans") +
    theme(
      panel.grid.minor  = element_blank(),
      panel.grid.major  = element_line(colour = "grey88", linewidth = 0.3),
      panel.border      = element_rect(colour = "grey40", linewidth = 0.5),
      axis.title        = element_text(size = base),
      axis.text         = element_text(size = AXIS_PT),
      legend.position   = "none",
      plot.margin       = margin(0, 1, 0, 1, "mm")
    )
}

# ---- Panel (a): RPD boxplot by neighbourhood ---------------------------------
d4 <- read.csv(DATA_EXP4, stringsAsFactors = FALSE)
d4$neigh_f <- factor(neigh_map[d4$neigh], levels = NEIGH_ORDER)
d4 <- d4 %>%
  group_by(instance, run) %>%
  mutate(rpd_pct = (midpoint - min(midpoint)) / min(midpoint) * 100) %>%
  ungroup()

pa <- ggplot(d4, aes(x = neigh_f, y = rpd_pct, fill = neigh_f)) +
  geom_boxplot(
    outlier.size   = 0.4,
    outlier.alpha  = 0.35,
    outlier.colour = "grey50",
    linewidth      = 0.45,
    width          = 0.65
  ) +
  scale_fill_manual(values = NEIGH_COLORS) +
  scale_y_continuous(
    name   = "Per-instance RPD on midpoint [%]",
    labels = function(x) sprintf("%.1f", x),
    expand = expansion(mult = c(0.02, 0.06))
  ) +
  xlab("Neighbourhood") +
  ijsp_theme()

# ---- Panel (b): Quality vs runtime scatter (N3 excluded) --------------------
instance_to_group <- function(inst) {
  ifelse(grepl("^tai", inst),
         sub("(tai[0-9]+_[0-9]+)_.*", "\\1", inst),
         sub("F0\\.15\\.0\\.([a-z]+[0-9]+)_.*", "\\1", inst))
}

d4b <- d4 %>%
  mutate(group = instance_to_group(instance)) %>%
  filter(neigh_f != "N3")

# Mean RPD per (group, neighbourhood)
rpd_group <- d4b %>%
  group_by(group, neigh_f) %>%
  summarise(rpd_mean = mean(rpd_pct, na.rm = TRUE), .groups = "drop")

# Median runtime per (group, neighbourhood) from phaseB_group_summary
rt_group <- read.csv(GROUP_B, stringsAsFactors = FALSE) %>%
  mutate(neigh_f = neigh_map[neigh]) %>%
  filter(!is.na(neigh_f), neigh_f != "N3") %>%
  select(group, neigh_f, med_rt)

dg_joined <- inner_join(rpd_group, rt_group, by = c("group", "neigh_f"))
dg_joined$neigh_f <- factor(dg_joined$neigh_f, levels = NEIGH_ORDER)

qr <- dg_joined %>%
  group_by(neigh_f) %>%
  summarise(
    mean_rpd = mean(rpd_mean, na.rm = TRUE),
    se_rpd   = sd(rpd_mean,   na.rm = TRUE) / sqrt(n()),
    mean_rt  = mean(med_rt,   na.rm = TRUE),
    se_rt    = sd(med_rt,     na.rm = TRUE) / sqrt(n()),
    .groups  = "drop"
  )

cat("Panel (b) RPD check (TS ordering expected: N2 ≤ N8 < N_ext < N1):\n")
print(qr %>% select(neigh_f, mean_rpd, mean_rt) %>% arrange(mean_rpd))

SHAPES4 <- c("N2"=15, "N8"=18, "N_ext"=16, "N1"=17)

pb <- ggplot(qr, aes(x = mean_rt, y = mean_rpd,
                      colour = neigh_f, shape = neigh_f, label = neigh_f)) +
  geom_errorbar(aes(ymin = mean_rpd - se_rpd, ymax = mean_rpd + se_rpd),
                width = 0.4, linewidth = 0.6, alpha = 0.7) +
  geom_segment(aes(x = mean_rt - se_rt, xend = mean_rt + se_rt,
                   y = mean_rpd,        yend = mean_rpd),
               linewidth = 0.6, alpha = 0.7) +
  geom_point(size = 3.5) +
  geom_label(size = 2.8, fontface = "bold", linewidth = 0.25,
             vjust = -0.9, show.legend = FALSE) +
  scale_colour_manual(values = NEIGH_COLORS) +
  scale_shape_manual(values = SHAPES4) +
  scale_x_continuous(name = "Median runtime per run [s]") +
  scale_y_continuous(name = "Mean RPD on midpoint [%]") +
  ijsp_theme()

# ---- Combine: (a) 3/5 width, (b) 2/5 width ----------------------------------
p_combined <- (pa | pb) +
  plot_annotation(
    tag_levels = "a",
    tag_prefix = "(",
    tag_suffix = ")",
    theme = theme(
      plot.tag    = element_text(size = BASE_PT, face = "bold"),
      plot.margin = margin(0, 0, 0, 0, "mm")
    )
  ) +
  plot_layout(widths = c(3, 2)) &
  theme(plot.margin = margin(0, 1, 0, 1, "mm"))

# ---- Save -------------------------------------------------------------------
fpath <- file.path(OUT_DIR, "fig5_combined.pdf")
cairo_pdf(filename = fpath, width = W_FULL, height = 80 * MM2IN,
          family = "sans", antialias = "subpixel")
print(p_combined)
dev.off()
cat(sprintf("[OK] %s\n", fpath))
