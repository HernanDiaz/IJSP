# ¿El predictor 'gap' funciona DENTRO de cada algoritmo? (la decision util en la
# practica: fijado mi solver, ¿en que instancias conviene sembrar?)
# Ademas se prueban predictores alternativos: dispersion del pool (diversidad de
# calidad) y tamaño de la instancia.
gen <- "v2"; arm <- "V2"
algos <- c("abce3","ga","feabcls")
pq <- read.csv("final/pool_quality.csv", stringsAsFactors = FALSE)
pq <- pq[pq$gen == gen, ]

cat("=== correlaciones DENTRO de cada algoritmo (Spearman, n=61) ===\n")
cat(sprintf("%-8s %14s %14s %14s %10s\n", "algo", "gap~delta", "sd_pool~delta", "tam~delta", "%ayuda"))
for (al in algos) {
  f <- sprintf("final/phase2/results_%s.csv", al)
  if (!file.exists(f)) next
  r <- read.csv(f, stringsAsFactors = FALSE)
  d <- data.frame()
  for (ins in unique(r$inst)) {
    a0 <- r$ecmax[r$inst == ins & r$arm == "A0"]
    xx <- r$ecmax[r$inst == ins & r$arm == arm]
    p  <- pq[pq$inst == ins, ]
    if (!length(a0) || !length(xx) || !nrow(p)) next
    n_ops <- switch(sub("_[0-9]+$","",ins), "tai15_15"=225, "tai20_20"=400, "tai30_15"=450,
                    "tai30_20"=600, "tai50_15"=750, "tai50_20"=1000, 100)
    d <- rbind(d, data.frame(
      gap    = (p$pool_mean[1] - mean(a0)) / mean(a0) * 100,
      sdrel  = p$pool_sd[1] / p$pool_mean[1] * 100,
      ops    = n_ops,
      delta  = (mean(xx) - mean(a0)) / mean(a0) * 100))
  }
  if (nrow(d) < 5) next
  c1 <- suppressWarnings(cor(d$gap,   d$delta, method="spearman"))
  c2 <- suppressWarnings(cor(d$sdrel, d$delta, method="spearman"))
  c3 <- suppressWarnings(cor(d$ops,   d$delta, method="spearman"))
  cat(sprintf("%-8s %14.3f %14.3f %14.3f %9.0f%%\n", al, c1, c2, c3, 100*mean(d$delta < 0)))
}
cat("\n(|rho| < 0.3 = practicamente sin poder predictivo)\n")
