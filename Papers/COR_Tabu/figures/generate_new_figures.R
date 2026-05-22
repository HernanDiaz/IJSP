#!/usr/bin/env Rscript
# =============================================================================
# generate_new_figures.R  —  6 new figures for IJSP paper
# =============================================================================

suppressPackageStartupMessages({
  library(ggplot2)
  library(dplyr)
  library(tidyr)
  library(scales)
  library(patchwork)
})

COR_DIR   <- "C:/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu"
OUT_DIR   <- file.path(COR_DIR, "figures")
FLOATS    <- file.path(COR_DIR, "data/phaseA/floats_data.csv")
RUNS_B    <- file.path(COR_DIR, "data/phaseB_ts/runs_data.csv")     # Exp4: irace-tuned TS
ALL_RES   <- file.path(COR_DIR, "data/phaseA/all_results_full.csv")
GROUP_B   <- file.path(COR_DIR, "data/phaseB_ts/phaseB_group_summary.csv")

MM2IN    <- 1 / 25.4
W_FULL   <- 160 * MM2IN
W_HALF   <- 90  * MM2IN
BASE_PT  <- 9
AXIS_PT  <- 8

NEIGH_ORDER   <- c("N_ext", "N1", "N2", "N8", "N3")   # Phase A (HC exp1)
NEIGH_ORDER_B <- c("N2", "N8", "N_ext", "N1", "N3")   # Phase B TS (exp4): best→worst
NEIGH_COLORS  <- c("N_ext"="#0072B2","N1"="#009E73","N2"="#E69F00",
                   "N8"="#D55E00","N3"="#CC79A7")
OP_ORDER  <- c("LEX2","YX","EV","LEX1")
OP_COLORS <- c("LEX2"="#0072B2","YX"="#009E73","EV"="#E69F00","LEX1"="#D55E00")

neigh_map <- c(n1="N1", n2="N2", n3="N3", n8="N8",
               "next"="N_ext", next.="N_ext", nh="N8")

ijsp_theme <- function(base=BASE_PT) {
  theme_bw(base_size=base, base_family="sans") +
    theme(
      panel.grid.minor    = element_blank(),
      panel.grid.major    = element_line(colour="grey88", linewidth=0.3),
      panel.border        = element_rect(colour="grey40", linewidth=0.5),
      strip.background    = element_rect(fill="grey92", colour="grey50",
                                         linewidth=0.4),
      strip.text          = element_text(size=base, face="bold"),
      axis.title          = element_text(size=base),
      axis.text           = element_text(size=AXIS_PT),
      legend.text         = element_text(size=AXIS_PT),
      legend.title        = element_text(size=base, face="bold"),
      legend.key.size     = unit(0.35,"cm"),
      legend.background   = element_rect(fill="white", colour="grey70",
                                         linewidth=0.3),
      plot.margin         = margin(3,4,2,4,"mm")
    )
}

save_fig <- function(p, name, w=W_FULL, h=90*MM2IN) {
  fpath <- file.path(OUT_DIR, paste0(name, ".pdf"))
  cairo_pdf(filename=fpath, width=w, height=h, family="sans",
            antialias="subpixel")
  print(p)
  dev.off()
  cat(sprintf("  [OK] %s\n", basename(fpath)))
}

# ---- Load Phase A floats data ------------------------------------------------
df <- read.csv(FLOATS, stringsAsFactors=FALSE)
df$neigh_f <- factor(neigh_map[df$neigh], levels=NEIGH_ORDER)
df$op_f    <- factor(df$comp, levels=OP_ORDER)

# =============================================================================
# FIG A1 — Heatmap 5x4: mean interval width by neighbourhood × operator
# =============================================================================
cat("=== Fig A1: Heatmap neigh x operator ===\n")

heat <- df %>%
  group_by(neigh_f, op_f) %>%
  summarise(mean_w = mean(width, na.rm=TRUE),
            sd_w   = sd(width,   na.rm=TRUE),
            .groups="drop") %>%
  filter(!is.na(neigh_f))

# Row-normalise: % deviation from row mean (to show operator effect within each neigh)
heat <- heat %>%
  group_by(neigh_f) %>%
  mutate(row_mean = mean(mean_w),
         dev_pct  = (mean_w - row_mean) / row_mean * 100) %>%
  ungroup()

