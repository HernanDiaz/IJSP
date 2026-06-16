#!/usr/bin/env Rscript
# =============================================================================
# combine_fig5.R  (COR_Tabu  —  Tabu Search version, exp4)
# Combined Figure 5 for the TS paper:
#   (a) Phase B RE(%) by neighbourhood (boxplot, irace-tuned TS)
#   (b) Quality vs runtime trade-off (scatter, N3 excluded)
# Layout: (a) 3/5 width, (b) 2/5 width
# Metric: RE(%) = (E[Cmax] - LB) / LB * 100  (same as §6.6 and §6.7)
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

# ── LB lookup tables (deterministic published lower bounds) ───────────────────
CLASSICAL_LB <- c(
  abz7=656, abz8=645, abz9=661,
  ft10=930, ft20=1165,
  la21=1046, la24=935, la25=977,
  la27=1235, la29=1152, la38=1196, la40=1222
)
TAILLARD_LBS <- list(
  tai15_15 = c(1231,1244,1218,1175,1224,1238,1227,1217,1274,1241),
  tai20_15 = c(1357,1367,1342,1345,1339,1360,1462,1377,1332,1348),
  tai20_20 = c(1642,1561,1518,1644,1558,1591,1652,1603,1583,1528),
  tai30_15 = c(1764,1774,1788,1828,2007,1819,1771,1673,1795,1651),
  tai30_20 = c(1906,1884,1809,1948,1997,1957,1807,1912,1931,1833),
  tai50_15 = c(2760,2756,2717,2839,2679,2781,2943,2885,2655,2723),
  tai50_20 = c(2868,2869,2755,2702,2725,2845,2825,2784,3071,2995)
)

get_lb <- function(inst) {
  if (grepl("^F0\\.15\\.0\\.", inst)) {
    name <- sub("^F0\\.15\\.0\\.([a-z]+[0-9]+)_.*", "\\1", inst)
    lb <- CLASSICAL_LB[name]
    if (is.na(lb)) { warning("No LB for ", inst); return(NA_real_) }
    return(unname(lb))
  } else if (grepl("^tai", inst)) {
    group <- sub("^(tai[0-9]+_[0-9]+)_.*", "\\1", inst)
    idx   <- as.integer(sub("^tai[0-9]+_[0-9]+_(\\d+).*", "\\1", inst))
    lbs   <- TAILLARD_LBS[[group]]
    if (is.null(lbs) || idx < 1 || idx > length(lbs)) {
      warning("No LB for ", inst); return(NA_real_)
    }
    return(lbs[idx])
  }
  warning("Unknown instance format: ", inst)
  return(NA_real_)
}

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

# ---- Panel (a): RE(%) boxplot by neighbourhood --------------------------------
d4 <- read.csv(DATA_EXP4, stringsAsFactors = FALSE)
d4$neigh_f <- factor(neigh_map[d4$neigh], levels = NEIGH_ORDER)

# Compute LB for each instance and RE(%) = (midpoint - LB) / LB * 100
d4$lb     <- sapply(d4$instance, get_lb)
d4$re_pct <- (d4$midpoint - d4$lb) / d4$lb * 100

cat("RE(%) summary by neighbourhood:\n")
print(d4 %>% group_by(neigh_f) %>%
      summarise(mean=mean(re_pct, na.rm=TRUE),
                median=median(re_pct, na.rm=TRUE), .groups="drop"))

pa <- ggplot(d4, aes(x = neigh_f, y = re_pct, fill = neigh_f)) +
  geom_boxplot(
    outlier.size   = 0.4,
    outlier.alpha  = 0.35,
    outlier.colour = "grey50",
    linewidth      = 0.45,
    width          = 0.65
  ) +
  scale_fill_manual(values = NEIGH_COLORS) +
  scale_y_continuous(
    name   = "RE [%]",
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

# Mean RE(%) per (group, neighbourhood)
re_group <- d4b %>%
  group_by(group, neigh_f) %>%
  summarise(re_mean = mean(re_pct, na.rm = TRUE), .groups = "drop")

# Median runtime per (group, neighbourhood) from phaseB_group_summary
rt_group <- read.csv(GROUP_B, stringsAsFactors = FALSE) %>%
  mutate(neigh_f = neigh_map[neigh]) %>%
  filter(!is.na(neigh_f), neigh_f != "N3") %>%
  select(group, neigh_f, med_rt)

dg_joined <- inner_join(re_group, rt_group, by = c("group", "neigh_f"))
dg_joined$neigh_f <- factor(dg_joined$neigh_f, levels = NEIGH_ORDER)

qr <- dg_joined %>%
  group_by(neigh_f) %>%
  summarise(
    mean_re  = mean(re_mean, na.rm = TRUE),
    se_re    = sd(re_mean,   na.rm = TRUE) / sqrt(n()),
    mean_rt  = mean(med_rt,  na.rm = TRUE),
    se_rt    = sd(med_rt,    na.rm = TRUE) / sqrt(n()),
    .groups  = "drop"
  )

cat("\nPanel (b) RE(%) check (TS ordering expected: N2 <= N8 < N_ext < N1):\n")
print(qr %>% select(neigh_f, mean_re, mean_rt) %>% arrange(mean_re))

SHAPES4 <- c("N2"=15, "N8"=18, "N_ext"=16, "N1"=17)

pb <- ggplot(qr, aes(x = mean_rt, y = mean_re,
                      colour = neigh_f, shape = neigh_f, label = neigh_f)) +
  geom_errorbar(aes(ymin = mean_re - se_re, ymax = mean_re + se_re),
                width = 0.4, linewidth = 0.6, alpha = 0.7) +
  geom_segment(aes(x = mean_rt - se_rt, xend = mean_rt + se_rt,
                   y = mean_re,         yend = mean_re),
               linewidth = 0.6, alpha = 0.7) +
  geom_point(size = 3.5) +
  geom_label(size = 2.8, fontface = "bold", linewidth = 0.25,
             vjust = -0.9, show.legend = FALSE) +
  scale_colour_manual(values = NEIGH_COLORS) +
  scale_shape_manual(values = SHAPES4) +
  scale_x_continuous(name = "Median runtime per run [s]") +
  scale_y_continuous(name = "Mean RE [%]") +
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
