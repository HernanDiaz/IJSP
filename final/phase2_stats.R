# Analisis por solver de la Fase 2: por brazo, wins/ties/losses vs A0 por
# instancia (Wilcoxon pareado, Holm dentro de cada instancia sobre los 6
# brazos), delta medio por clase, y A12 de Vargha-Delaney agregado.
# Uso: Rscript phase2_stats.R final/phase2/results_<algo>.csv
args <- commandArgs(trailingOnly = TRUE)
d <- read.csv(args[1], stringsAsFactors = FALSE)
arms <- c("V2H","V2","MOR","GT","GP","MIX","MIXH")
insts <- unique(d$inst)

a12 <- function(x, y) { # P(X < Y) + 0.5 P(=) ; x=brazo, y=A0 (menor mejor)
  m <- length(x); n <- length(y)
  r <- rank(c(x, y)); rx <- sum(r[1:m])
  (rx/m - (m+1)/2) / n
}

res <- list()
for (ar in arms) {
  W <- 0; T <- 0; L <- 0; a12s <- c(); deltas <- c()
  for (ins in insts) {
    a0 <- d[d$inst==ins & d$arm=="A0", ]; a0 <- a0[order(a0$run), "ecmax"]
    xx <- d[d$inst==ins & d$arm==ar,   ]; xx <- xx[order(xx$run), "ecmax"]
    if (length(a0) < 2 || length(xx) < 2 || length(a0) != length(xx)) next
    # p-values de los 6 brazos de esta instancia para Holm
    ps <- sapply(arms, function(a2) {
      yy <- d[d$inst==ins & d$arm==a2, ]; yy <- yy[order(yy$run), "ecmax"]
      if (length(yy) != length(a0)) return(NA)
      tryCatch(wilcox.test(yy, a0, paired=TRUE, exact=FALSE)$p.value, error=function(e) NA)
    })
    ph <- p.adjust(ps, method="holm")[ar]
    md <- mean(xx) - mean(a0)
    deltas <- c(deltas, md)
    a12s <- c(a12s, a12(xx, a0))
    if (!is.na(ph) && ph < 0.05) { if (md < 0) W <- W+1 else L <- L+1 } else T <- T+1
  }
  res[[ar]] <- c(W=W, T=T, L=L, dmean=mean(deltas), A12=mean(a12s))
}
cat(sprintf("%-5s %6s %6s %6s %10s %8s\n", "arm", "wins", "ties", "losses", "dAvg", "A12"))
# Los recuentos se PERSISTEN, no solo se imprimen. Codificarlos a mano en el
# generador de la tabla del articulo hizo que, al anadir un brazo, las filas
# antiguas conservaran los valores de la familia anterior: el Holm dentro de
# cada instancia se calcula sobre todos los brazos sembrados, asi que anadir
# uno cambia TODOS los recuentos, no solo el nuevo.
csvout <- sprintf("final/wl_%s.csv", sub(".*results_(.*)\\.csv", "\\1", args[1]))
wl <- do.call(rbind, lapply(arms, function(a) {
  r <- res[[a]]
  data.frame(arm = a, W = r["W"], T = r["T"], L = r["L"],
             dmean = r["dmean"], A12 = r["A12"], row.names = NULL) }))
write.csv(wl, csvout, row.names = FALSE)

for (ar in arms) {
  r <- res[[ar]]
  cat(sprintf("%-5s %6d %6d %6d %10.1f %8.3f\n", ar, r["W"], r["T"], r["L"], r["dmean"], r["A12"]))
}
cat("\n(wins/losses: Wilcoxon pareado con Holm por instancia, alfa=0.05; dAvg: media de deltas vs A0; A12<0.5 = el brazo tiende a ser mejor)\n\n")

cat("=== delta medio vs A0 por clase ===\n")
cls <- unique(d$clase)
cat(sprintf("%-7s", "clase")); for (ar in arms) cat(sprintf(" %8s", ar)); cat("\n")
for (cl in sort(cls)) {
  cat(sprintf("%-7s", cl))
  for (ar in arms) {
    dd <- c()
    for (ins in unique(d$inst[d$clase==cl])) {
      a0 <- mean(d$ecmax[d$inst==ins & d$arm=="A0"])
      xa <- mean(d$ecmax[d$inst==ins & d$arm==ar])
      dd <- c(dd, xa - a0)
    }
    cat(sprintf(" %8.1f", mean(dd)))
  }
  cat("\n")
}