p_a1 <- ggplot(heat, aes(x=op_f, y=neigh_f, fill=mean_w)) +
  geom_tile(colour="white", linewidth=0.8) +
  geom_text(aes(label=sprintf("%.1f\n(%+.1f%%)", mean_w, dev_pct)),
            size=2.5, lineheight=1.2, colour="grey10") +
  scale_fill_gradientn(
    name    = "Mean width",
    colours = c("#1a9850","#91cf60","#d9ef8b","#fee08b","#fc8d59","#d73027"),
    values  = rescale(c(240,255,265,275,285,310)),
    limits  = c(240,310),
    oob     = squish,
    guide   = guide_colourbar(barwidth=unit(4,"cm"), barheight=unit(0.35,"cm"),
                              title.position="top", title.hjust=0.5)
  ) +
  scale_x_discrete(name="Ranking operator", position="bottom") +
  scale_y_discrete(name="Neighbourhood") +
  labs(title=expression("Phase A — Mean makespan interval width  ["*C[max]^"+"*" − "*C[max]^"−"*"]")) +
  ijsp_theme() +
  theme(legend.position="top",
        legend.margin=margin(0,0,-1,0,"mm"),
        panel.grid=element_blank(),
        plot.title=element_text(size=BASE_PT, face="bold"))

save_fig(p_a1, "figA1_width_heatmap_neigh_op",
         w=W_HALF, h=80*MM2IN)

# =============================================================================
# FIG A2 — Decomposition C⁻ / C⁺ by operator
# Key: LEX2 narrow because it reduces C⁺, not because it raises C⁻
# =============================================================================
cat("=== Fig A2: C- / C+ decomposition by operator ===\n")

decomp <- df %>%
  group_by(op_f) %>%
  summarise(
    m_minus   = mean(cmax_m, na.rm=TRUE),
    se_minus  = sd(cmax_m,   na.rm=TRUE) / sqrt(n()),
    m_plus    = mean(cmax_p, na.rm=TRUE),
    se_plus   = sd(cmax_p,   na.rm=TRUE) / sqrt(n()),
    m_width   = mean(width,  na.rm=TRUE),
    .groups   = "drop"
  )

# Long format for faceting
decomp_long <- decomp %>%
  pivot_longer(cols=c(m_minus, m_plus),
               names_to="component", values_to="mean_val") %>%
  mutate(
    se_val    = ifelse(component=="m_minus", se_minus, se_plus),
    comp_label = factor(ifelse(component=="m_minus",
                               expression(C[max]^"-"),
                               expression(C[max]^"+")),
                        levels=c(expression(C[max]^"-"),
                                 expression(C[max]^"+")))
  )

decomp_long$comp_label <- factor(
  decomp_long$component,
  levels=c("m_minus","m_plus"),
  labels=c("C[max]^{'-'}~(best~scenario)", "C[max]^{'+'}~(worst~scenario)")
)

p_a2 <- ggplot(decomp_long, aes(x=op_f, y=mean_val, colour=op_f, shape=op_f)) +
  geom_errorbar(aes(ymin=mean_val - 2*se_val, ymax=mean_val + 2*se_val),
                width=0.22, linewidth=0.6) +
  geom_point(size=3.2) +
  facet_wrap(~comp_label, scales="free_y",
             labeller=label_parsed) +
  scale_colour_manual(values=OP_COLORS, name="Operator") +
  scale_shape_manual(values=c(16,17,15,4), name="Operator") +
  scale_y_continuous(labels=comma) +
  labs(
    x     = "Ranking operator",
    y     = "Mean makespan [time units]",
    title = expression("Phase A — Decomposition: "*C[max]^"-"*" (best) vs "*
                       C[max]^"+"*" (worst) by operator")
  ) +
  ijsp_theme() +
  theme(legend.position="none",
        plot.title=element_text(size=BASE_PT, face="bold"))

save_fig(p_a2, "figA2_cmin_cmax_decomposition",
         w=W_FULL, h=80*MM2IN)

# =============================================================================
# FIG A3 — Scatter pareado: LEX2 vs cada operador  (Phase A)
# Each point = one (instance, neigh) pair; diagonal = equality
# =============================================================================
cat("=== Fig A3: Paired scatter LEX2 vs others ===\n")

# Pivot wider: one row per (instance, neigh), one column per operator
wide <- df %>%
  select(instance, neigh_f, op_f, width) %>%
  filter(!is.na(neigh_f)) %>%
  pivot_wider(names_from=op_f, values_from=width)

