# Figuras y tablas del experimento final para un algoritmo.
# Uso: Rscript fig_tables.R <algo>
#   Entradas: final/phase2/results_<algo>.csv   (E[Cmax] por run)
#             final/phase2/anytime_<algo>.csv   (curvas remuestreadas, opcional)
#             final/refs.csv                    (best_known por instancia)
#   Salidas:  final/figs/<algo>_rpd_by_class.png     (RPD medio por clase y brazo)
#             final/figs/<algo>_anytime_<clase>.png  (curvas anytime por clase)
#             final/tables/<algo>_by_class.csv       (tabla resumen)
args <- commandArgs(trailingOnly = TRUE)
algo <- args[1]
dir.create("final/figs", showWarnings = FALSE, recursive = TRUE)
dir.create("final/tables", showWarnings = FALSE, recursive = TRUE)

d    <- read.csv(sprintf("final/phase2/results_%s.csv", algo), stringsAsFactors = FALSE)
# RPD contra la COTA INFERIOR publicada (final/ta_lb.csv), no el best_known blando.
refs <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst", "lb")]
names(refs)[2] <- "best_known"
d <- merge(d, refs, by = "inst")
d$rpd <- 100 * (d$ecmax - d$best_known) / d$best_known

arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
labs <- c(A0="aleatorio", V2H="v2@50", V2="v2@100", MOR="MOR", GT="GT", GP="GP", MIX="mezcla")
cols <- c(A0="black", V2H="#d62728", V2="#8c1515", MOR="#ff7f0e", GT="#2ca02c", GP="#1f77b4", MIX="#9467bd")
clases <- c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")
clases <- clases[clases %in% unique(d$clase)]

# ---- tabla: RPD medio por clase y brazo, y delta vs A0 ----
tab <- data.frame()
for (cl in clases) {
  fila <- list(clase = cl, n_inst = length(unique(d$inst[d$clase == cl])))
  for (ar in arms) {
    v <- d$rpd[d$clase == cl & d$arm == ar]
    fila[[paste0("rpd_", ar)]] <- if (length(v)) round(mean(v), 2) else NA
  }
  a0 <- fila[["rpd_A0"]]
  for (ar in setdiff(arms, "A0")) fila[[paste0("d_", ar)]] <- round(fila[[paste0("rpd_", ar)]] - a0, 2)
  tab <- rbind(tab, as.data.frame(fila, stringsAsFactors = FALSE))
}
write.csv(tab, sprintf("final/tables/%s_by_class.csv", algo), row.names = FALSE)
cat("=== RPD medio (%) por clase ===\n")
print(tab[, c("clase","n_inst", paste0("rpd_", arms))], row.names = FALSE)
cat("\n=== delta de RPD vs control (negativo = la siembra mejora) ===\n")
print(tab[, c("clase", paste0("d_", setdiff(arms,"A0")))], row.names = FALSE)

# ---- figura 1: RPD medio por clase (barras agrupadas) ----
png(sprintf("final/figs/%s_rpd_by_class.png", algo), width = 1250, height = 700, res = 130)
par(mar = c(4.5, 4.5, 3, 8), xpd = FALSE)
m <- t(as.matrix(tab[, paste0("rpd_", arms)]))
colnames(m) <- tab$clase
barplot(m, beside = TRUE, col = cols[arms], border = NA,
        ylab = "RPD medio (%) vs mejor conocida", xlab = "clase de instancia",
        main = sprintf("%s — calidad final por clase y estrategia de siembra", toupper(algo)))
par(xpd = TRUE)
legend("topright", inset = c(-0.13, 0), legend = labs[arms], fill = cols[arms], border = NA, bty = "n", cex = 0.8)
dev.off()

# ---- figura 2: curvas anytime por clase (si hay datos remuestreados) ----
af <- sprintf("final/phase2/anytime_%s.csv", algo)
if (file.exists(af)) {
  a <- read.csv(af, stringsAsFactors = FALSE)
  a <- merge(a, refs, by = "inst")
  a$rpd <- 100 * (a$bestcmax - a$best_known) / a$best_known
  cl_of <- unique(d[, c("inst","clase")])
  a <- merge(a, cl_of, by = "inst")
  for (cl in clases) {
    sub <- a[a$clase == cl, ]
    if (!nrow(sub)) next
    # CORRECCION de sesgo de supervivencia: cada (instancia, brazo) se prolonga
    # hasta el t maximo de la clase con su ultimo valor (al terminar el run, su
    # mejor solucion persiste). Sin esto, las instancias que acaban antes salen
    # del promedio y la curva media sube artificialmente al final.
    tmax <- max(sub$t)
    grid <- sort(unique(sub$t))
    ext <- do.call(rbind, lapply(split(sub, list(sub$inst, sub$arm), drop = TRUE), function(g) {
      g <- g[order(g$t), ]
      falta <- grid[grid > max(g$t)]
      if (!length(falta)) return(g[, c("inst","arm","t","rpd")])
      rbind(g[, c("inst","arm","t","rpd")],
            data.frame(inst = g$inst[1], arm = g$arm[1], t = falta,
                       rpd = g$rpd[nrow(g)]))
    }))
    png(sprintf("final/figs/%s_anytime_%s.png", algo, cl), width = 1100, height = 700, res = 130)
    par(mar = c(4.5, 4.5, 3, 1))
    ag <- aggregate(rpd ~ arm + t, data = ext, FUN = mean)
    plot(NA, xlim = range(ag$t), ylim = range(ag$rpd),
         xlab = "tiempo de CPU (s)", ylab = "RPD medio (%)",
         main = sprintf("%s — %s (media de %d instancias)", toupper(algo), cl, length(unique(sub$inst))))
    for (ar in arms) {
      s <- ag[ag$arm == ar, ]; s <- s[order(s$t), ]
      if (nrow(s)) lines(s$t, s$rpd, col = cols[[ar]], lwd = if (ar == "A0") 3 else 1.8)
    }
    legend("topright", legend = labs[arms], col = cols[arms], lwd = 2, bty = "n", cex = 0.75)
    dev.off()
  }
  cat("\ncurvas anytime generadas por clase\n")
} else {
  cat("\n(sin anytime_%s.csv: ejecutar final/resample_anytime.sh)\n")
}
