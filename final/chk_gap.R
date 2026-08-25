# Brecha entre el ultimo punto de la curva anytime y el E[Cmax] realmente reportado
lb <- read.csv("final/ta_lb.csv")[,c("inst","lb")]
for (al in c("ga","abce3","feabcls","tsn2")) {
  a <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors=FALSE)
  a <- a[a$arm=="A0",]
  fin_traza <- sapply(split(a, a$inst), function(s) s$bestcmax[which.max(s$t)])
  r <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors=FALSE)
  r <- r[r$arm=="A0",]
  fin_real <- tapply(r$ecmax, r$inst, mean)
  ins <- intersect(names(fin_traza), names(fin_real))
  d <- 100*(fin_traza[ins] - fin_real[ins])/fin_real[ins]
  cat(sprintf("  %-8s brecha traza-vs-reportado: mediana %+.2f%%  max %+.2f%%  (n=%d)\n",
      al, median(d), max(d), length(d)))
  peor <- ins[which.max(d)]
  cat(sprintf("           peor caso %s: traza %.1f, reportado %.1f\n", peor, fin_traza[peor], fin_real[peor]))
}