# Helper: one panel
scatter_panel <- function(data, xcol, ycol="LEX2",
                          xlab, ylab="LEX2 width", subtitle) {
  d <- data %>% filter(!is.na(.data[[xcol]]), !is.na(.data[[ycol]]))
  lims <- range(c(d[[xcol]], d[[ycol]]), na.rm=TRUE)
  pct_below <- mean(d[[ycol]] < d[[xcol]], na.rm=TRUE) * 100
  ggplot(d, aes(x=.data[[xcol]], y=.data[[ycol]], colour=neigh_f)) +
    geom_abline(slope=1, intercept=0, colour="grey60", linewidth=0.5,
                linetype="dashed") +
    geom_point(size=0.8, alpha=0.55) +
    scale_colour_manual(values=NEIGH_COLORS, name="Neighbourhood") +
    labs(x=xlab, y=ylab, subtitle=subtitle) +
    scale_x_continuous(labels=comma, limits=lims, expand=expansion(mult=0.02)) +
    scale_y_continuous(labels=comma, limits=lims, expand=expansion(mult=0.02)) +
    ijsp_theme() +
    theme(legend.position="none",
          plot.subtitle=element_text(size=AXIS_PT, face="bold"),
          plot.margin=margin(1, 2, 1, 2, "mm"))
}

p_lex2_lex1 <- scatter_panel(wide, "LEX1", xlab="LEX1 width",
                              subtitle="LEX2 vs LEX1")
p_lex2_ev   <- scatter_panel(wide, "EV",   xlab="EV width",
                              subtitle="LEX2 vs EV")
p_lex2_yx   <- scatter_panel(wide, "YX",   xlab="YX width",
                              subtitle="LEX2 vs YX")

p_a3 <- (p_lex2_lex1 | p_lex2_ev | p_lex2_yx) &
  theme(plot.margin=margin(1, 2, 1, 2, "mm"))

save_fig(p_a3, "figA3_paired_scatter_lex2",
         w=W_FULL, h=60*MM2IN)

# =============================================================================
# FIG B1 — Parallel coordinates: RPD per instance across 5 neighbourhoods
# Phase B (irace-tuned HC); colour = instance size group
# =============================================================================
cat("=== Fig B1: Parallel coordinates Phase B ===\n")

db <- read.csv(RUNS_B, stringsAsFactors=FALSE)
db$neigh_f <- factor(neigh_map[db$neigh], levels=NEIGH_ORDER_B)

# Best of 30 per (instance, neigh)
best_b <- db %>%
  group_by(instance, neigh_f) %>%
  summarise(best_mid = min(midpoint, na.rm=TRUE), .groups="drop") %>%
  filter(!is.na(neigh_f))

# Per-instance RPD: reference = minimum across all 5 neigh
best_b <- best_b %>%
  group_by(instance) %>%
  mutate(ref  = min(best_mid),
         rpd  = (best_mid - ref) / ref * 100) %>%
  ungroup()

# Instance size group from name
best_b <- best_b %>%
  mutate(group = case_when(
    grepl("tai50",  instance) ~ "tai 50×",
    grepl("tai30",  instance) ~ "tai 30×",
    grepl("tai20",  instance) ~ "tai 20×",
    grepl("tai15",  instance) ~ "tai 15×15",
    grepl("abz|ft", instance) ~ "small classic",
    grepl("la",     instance) ~ "la series",
    TRUE ~ "other"
  ))

group_colors <- c(
  "small classic" = "#0072B2",
  "la series"     = "#009E73",
  "tai 15×15"     = "#E69F00",
  "tai 20×"       = "#D55E00",
  "tai 30×"       = "#CC79A7",
  "tai 50×"       = "#56B4E9"
)

# Parallel coordinates: one line per instance
p_b1 <- ggplot(best_b, aes(x=neigh_f, y=rpd,
                             group=instance, colour=group)) +
  geom_line(linewidth=0.35, alpha=0.55) +
  geom_point(size=0.9, alpha=0.7) +
  scale_colour_manual(values=group_colors, name="Instance group") +
  scale_y_continuous(
    name   = "Per-instance RPD on midpoint [%]",
    limits = c(-0.1, NA),
    expand = expansion(mult=c(0, 0.06))
  ) +
  scale_x_discrete(name="Neighbourhood (ordered by median RPD)") +
  labs(title="Phase B — Per-instance RPD across 5 neighbourhoods (irace-tuned TS)",
       subtitle="Each line = one instance; colour = instance size group") +
  ijsp_theme() +
  theme(legend.position="right",
        plot.title   = element_text(size=BASE_PT, face="bold"),
        plot.subtitle= element_text(size=AXIS_PT))

