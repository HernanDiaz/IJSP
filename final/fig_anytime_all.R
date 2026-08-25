# FIGURA ANYTIME GENERAL: las 61 instancias y los 7 BRAZOS, un panel por algoritmo.
#
# El presupuesto va de 60 s a 900 s segun clase y algoritmo, asi que el tiempo
# absoluto no es comparable entre instancias. Se normaliza cada traza a FRACCION
# DE PRESUPUESTO CONSUMIDO (t / t_max de esa instancia) y se promedia sobre las 61
# instancias en una rejilla comun, de modo que ninguna instancia entra o sale a
# mitad de curva (el sesgo de supervivencia que ya nos mordio antes).
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga = "GA", abce3 = "ABCE3", feabcls = "fEABCLS", tsn2 = "TSN2")
arms <- c(A0 = "Unseeded (A0)", V2H = "Learned, half (V2H)", V2 = "Learned (V2)",
          MOR = "MOR-eps", GT = "GT-eps", GP = "GP-eps", MIX = "Mixed pool (MIX)")
cols <- c(A0 = "#000000", V2H = "#0072B2", V2 = "#56B4E9", MOR = "#E69F00",
          GT = "#D55E00", GP = "#009E73", MIX = "#CC0000")
lwds <- c(A0 = 3.2, V2H = 1.8, V2 = 1.8, MOR = 1.8, GT = 1.8, GP = 1.8, MIX = 3.2)
ltys <- c(A0 = 1, V2H = 2, V2 = 1, MOR = 4, GT = 5, GP = 3, MIX = 1)
fr <- seq(0.02, 1, by = 0.02)

png("final/figs/anytime_all.png", width = 1800, height = 1350, res = 145)
par(mfrow = c(2, 2), mar = c(4.0, 4.3, 2.6, 1.0))
resumen <- list()
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors = FALSE)
  d <- d[d$arm %in% names(arms), ]
  d <- merge(d, lb, by = "inst")
  d$rpd <- 100 * (d$bestcmax - d$lb) / d$lb
  ok <- Reduce(intersect, lapply(names(arms), function(a) unique(d$inst[d$arm == a])))
  M <- sapply(names(arms), function(a) {
    curvas <- sapply(ok, function(i) {
      s <- d[d$inst == i & d$arm == a, ]; s <- s[order(s$t), ]
      if (nrow(s) < 2) return(rep(NA, length(fr)))
      tn <- s$t / max(s$t)
      sapply(fr, function(f) { j <- which(tn <= f); if (!length(j)) s$rpd[1] else s$rpd[max(j)] })
    })
    rowMeans(curvas, na.rm = TRUE)
  })
  resumen[[al]] <- M
  plot(NA, xlim = c(0, 1), ylim = range(M), xlab = "Fraction of budget consumed",
       ylab = "Mean RPD vs. lower bound (%)",
       main = sprintf("%s  (all 61 instances)", algos[[al]]))
  grid(col = "grey90")
  for (a in names(arms)) lines(fr, M[, a], col = cols[[a]], lwd = lwds[[a]], lty = ltys[[a]])
  legend("topright", legend = unname(arms), col = cols[names(arms)],
         lwd = lwds[names(arms)], lty = ltys[names(arms)], bty = "n", cex = 0.72)
}
dev.off()

cat("=== RPD medio (%) sobre las 61 instancias, por fraccion de presupuesto ===\n")
cat(sprintf("%-8s %-4s %7s %7s %7s %7s %7s\n", "algo","arm","10%","25%","50%","75%","100%"))
idx <- sapply(c(.10,.25,.50,.75,1.0), function(f) which.min(abs(fr - f)))
for (al in names(algos)) { for (a in names(arms)) {
  v <- resumen[[al]][idx, a]
  cat(sprintf("%-8s %-4s %7.2f %7.2f %7.2f %7.2f %7.2f\n", algos[[al]], a, v[1],v[2],v[3],v[4],v[5]))
} ; cat("\n") }
cat("figura: final/figs/anytime_all.png\n")
