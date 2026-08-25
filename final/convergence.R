# ¿QUE CELDAS SATISFACEN LA REGLA DE MESETA Y CUALES NO?
# Regla del diseno: converge si la mejora del OBJETIVO (E[Cmax], no del RPD) en
# el ultimo 10% del presupuesto es < 0.1%. Se evalua sobre el CONTROL (A0),
# que es quien determina si la celda admite afirmaciones de calidad convergida.
# Las que no la cumplen son "anytime-only": aportan resultados a presupuesto
# fijo, pero no calidad convergida.
#
# Requiere las curvas anytime MONOTONAS (minimo acumulado por ejecucion); sobre
# una serie no monotona la "mejora en el ultimo 10%" puede salir negativa.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
clases <- c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
UMBRAL <- 0.1   # %

# clase por instancia, tomada del propio CSV de resultados (no re-derivada)
cl <- unique(read.csv("final/phase2/results_ga.csv", stringsAsFactors = FALSE)[, c("inst","clase")])

conv <- list()
cat(sprintf("%-8s %-8s %6s %11s   %s\n", "solver","clase","n","mejora 10%","veredicto"))
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors = FALSE)
  d <- merge(d[d$arm == "A0", ], lb, by = "inst")
  d$rpd <- 100 * (d$bestcmax - d$lb) / d$lb
  est <- data.frame()
  for (i in unique(d$inst)) {
    s <- d[d$inst == i, ]; s <- s[order(s$t), ]
    if (nrow(s) < 5) next
    T <- max(s$t)
    idx <- which(s$t <= 0.9 * T)
    if (!length(idx)) next
    v90 <- s$bestcmax[idx[length(idx)]]
    vfin <- s$bestcmax[nrow(s)]
    est <- rbind(est, data.frame(inst = i, mejora = 100*(v90 - vfin)/vfin))
  }
  est <- merge(est, cl, by = "inst", all.x = FALSE)
  est$conv <- est$mejora < UMBRAL
  conv[[al]] <- est
  for (k in clases) {
    x <- est[est$clase == k, ]
    if (nrow(x) == 0) next
    nc <- sum(x$conv)
    cat(sprintf("%-8s %-8s %6d %10.3f%%   %s\n", algos[[al]], k, nrow(x), mean(x$mejora),
        if (nc == nrow(x)) "converge" else sprintf("*** %d/%d ANYTIME-ONLY ***", nrow(x)-nc, nrow(x))))
  }
  cat("\n")
}

cat("=== celdas anytime-only por solver ===\n")
tot <- 0
for (al in names(algos)) {
  e <- conv[[al]]; n <- sum(!e$conv); tot <- tot + n
  cat(sprintf("  %-8s %2d de %d\n", algos[[al]], n, nrow(e)))
}
cat(sprintf("  TOTAL: %d de %d celdas de control\n", tot, 4*61))

cat("\n=== RPD medio: TODAS las instancias vs SOLO las convergidas ===\n")
for (al in names(algos)) {
  r <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  r$rpd <- 100*(r$ecmax - r$lb)/r$lb
  ok <- conv[[al]]$inst[conv[[al]]$conv]
  todo <- sapply(arms, function(a) mean(r$rpd[r$arm == a]))
  solo <- if (length(ok)) sapply(arms, function(a) mean(r$rpd[r$arm == a & r$inst %in% ok])) else rep(NA, 7)
  cat(sprintf("\n  %s  (%d de %d instancias convergen)\n", algos[[al]], length(ok), nrow(conv[[al]])))
  cat(sprintf("    %-12s %s\n", "brazo",       paste(sprintf("%7s", arms), collapse="")))
  cat(sprintf("    %-12s %s\n", "todas",       paste(sprintf("%7.2f", todo), collapse="")))
  cat(sprintf("    %-12s %s\n", "convergidas", paste(sprintf("%7.2f", solo), collapse="")))
  if (length(ok) > 1)
    cat(sprintf("    mejor brazo: %s (todas) / %s (convergidas)\n",
        arms[which.min(todo)], arms[which.min(solo)]))
}

# --- se persiste la clasificacion para que el paper pueda citarla ---
out <- do.call(rbind, lapply(names(conv), function(a)
  data.frame(algo = a, conv[[a]][, c("inst","clase","mejora","conv")])))
write.csv(out, "final/convergencia.csv", row.names = FALSE)
cat(sprintf("\nescrito: final/convergencia.csv (%d filas)\n", nrow(out)))