save_fig(p_b1, "figB1_parallel_coords_phaseB",
         w=W_FULL, h=95*MM2IN)

# =============================================================================
# FIG B2 — Quality vs Runtime scatter (Phase B, by neighbourhood)
# =============================================================================
cat("=== Fig B2: Quality vs Runtime Phase B ===\n")

dg <- read.csv(GROUP_B, stringsAsFactors=FALSE)
dg$neigh_f <- factor(neigh_map[dg$neigh], levels=NEIGH_ORDER_B)
dg <- dg %>% filter(!is.na(neigh_f))

# Aggregate by neighbourhood (over all groups)
qr <- dg %>%
  group_by(neigh_f) %>%
  summarise(
    mean_rpd = mean(rpd_mean, na.rm=TRUE),
    se_rpd   = sd(rpd_mean,   na.rm=TRUE) / sqrt(n()),
    mean_rt  = mean(med_rt,   na.rm=TRUE),
    se_rt    = sd(med_rt,     na.rm=TRUE) / sqrt(n()),
    .groups  = "drop"
  )

p_b2 <- ggplot(qr, aes(x=mean_rt, y=mean_rpd,
                         colour=neigh_f, shape=neigh_f, label=neigh_f)) +
  geom_errorbar(aes(ymin=mean_rpd - se_rpd, ymax=mean_rpd + se_rpd),
                width=0.4, linewidth=0.6, alpha=0.7) +
  geom_errorbarh(aes(xmin=mean_rt - se_rt, xmax=mean_rt + se_rt),
                 height=0.04, linewidth=0.6, alpha=0.7) +
  geom_point(size=4) +
  geom_label(
    size=2.8, fontface="bold", label.size=0.25,
    vjust=-0.9, show.legend=FALSE
  ) +
  scale_colour_manual(values=NEIGH_COLORS, name="Neighbourhood") +
  scale_shape_manual(values=c(16,17,15,18,4), name="Neighbourhood") +
  scale_x_continuous(name="Median runtime per run [s]") +
  scale_y_continuous(name="Mean RPD on midpoint [%]") +
  labs(title="Phase B — Quality vs Runtime trade-off by neighbourhood (irace-tuned TS)",
       subtitle="Lower-left = faster AND better  |  error bars = ±1 SE across instance groups") +
  ijsp_theme() +
  theme(legend.position="none",
        plot.title   = element_text(size=BASE_PT, face="bold"),
        plot.subtitle= element_text(size=AXIS_PT))

save_fig(p_b2, "figB2_quality_vs_runtime",
         w=W_HALF + 20*MM2IN, h=90*MM2IN)

# =============================================================================
# FIG B3 — Dominance matrix: % instances where row beats column (Phase B)
# =============================================================================
cat("=== Fig B3: Dominance matrix Phase B ===\n")

# Pivot: one row per instance, columns = neigh RPD
dom_wide <- best_b %>%
  select(instance, neigh_f, rpd) %>%
  pivot_wider(names_from=neigh_f, values_from=rpd)

neigh_cols <- NEIGH_ORDER_B[NEIGH_ORDER_B %in% names(dom_wide)]

# Build pairwise dominance
pairs <- expand.grid(row=neigh_cols, col=neigh_cols, stringsAsFactors=FALSE)
pairs$pct_row_wins <- mapply(function(r, c) {
  if (r == c) return(NA_real_)
  rv <- dom_wide[[r]]
  cv <- dom_wide[[c]]
  ok <- !is.na(rv) & !is.na(cv)
  mean(rv[ok] < cv[ok]) * 100
}, pairs$row, pairs$col)

pairs$row_f <- factor(pairs$row, levels=NEIGH_ORDER_B)
pairs$col_f <- factor(pairs$col, levels=NEIGH_ORDER_B)

# Text colour: white when far from 50%
pairs$txt_col <- ifelse(is.na(pairs$pct_row_wins), "white",
                        ifelse(abs(pairs$pct_row_wins - 50) > 25, "white", "grey15"))

