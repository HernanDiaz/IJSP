# ¿Cuanta informacion se pierde realmente por repetir 5 poblaciones iniciales?
#
# En los algoritmos troceados la extraccion numera las ejecuciones 1..30
# concatenando c1..c6, 5 por trozo. Luego el BLOQUE de semillas de la ejecucion r
# es ((r-1) mod 5) + 1, y las 6 ejecuciones que comparten bloque parten de la
# MISMA poblacion inicial pero con distinto flujo aleatorio del algoritmo.
#
# Descomposicion de varianza por celda (instancia x brazo):
#   ICC = var_entre_bloques / var_total
# ICC alto  -> la poblacion inicial manda; repetirla desperdicia mucho.
# ICC bajo  -> el azar del algoritmo domina; las 30 ejecuciones siguen aportando
#              casi tanta informacion como 30 poblaciones distintas.
for (al in c("tsn2","feabcls")) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  d <- d[d$arm != "A0", ]                      # el control no usa pool
  d$blk <- ((d$run - 1) %% 5) + 1
  iccs <- c(); nef <- c()
  for (k in unique(paste(d$inst, d$arm))) {
    s <- d[paste(d$inst, d$arm) == k, ]
    if (nrow(s) < 30) next
    g <- tapply(s$ecmax, s$blk, mean)
    vb <- var(g)                                # varianza entre bloques (n=5)
    vw <- mean(tapply(s$ecmax, s$blk, var))     # varianza dentro de bloque
    vt <- vb + vw
    if (!is.finite(vt) || vt <= 0) next
    icc <- vb / vt
    iccs <- c(iccs, icc)
    # tamano muestral efectivo de 30 ejecuciones con correlacion intra-bloque icc
    nef <- c(nef, 30 / (1 + (6 - 1) * icc))
  }
  cat(sprintf("=== %s (%d celdas instancia x brazo) ===\n", toupper(al), length(iccs)))
  cat(sprintf("  ICC (proporcion de varianza atribuible a la poblacion inicial):\n"))
  cat(sprintf("    mediana %.3f   media %.3f   rango [%.3f, %.3f]\n",
              median(iccs), mean(iccs), min(iccs), max(iccs)))
  cat(sprintf("  n efectivo de las 30 ejecuciones:  mediana %.1f   media %.1f   rango [%.1f, %.1f]\n\n",
              median(nef), mean(nef), min(nef), max(nef)))
}
cat("(n efectivo = 30 / (1 + 5*ICC).  Si ICC=0 -> 30 (no se pierde nada);\n")
cat(" si ICC=1 -> 5 (solo cuentan las 5 poblaciones distintas).)\n")
