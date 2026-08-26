# Coste de generar las 250 semillas de UNA ejecucion frente al presupuesto.
# GP y GT: medidos (log del 13 de julio, 1024 soluciones por pool, 1 hilo).
# v2: EXTRAPOLADO desde tiempos de inferencia por rollout. No es una medida.
cls  <- c("15x15","20x20","30x15","30x20","50x15","50x20")
gp   <- c(91,205,301,486,834,1255)/1024      # s por solucion
gt   <- c(89,257,299,489,1052,1500)/1024
v2   <- c(0.60,  NA, NA, 1.86, NA, 3.98)     # medidos solo en 4 clases
bud  <- list(GA=c(60,60,109,154,233,321), ABCE3=rep(60,6),
             fEABCLS=c(60,67,371,900,900,900), TSN2=c(60,486,630,900,900,900))

cat("Coste de las 250 semillas de UNA ejecucion (s), un hilo:\n\n")
cat(sprintf("%-8s %8s %8s %8s\n","clase","GP","GT","v2*"))
for (i in seq_along(cls))
  cat(sprintf("%-8s %8.0f %8.0f %8s\n", cls[i], 250*gp[i], 250*gt[i],
      ifelse(is.na(v2[i]), "--", sprintf("%.0f", 250*v2[i]))))

cat("\nFrente al 10% del presupuesto de cada solver (razon coste/presupuesto):\n\n")
cat(sprintf("%-8s %10s %10s %10s %10s\n","clase","GA","ABCE3","fEABCLS","TSN2"))
for (i in seq_along(cls)) {
  r <- sapply(names(bud), function(s) 250*gp[i] / (0.10*bud[[s]][i]))
  cat(sprintf("%-8s %9.0fx %9.0fx %9.0fx %9.0fx\n", cls[i], r[1], r[2], r[3], r[4]))
}
cat("  (GP, el mas barato de los tres; con v2 multiplica por ~3)\n")

cat("\nAmortizado sobre las 30 ejecuciones que comparten el pool:\n\n")
cat(sprintf("%-8s %10s %10s %10s %10s\n","clase","GA","ABCE3","fEABCLS","TSN2"))
for (i in seq_along(cls)) {
  r <- sapply(names(bud), function(s) (1024*gp[i]/30) / (0.10*bud[[s]][i]))
  cat(sprintf("%-8s %9.1fx %9.1fx %9.1fx %9.1fx\n", cls[i], r[1], r[2], r[3], r[4]))
}
