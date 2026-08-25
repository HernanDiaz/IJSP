# ¿Importa el 22% de solape entre bloques (pool 1024) frente al 0% (pool 7500)?
# ABCE3 y GA corrieron monoliticos: sus 30 ejecuciones ya usan 30 bloques
# DISTINTOS. La unica diferencia entre brazos es el solape: MIX (L=7500) tiene
# bloques disjuntos; v2/gp/mor/gt (L=1024) se solapan un 22% de media.
# Si el solape correlacionase las ejecuciones, MIX deberia mostrar MAS dispersion
# entre ejecuciones que los brazos de pool pequeno (menos correlacion -> mas var).
# Se compara el coeficiente de variacion entre ejecuciones, que es adimensional.
for (al in c("abce3","ga")) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  arms <- c("V2","MOR","GT","GP","MIX")
  cv <- list()
  for (a in arms) {
    v <- c()
    for (ins in unique(d$inst)) {
      x <- d$ecmax[d$inst == ins & d$arm == a]
      if (length(x) < 10 || mean(x) <= 0) next
      v <- c(v, 100 * sd(x) / mean(x))
    }
    cv[[a]] <- v
  }
  cat(sprintf("=== %s : coeficiente de variacion entre las 30 ejecuciones (%%) ===\n", toupper(al)))
  cat(sprintf("%-5s %8s %8s   %s\n", "brazo", "mediana", "media", "tamano de pool"))
  for (a in arms) {
    L <- if (a == "MIX") "7500 (disjunto)" else "1024 (solape 22%)"
    cat(sprintf("%-5s %8.3f %8.3f   %s\n", a, median(cv[[a]]), mean(cv[[a]]), L))
  }
  # ¿es MIX significativamente mas disperso que los de pool pequeno?
  small <- unlist(cv[c("V2","GP")])
  p <- suppressWarnings(wilcox.test(cv[["MIX"]], small)$p.value)
  cat(sprintf("  MIX vs (V2,GP):  diferencia de CV = %+.3f puntos, p = %.3f  %s\n\n",
              mean(cv[["MIX"]]) - mean(small), p,
              ifelse(p < 0.05, "<- diferencia apreciable", "<- sin diferencia detectable")))
}
cat("Si no hay diferencia, el solape del 22% no correlaciona las ejecuciones de\n")
cat("forma medible, y ampliar los pools a 7500 no compraria potencia estadistica.\n")
