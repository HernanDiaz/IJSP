# ¿SON DOS EFECTOS O UNO?
#
# El articulo tiene dos afirmaciones de mecanismo medidas sobre pools distintos:
#   - composicion: MIX (mezcla) frente a los generadores unicos, a fraccion 100%
#   - fraccion:    V2H (media poblacion) frente a V2, sobre el pool v2
# MIXH combina ambas. Si mejora sobre MIX, los efectos se suman; si no, la
# composicion ya estaba haciendo el trabajo de la aleatoriedad retenida.
#
# Contrastes emparejados por instancia (n=61), negativo = el primero es mejor.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
set.seed(20260828)
B <- 10000

par <- list(c("MIXH","MIX"),    # ¿anade algo retener aleatorio sobre el mixto?
            c("MIXH","V2H"),    # ¿anade algo componer sobre media poblacion?
            c("MIXH","A0"))     # ¿mejora al control?

cat("=== MIXH frente a los brazos que combina ===\n")
cat("    diferencia media por instancia, IC bootstrap sobre instancias,\n")
cat("    p de Wilcoxon emparejado. Negativo = MIXH mejor.\n\n")
res <- data.frame()
for (al in names(algos)) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  d$rpd <- 100 * (d$ecmax - d$lb) / d$lb
  m <- aggregate(rpd ~ inst + arm, d, mean)
  w <- reshape(m, idvar = "inst", timevar = "arm", direction = "wide")
  cat(sprintf("%s\n", algos[[al]]))
  cat(sprintf("  %-12s %9s %20s %9s\n", "contraste", "dif", "IC 95%", "p"))
  for (p in par) {
    a <- w[[paste0("rpd.", p[1])]]; b <- w[[paste0("rpd.", p[2])]]
    if (is.null(a) || is.null(b)) next
    x <- a - b
    M <- matrix(sample(x, length(x)*B, TRUE), nrow = length(x))
    ci <- quantile(colMeans(M), c(.025, .975))
    pv <- suppressWarnings(wilcox.test(x)$p.value)
    cat(sprintf("  %-12s %+9.3f   [%+7.3f, %+7.3f] %9.4f%s\n",
                paste(p, collapse = " vs "), mean(x), ci[1], ci[2], pv,
                ifelse(pv < 0.05, " *", "")))
    res <- rbind(res, data.frame(algo = al, a = p[1], b = p[2],
                                 dif = mean(x), lo = ci[1], hi = ci[2], p = pv))
  }
  cat("\n")
}
write.csv(res, "final/mixh_contrastes.csv", row.names = FALSE)
cat("escrito: final/mixh_contrastes.csv\n")
