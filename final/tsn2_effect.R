# ¿Cuanto mejora exactamente TSN2 con siembra MIX? Magnitud absoluta y relativa,
# global y por clase, mas el detalle de las instancias donde gana y donde pierde.
d  <- read.csv("final/phase2/results_tsn2.csv", stringsAsFactors = FALSE)
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
m  <- aggregate(ecmax ~ inst + clase + arm, data = d, FUN = mean)
a0 <- m[m$arm == "A0", c("inst","clase","ecmax")]; names(a0)[3] <- "base"
s  <- merge(m[m$arm == "MIX", ], a0, by = c("inst","clase"))
s  <- merge(s, lb, by = "inst")
s$dabs <- s$ecmax - s$base
s$drel <- 100 * s$dabs / s$base
s$rpd0 <- 100 * (s$base  - s$lb) / s$lb
s$rpd1 <- 100 * (s$ecmax - s$lb) / s$lb

cat("=== GLOBAL (61 instancias) ===\n")
cat(sprintf("  E[Cmax] medio  control %.1f  ->  MIX %.1f   (%.1f unidades, %+.2f%%)\n",
            mean(s$base), mean(s$ecmax), mean(s$dabs), mean(s$drel)))
cat(sprintf("  RPD medio      control %.2f%% ->  MIX %.2f%%  (%.2f puntos, %.1f%% del exceso sobre la cota)\n",
            mean(s$rpd0), mean(s$rpd1), mean(s$rpd1) - mean(s$rpd0),
            100 * (mean(s$rpd0) - mean(s$rpd1)) / mean(s$rpd0)))

cat("\n=== POR CLASE ===\n")
cat(sprintf("%-8s %5s %12s %12s %10s %9s\n", "clase", "n", "E[Cmax] A0", "E[Cmax] MIX", "dif.abs", "dif.rel"))
for (cl in c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")) {
  x <- s[s$clase == cl, ]; if (!nrow(x)) next
  cat(sprintf("%-8s %5d %12.1f %12.1f %10.1f %8.2f%%\n",
              cl, nrow(x), mean(x$base), mean(x$ecmax), mean(x$dabs), mean(x$drel)))
}

cat("\n=== donde se concentra: aportacion de cada clase a la mejora total ===\n")
tot <- sum(s$dabs)
for (cl in c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")) {
  x <- s[s$clase == cl, ]; if (!nrow(x)) next
  cat(sprintf("  %-8s suma de diferencias = %8.1f   (%5.1f%% del total)\n",
              cl, sum(x$dabs), 100 * sum(x$dabs) / tot))
}
cat(sprintf("  TOTAL %.1f\n", tot))

cat("\n=== reparto por instancia ===\n")
cat(sprintf("  mejora en %d de 61 instancias | empeora en %d | mejora mediana %.2f%% | mejor caso %.2f%% | peor caso %+.2f%%\n",
            sum(s$dabs < 0), sum(s$dabs > 0), median(s$drel), min(s$drel), max(s$drel)))
