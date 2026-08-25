# ¿Alguna ejecucion queda POR DEBAJO de la cota inferior publicada?
# Si ocurriera, o la cota esta mal asignada, o el makespan/decodificador falla,
# o la solucion es infactible. Es la comprobacion de cordura mas barata que hay.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
peor <- NULL; tot <- 0; viol <- 0
cat(sprintf("%-9s %8s %10s %10s %9s   %s\n", "algo", "runs", "min E[Cmax]", "peor RPD", "violac.", "instancia del minimo"))
for (al in c("ga","abce3","feabcls","tsn2")) {
  f <- sprintf("final/phase2/results_%s.csv", al); if (!file.exists(f)) next
  d <- merge(read.csv(f, stringsAsFactors = FALSE), lb, by = "inst")
  d$rpd <- 100 * (d$ecmax - d$lb) / d$lb
  v <- sum(d$rpd < 0); tot <- tot + nrow(d); viol <- viol + v
  i <- which.min(d$rpd)
  cat(sprintf("%-9s %8d %10.1f %9.2f%% %9d   %s (LB=%d)\n",
              al, nrow(d), d$ecmax[i], d$rpd[i], v, d$inst[i], d$lb[i]))
  if (v > 0) peor <- rbind(peor, d[d$rpd < 0, c("algo","inst","arm","run","ecmax","lb","rpd")])
}
cat(sprintf("\nTOTAL: %d ejecuciones | por debajo de la cota: %d\n", tot, viol))
if (!is.null(peor)) { cat("\n=== VIOLACIONES ===\n"); print(head(peor[order(peor$rpd), ], 20), row.names = FALSE) }

cat("\n=== margen minimo por instancia (el mejor E[Cmax] visto vs su cota) ===\n")
best <- NULL
for (al in c("ga","abce3","feabcls","tsn2")) {
  f <- sprintf("final/phase2/results_%s.csv", al); if (!file.exists(f)) next
  d <- read.csv(f, stringsAsFactors = FALSE)
  b <- aggregate(ecmax ~ inst, data = d, FUN = min)
  best <- if (is.null(best)) b else merge(best, b, by = "inst", suffixes = c("", al))
}
b <- data.frame(inst = best$inst, best = apply(best[, -1, drop = FALSE], 1, min))
b <- merge(b, lb, by = "inst"); b$gap <- 100 * (b$best - b$lb) / b$lb
b <- b[order(b$gap), ]
cat("  las 8 instancias con menos margen sobre la cota:\n")
print(head(b, 8), row.names = FALSE)
cat(sprintf("\n  margen minimo global: %+.2f%%  (si fuese negativo, habria un error)\n", min(b$gap)))
