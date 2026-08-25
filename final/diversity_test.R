# ¿Se sostiene la hipotesis del COLAPSO DE DIVERSIDAD?
# Proxy: brecha relativa (media_poblacion - mejor)/mejor * 100 a lo largo del run.
# Hipotesis: en el GA la poblacion sembrada colapsa (brecha pequena desde el inicio
# y se cierra antes) mientras el control mantiene dispersion; en ABCE3 no colapsa.
# Ademas: ¿la brecha temprana predice el signo del efecto de la siembra?
algos <- c("ga","abce3","feabcls")
res_all <- list()
cat("=== brecha media-mejor (%) por brazo y momento del run ===\n")
cat(sprintf("%-8s %-5s %8s %8s %8s %8s\n", "algo", "arm", "inicio", "25%", "50%", "final"))
for (al in algos) {
  f <- sprintf("final/phase2/diversity_%s.csv", al)
  if (!file.exists(f)) next
  d <- read.csv(f, stringsAsFactors = FALSE)
  res <- data.frame()
  for (ins in unique(d$inst)) for (ar in unique(d$arm[d$inst == ins])) {
    s <- d[d$inst == ins & d$arm == ar, ]
    s <- s[order(s$t), ]
    if (nrow(s) < 4) next
    tm <- max(s$t); if (tm <= 0) next
    val <- function(frac) { i <- which(s$t <= frac * tm); if (!length(i)) NA else s$spread[i[length(i)]] }
    res <- rbind(res, data.frame(inst = ins, arm = ar,
                                 ini = s$spread[1], q25 = val(.25), q50 = val(.50), fin = s$spread[nrow(s)]))
  }
  res_all[[al]] <- res
  for (ar in c("A0","V2","MIX","MOR")) {
    s <- res[res$arm == ar, ]; if (!nrow(s)) next
    cat(sprintf("%-8s %-5s %8.2f %8.2f %8.2f %8.2f\n", al, ar,
                mean(s$ini, na.rm=TRUE), mean(s$q25, na.rm=TRUE),
                mean(s$q50, na.rm=TRUE), mean(s$fin, na.rm=TRUE)))
  }
  cat("\n")
}

# ¿la brecha inicial del brazo sembrado predice el efecto (delta) en ese algoritmo?
cat("=== ¿predice la brecha temprana el efecto de la siembra? (Spearman) ===\n")
for (al in algos) {
  rf <- sprintf("final/phase2/results_%s.csv", al)
  if (!file.exists(rf) || is.null(res_all[[al]])) next
  r <- read.csv(rf, stringsAsFactors = FALSE)
  res <- res_all[[al]]
  d <- data.frame()
  for (ins in unique(res$inst[res$arm == "V2"])) {
    a0 <- r$ecmax[r$inst == ins & r$arm == "A0"]; xx <- r$ecmax[r$inst == ins & r$arm == "V2"]
    sp <- res$ini[res$inst == ins & res$arm == "V2"]
    sp0 <- res$ini[res$inst == ins & res$arm == "A0"]
    if (!length(a0) || !length(xx) || !length(sp) || !length(sp0)) next
    d <- rbind(d, data.frame(spread_rel = sp[1] / sp0[1],           # dispersion sembrada vs control
                             delta = (mean(xx) - mean(a0)) / mean(a0) * 100))
  }
  if (nrow(d) < 5) next
  rho <- suppressWarnings(cor(d$spread_rel, d$delta, method = "spearman"))
  cat(sprintf("  %-8s n=%2d  rho(dispersion_relativa , delta) = %+.3f   %s\n", al, nrow(d), rho,
              ifelse(abs(rho) > 0.4, "<- relacion apreciable", "")))
}
cat("\n(delta negativo = la siembra ayuda; dispersion_relativa <1 = el sembrado arranca menos diverso)\n")
