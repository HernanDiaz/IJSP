# REGLA DE DECISION: ¿se puede predecir, ANTES de sembrar, si la siembra va a
# ayudar en un par (algoritmo, instancia)?
#
# Predictor (barato, disponible a priori):
#   gap = (calidad_media_del_pool - calidad_del_control) / calidad_del_control
#   -> gap < 0: el pool es MEJOR que lo que el algoritmo alcanza solo
#   -> gap > 0: el algoritmo ya supera al pool por su cuenta
# Resultado observado:
#   delta = E[Cmax] medio del brazo sembrado - del control  (negativo = ayuda)
#
# Validacion: correlacion, y regla de umbral con validacion cruzada
# DEJANDO UNA INSTANCIA FUERA (el umbral se ajusta sin ver la instancia de test).
args <- commandArgs(trailingOnly = TRUE)
gen  <- ifelse(length(args) >= 1, args[1], "v2")     # generador a analizar
arm  <- ifelse(length(args) >= 2, args[2], "V2")     # brazo correspondiente
algos <- c("abce3","ga","feabcls")

pq <- read.csv("final/pool_quality.csv", stringsAsFactors = FALSE)
pq <- pq[pq$gen == gen, ]

d <- data.frame()
for (al in algos) {
  f <- sprintf("final/phase2/results_%s.csv", al)
  if (!file.exists(f)) next
  r <- read.csv(f, stringsAsFactors = FALSE)
  for (ins in unique(r$inst)) {
    a0 <- r$ecmax[r$inst == ins & r$arm == "A0"]
    xx <- r$ecmax[r$inst == ins & r$arm == arm]
    p  <- pq$pool_mean[pq$inst == ins]
    if (!length(a0) || !length(xx) || !length(p)) next
    d <- rbind(d, data.frame(algo = al, inst = ins,
                             q_a0 = mean(a0), q_arm = mean(xx), q_pool = p[1],
                             stringsAsFactors = FALSE))
  }
}
d$gap   <- (d$q_pool - d$q_a0) / d$q_a0 * 100      # % (positivo: pool peor)
d$delta <- (d$q_arm - d$q_a0) / d$q_a0 * 100       # % (negativo: siembra ayuda)
d$ayuda <- d$delta < 0

cat(sprintf("=== Predictor 'gap' (pool vs control) frente al resultado, generador %s ===\n", gen))
cat(sprintf("%-8s %6s %10s %10s %10s\n", "algo", "n", "gap_medio", "delta_medio", "%ayuda"))
for (al in algos) {
  s <- d[d$algo == al, ]; if (!nrow(s)) next
  cat(sprintf("%-8s %6d %9.1f%% %10.2f%% %9.0f%%\n", al, nrow(s),
              mean(s$gap), mean(s$delta), 100*mean(s$ayuda)))
}

cat("\n=== correlacion gap ~ delta ===\n")
ct <- cor.test(d$gap, d$delta, method = "spearman", exact = FALSE)
cat(sprintf("  Spearman rho = %.3f  (p = %.2e, n = %d)\n", ct$estimate, ct$p.value, nrow(d)))
cg <- cor(d$gap, d$delta, method = "pearson")
cat(sprintf("  Pearson  r   = %.3f\n", cg))

# --- regla de umbral con validacion cruzada dejando una instancia fuera ---
umbral_optimo <- function(df) {
  cand <- sort(unique(round(df$gap, 1)))
  best <- NA; bacc <- -1
  for (u in cand) {
    pred <- df$gap < u          # predice "ayuda" si el pool es suficientemente bueno
    acc <- mean(pred == df$ayuda)
    if (acc > bacc) { bacc <- acc; best <- u }
  }
  best
}
aciertos <- c(); base <- c()
for (i in seq_len(nrow(d))) {
  tr <- d[-i, ]; te <- d[i, ]
  u <- umbral_optimo(tr)
  aciertos <- c(aciertos, (te$gap < u) == te$ayuda)
  base <- c(base, te$ayuda)
}
cat("\n=== regla de umbral, validacion cruzada (deja-uno-fuera) ===\n")
cat(sprintf("  acierto de la regla:            %.1f%%\n", 100*mean(aciertos)))
maj <- max(mean(base), 1 - mean(base))
cat(sprintf("  acierto de la clase mayoritaria: %.1f%%  (linea base a batir)\n", 100*maj))
cat(sprintf("  umbral ajustado con todos:       gap < %.1f%%\n", umbral_optimo(d)))

cat("\n=== por algoritmo: gap medio y si la regla acierta el signo dominante ===\n")
for (al in algos) {
  s <- d[d$algo == al, ]; if (!nrow(s)) next
  cat(sprintf("  %-8s gap=%6.1f%%  ayuda en %2.0f%% de instancias  (delta medio %+.2f%%)\n",
              al, mean(s$gap), 100*mean(s$ayuda), mean(s$delta)))
}
write.csv(d, sprintf("final/tables/decision_rule_%s.csv", gen), row.names = FALSE)
