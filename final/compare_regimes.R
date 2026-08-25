# CONTROL DE ROBUSTEZ: ¿cambian las conclusiones al homogeneizar el protocolo?
# Compara, para GA y ABCE3, el regimen ANTERIOR (archivado) con el NUEVO
# (troceado con creation.seed.offset, y MIX entrelazado de 1024).
#
# Cambian dos cosas a la vez, asi que la comparacion es de robustez global:
#   - el reparto de semillas entre ejecuciones (offset)
#   - el pool MIX (antes 7500 por bloques, ahora 1024 entrelazado)
# En GA y ABCE3 el offset no altera nada (ya corrian monoliticos con 30 bloques
# distintos), de modo que lo que se aisla aqui es sobre todo el efecto del
# cambio de pool MIX y del troceado en si.
ARCH <- Sys.getenv("ARCH", "final/phase2/_archivo_regimen_previo_20260809_025344")
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")

rpd <- function(f) {
  if (!file.exists(f)) return(NULL)
  d <- merge(read.csv(f, stringsAsFactors = FALSE), lb, by = "inst")
  d$rpd <- 100 * (d$ecmax - d$lb) / d$lb
  ag <- aggregate(rpd ~ inst + arm, data = d, FUN = mean)
  w <- reshape(ag, idvar = "inst", timevar = "arm", direction = "wide")
  ok <- complete.cases(w[, paste0("rpd.", arms)])
  list(mean = sapply(arms, function(a) mean(w[ok, paste0("rpd.", a)])),
       per  = w[ok, ], n = sum(ok))
}

for (al in c("ga","abce3","feabcls","tsn2")) {
  new <- rpd(sprintf("final/phase2/results_%s.csv", al))
  old <- rpd(sprintf("%s/results_%s.csv", ARCH, al))
  if (is.null(new) || is.null(old)) { cat(sprintf("(%s: faltan datos)\n", al)); next }
  cat(sprintf("\n=== %s : RPD medio (%%) sobre la cota inferior  [n=%d instancias] ===\n",
              toupper(al), new$n))
  cat(sprintf("%-10s %s\n", "regimen", paste(sprintf("%8s", arms), collapse = "")))
  cat(sprintf("%-10s %s\n", "anterior", paste(sprintf("%8.2f", old$mean), collapse = "")))
  cat(sprintf("%-10s %s\n", "nuevo",    paste(sprintf("%8.2f", new$mean), collapse = "")))
  cat(sprintf("%-10s %s\n", "cambio",   paste(sprintf("%+8.2f", new$mean - old$mean), collapse = "")))

  # ¿coincide el ORDEN de los brazos? es lo que sostiene las conclusiones
  ro <- rank(old$mean); rn <- rank(new$mean)
  cat(sprintf("  orden de brazos: Spearman = %.3f  |  mejor brazo: antes %s, ahora %s\n",
              cor(ro, rn, method = "spearman"), arms[which.min(old$mean)], arms[which.min(new$mean)]))

  # ¿es el cambio por instancia mayor que el ruido entre ejecuciones?
  m <- merge(old$per, new$per, by = "inst", suffixes = c(".old",".new"))
  for (a in c("A0","MIX")) {
    d <- m[[paste0("rpd.", a, ".new")]] - m[[paste0("rpd.", a, ".old")]]
    p <- suppressWarnings(wilcox.test(d)$p.value)
    cat(sprintf("  %-4s: cambio por instancia  mediana %+.3f  rango [%+.2f, %+.2f]  p=%.3f %s\n",
                a, median(d), min(d), max(d), p,
                ifelse(p < 0.05, "<- diferencia sistematica", "<- sin diferencia sistematica")))
  }
}
