# Comprobacion: las curvas anytime regeneradas deben ser estrictamente monotonas
for (al in c("ga","abce3","feabcls","tsn2")) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors=FALSE)
  up <- 0; tot <- 0; ncell <- 0
  for (k in unique(paste(d$inst, d$arm))) {
    s <- d[paste(d$inst, d$arm) == k, ]; s <- s[order(s$t), ]
    if (nrow(s) < 2) next
    ncell <- ncell + 1
    dif <- diff(s$bestcmax); tot <- tot + length(dif); up <- up + sum(dif > 1e-9)
  }
  cat(sprintf("  %-8s %3d celdas | %6d puntos | %d subidas | %d instancias\n",
      al, ncell, tot, up, length(unique(d$inst))))
}