p_b3 <- ggplot(pairs, aes(x=col_f, y=row_f, fill=pct_row_wins)) +
  geom_tile(colour="white", linewidth=0.8) +
  geom_text(aes(label=ifelse(is.na(pct_row_wins), "—",
                             sprintf("%.0f%%", pct_row_wins)),
                colour=txt_col),
            size=3, fontface="bold") +
  scale_colour_identity() +
  scale_fill_gradientn(
    name    = "% instances\nrow beats col",
    colours = c("#d73027","#fc8d59","#fee090","#e0f3f8","#74add1","#313695"),
    values  = rescale(c(0,30,45,55,70,100)),
    limits  = c(0,100),
    guide   = guide_colourbar(barwidth=unit(3.5,"cm"), barheight=unit(0.35,"cm"),
                              title.position="top", title.hjust=0.5)
  ) +
  geom_tile(data=pairs[is.na(pairs$pct_row_wins),],
            aes(x=col_f, y=row_f), fill="grey85", colour="white", linewidth=0.8) +
  scale_x_discrete(name="Column neighbourhood (opponent)", position="bottom") +
  scale_y_discrete(name="Row neighbourhood") +
  labs(title="Phase B — Pairwise dominance: % of instances where row neighbourhood beats column",
       subtitle="Values > 50%: row wins more often  |  diagonal = self-comparison") +
  ijsp_theme() +
  theme(legend.position="top",
        legend.margin=margin(0,0,-1,0,"mm"),
        panel.grid=element_blank(),
        plot.title   = element_text(size=BASE_PT, face="bold"),
        plot.subtitle= element_text(size=AXIS_PT))

save_fig(p_b3, "figB3_dominance_matrix",
         w=W_HALF + 10*MM2IN, h=90*MM2IN)

# =============================================================================
# FIG B2b — Quality vs Runtime (Phase B), N3 excluded, no title, no margins
# =============================================================================
cat("=== Fig B2b: Quality vs Runtime (no N3) ===\n")

dg2 <- read.csv(GROUP_B, stringsAsFactors=FALSE)
dg2$neigh_f <- factor(neigh_map[dg2$neigh], levels=NEIGH_ORDER_B)
dg2 <- dg2 %>% filter(!is.na(neigh_f), neigh_f != "N3")

qr2 <- dg2 %>%
  group_by(neigh_f) %>%
  summarise(
    mean_rpd = mean(rpd_mean, na.rm=TRUE),
    se_rpd   = sd(rpd_mean,   na.rm=TRUE) / sqrt(n()),
    mean_rt  = mean(med_rt,   na.rm=TRUE),
    se_rt    = sd(med_rt,     na.rm=TRUE) / sqrt(n()),
    .groups  = "drop"
  )

NEIGH_SHAPES4 <- c("N_ext"=16, "N1"=17, "N2"=15, "N8"=18)

p_b2b <- ggplot(qr2, aes(x=mean_rt, y=mean_rpd,
                           colour=neigh_f, shape=neigh_f, label=neigh_f)) +
  geom_errorbar(aes(ymin=mean_rpd - se_rpd, ymax=mean_rpd + se_rpd),
                width=0.4, linewidth=0.6, alpha=0.7) +
  geom_segment(aes(x=mean_rt - se_rt, xend=mean_rt + se_rt,
                   y=mean_rpd, yend=mean_rpd),
               linewidth=0.6, alpha=0.7) +
  geom_point(size=4) +
  geom_label(size=2.8, fontface="bold", linewidth=0.25,
             vjust=-0.9, show.legend=FALSE) +
  scale_colour_manual(values=NEIGH_COLORS, name="Neighbourhood") +
  scale_shape_manual(values=NEIGH_SHAPES4, name="Neighbourhood") +
  scale_x_continuous(name="Median runtime per run [s]") +
  scale_y_continuous(name="Mean RPD on midpoint [%]") +
  ijsp_theme() +
  theme(legend.position="none",
        plot.margin=margin(0, 0, 0, 0, "mm"))

save_fig(p_b2b, "figB2_quality_vs_runtime_no_n3",
         w=W_HALF + 20*MM2IN, h=75*MM2IN)

cat("\nDone. All new figures saved to:", OUT_DIR, "\n")
