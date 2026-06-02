#!/usr/bin/env Rscript
# =============================================================================
# generate_fig_taillard_compar.R
# Per-class grouped bar chart for Phase C Taillard comparison
# (Proposal A — chosen design, restyled to match the rest of the paper).
#
# Inherits the journal style used by fig5_combined and fig6_convergence:
#   - ijsp_theme() (theme_bw + grey88 gridlines + sans 9pt)
#   - Okabe-Ito palette: TS-N_2 = #E69F00 (orange), TS-N_8 = #D55E00 (vermillion)
#   - cairo_pdf, 160 mm full text width, subpixel antialias
#
# Data are the per-class mean RE(%) on the 70 Taillard IJSP instances:
# fEABC and ESABC values are aggregated from the supplementary tables
# (supplementary/supp_taillard_tables.tex);  TS-N_2 and TS-N_8 values match
# tab_phaseb_groups (Phase B summary table in the main paper).
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
})

COR_DIR <- "C:/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu"
OUT_DIR <- file.path(COR_DIR, "figures")

MM2IN   <- 1 / 25.4
W_FULL  <- 160 * MM2IN
BASE_PT <- 9
AXIS_PT <- 8

# --- ijsp_theme (identical to combine_fig5.R) --------------------------------
ijsp_theme <- function(base = BASE_PT) {
  theme_bw(base_size = base, base_family = "sans") +
    theme(
      panel.grid.minor = element_blank(),
      panel.grid.major.x = element_blank(),
      panel.grid.major.y = element_line(colour = "grey88", linewidth = 0.3),
      panel.border     = element_rect(colour = "grey40", linewidth = 0.5),
      axis.title       = element_text(size = base),
      axis.text        = element_text(size = AXIS_PT),
      legend.position  = "top",
      legend.title     = element_blank(),
      legend.text      = element_text(size = AXIS_PT),
      legend.key.size  = unit(3, "mm"),
      legend.margin    = margin(0, 0, 0, 0),
      legend.box.margin= margin(0, 0, -3, 0),
      plot.margin      = margin(0, 1, 0, 1, "mm")
    )
}

# --- Per-class mean RE(%) data ----------------------------------------------
# Order: 15x15, 20x15, 20x20, 30x15, 30x20, 50x15, 50x20
classes <- c("15x15", "20x15", "20x20", "30x15", "30x20", "50x15", "50x20")
class_labels <- c(expression(15 %*% 15),
                  expression(20 %*% 15),
                  expression(20 %*% 20),
                  expression(30 %*% 15),
                  expression(30 %*% 20),
                  expression(50 %*% 15),
                  expression(50 %*% 20))

dat <- data.frame(
  class = factor(rep(classes, times = 4), levels = classes),
  alg   = factor(rep(c("fEABC", "ESABC", "TS-N8", "TS-N2"), each = length(classes)),
                 levels = c("fEABC", "ESABC", "TS-N8", "TS-N2")),
  re    = c(
    # fEABC (DiazFEABC2023): computed from supplementary/supp_taillard_tables.tex
    5.87, 8.67, 10.27,  9.82, 16.38, 5.71, 9.01,
    # ESABC (DiazICAE2023): computed from the same supplementary
    5.86, 8.00,  9.85,  9.09, 15.32, 4.55, 7.91,
    # TS-N_8 (this work): matches tab_phaseb_groups
    2.05, 3.66,  5.52,  3.88,  8.78, 0.22, 2.11,
    # TS-N_2 (this work): matches tab_phaseb_groups
    2.04, 3.58,  5.34,  3.68,  8.62, 0.32, 2.03
  )
)

# Colors: re-use Okabe-Ito palette already used in fig5/fig6 for N_2 and N_8;
# grays for the prior-art ABC methods.
ALG_COLORS <- c(
  "fEABC"   = "#BDBDBD",
  "ESABC"   = "#7A7A7A",
  "TS-N8"   = "#D55E00",   # same as N8 in combine_fig5.R
  "TS-N2"   = "#E69F00"    # same as N2 in combine_fig5.R
)

# Custom legend labels with italic ABC names and math-style TS-N_k
ALG_LABELS <- c(
  "fEABC"   = expression(italic(fEABC)),
  "ESABC"   = expression(italic(ESABC)),
  "TS-N8"   = expression(TS-N[8]),
  "TS-N2"   = expression(TS-N[2])
)

p <- ggplot(dat, aes(x = class, y = re, fill = alg)) +
  geom_bar(stat = "identity",
           position = position_dodge(width = 0.78),
           width = 0.72,
           colour = "black",
           linewidth = 0.3) +
  scale_fill_manual(values = ALG_COLORS, labels = ALG_LABELS) +
  scale_x_discrete(labels = class_labels) +
  scale_y_continuous(name   = "mean RE [%]",
                     breaks = seq(0, 18, 3),
                     limits = c(0, 18),
                     expand = expansion(mult = c(0, 0.02))) +
  xlab(NULL) +
  guides(fill = guide_legend(nrow = 1, byrow = TRUE)) +
  ijsp_theme()

# Quick sanity check printed to stdout
cat("Per-class grand means (across 7 Taillard classes):\n")
print(dat %>% group_by(alg) %>%
        summarise(grand = mean(re), .groups = "drop"))

# Save
fpath <- file.path(OUT_DIR, "fig_taillard_compar.pdf")
cairo_pdf(filename = fpath, width = W_FULL, height = 60 * MM2IN,
          family = "sans", antialias = "subpixel")
print(p)
dev.off()
cat(sprintf("[OK] %s\n", fpath))
