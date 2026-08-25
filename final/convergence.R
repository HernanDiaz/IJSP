# ¿QUE CELDAS SATISFACEN LA REGLA DE MESETA Y CUALES NO?
#
# Regla del diseno: una celda de CONTROL converge si la mejora del objetivo
# (E[Cmax], no del RPD) en el ultimo 10% del presupuesto es < 0.1%.
#
# DOS DEFECTOS DE LA VERSION ANTERIOR, ambos corregidos aqui:
#
# 1. Usaba como valor final el ULTIMO PUNTO DE LA CURVA promediada, que es una
#    instantanea del mejor de la POBLACION. El solver devuelve una solucion que
#    puede ser mejor, porque retiene una estructura elite y porque la ultima
#    mejora puede caer entre la ultima muestra y la terminacion. El propio
#    articulo mide ese desfase: mediana 2.20% en ABCE3, VEINTIDOS VECES el
#    umbral de 0.1% que la regla contrasta. Una regla no puede discriminar con
#    un umbral veinte veces menor que el error de su propio estimador.
#    Aqui el valor final es el DEVUELTO por el solver (results_<algo>.csv).
#
# 2. Aplicaba la regla a la curva PROMEDIADA sobre las 30 ejecuciones, lo que
#    puede ocultar que un subconjunto sigue mejorando al final. Aqui la regla se
#    evalua EJECUCION A EJECUCION y se agrega con un criterio declarado.
#
# SENTIDO DEL SESGO RESIDUAL. v90 es el minimo acumulado de la poblacion en
# t <= 0.9T, que es >= al incumbente real en ese instante (la elite puede
# guardar ya algo mejor). Por tanto la mejora medida (v90 - final)/final es una
# COTA SUPERIOR de la mejora real: la regla puede declarar "no convergida" una
# celda que si converge, nunca al reves. El error es conservador.
#
# Criterio de agregacion (declarado, no ajustado a posteriori): la celda
# converge si al menos el 90% de sus ejecuciones cumplen la regla. Se acompana
# de un analisis de sensibilidad sobre umbral y fraccion.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
clases <- c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
UMBRAL <- 0.1     # % de mejora en el ultimo 10% del presupuesto
FRAC   <- 0.90    # fraccion de ejecuciones que deben cumplirla

cl <- unique(read.csv("final/phase2/results_ga.csv", stringsAsFactors = FALSE)[, c("inst","clase")])

# --- mejora por ejecucion, con el valor final DEVUELTO por el solver ---
por_ejec <- function(al) {
  cv <- read.csv(sprintf("final/phase2/conv_runs_%s.csv", al), stringsAsFactors = FALSE)
  rs <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  rs <- rs[rs$arm == "A0", c("inst","run","ecmax")]
  m <- merge(cv, rs, by = c("inst","run"))
  m$mejora <- 100 * (m$v90 - m$ecmax) / m$ecmax
  m
}

cat("=== Regla de meseta sobre el control, por ejecucion ===\n")
cat(sprintf("    converge si >= %.0f%% de las 30 ejecuciones mejoran < %.1f%%\n",
            100*FRAC, UMBRAL))
cat("    valor final = solucion DEVUELTA, no ultimo punto de la curva\n\n")

datos <- list(); conv <- list()
cat(sprintf("%-8s %-8s %6s %11s %11s   %s\n",
            "solver","clase","n","mejora med","% ejec ok","veredicto"))
for (al in names(algos)) {
  m <- por_ejec(al); datos[[al]] <- m
  est <- aggregate(cbind(mejora, ok = mejora < UMBRAL) ~ inst, data = m,
                   FUN = function(x) c(med = median(x), mn = mean(x)))
  est <- data.frame(inst = est$inst,
                    mejora = est$mejora[, "med"],
                    frac_ok = est$ok[, "mn"])
  est$conv <- est$frac_ok >= FRAC
  est <- merge(est, cl, by = "inst")
  conv[[al]] <- est
  for (k in clases) {
    x <- est[est$clase == k, ]
    if (nrow(x) == 0) next
    nc <- sum(x$conv)
    cat(sprintf("%-8s %-8s %6d %10.3f%% %10.0f%%   %s\n", algos[[al]], k, nrow(x),
        median(x$mejora), 100*mean(x$frac_ok),
        if (nc == nrow(x)) "converge" else sprintf("*** %d/%d ANYTIME-ONLY ***", nrow(x)-nc, nrow(x))))
  }
  cat("\n")
}

cat("=== celdas anytime-only por solver ===\n")
tot <- 0
for (al in names(algos)) {
  e <- conv[[al]]; n <- sum(!e$conv); tot <- tot + n
  cat(sprintf("  %-8s %2d de %d\n", algos[[al]], n, nrow(e)))
}
cat(sprintf("  TOTAL: %d de %d celdas de control\n", tot, 4*61))

# --- comparacion con la clasificacion anterior (curva promediada) ---
cat("\n=== Cuanto cambia respecto a la regla sobre la curva promediada ===\n")
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors = FALSE)
  d <- d[d$arm == "A0", ]
  viejo <- sapply(unique(d$inst), function(i) {
    s <- d[d$inst == i, ]; s <- s[order(s$t), ]
    if (nrow(s) < 5) return(NA)
    Tm <- max(s$t); idx <- which(s$t <= 0.9*Tm)
    if (!length(idx)) return(NA)
    100*(s$bestcmax[idx[length(idx)]] - s$bestcmax[nrow(s)]) / s$bestcmax[nrow(s)] < UMBRAL
  })
  e <- conv[[al]]
  v <- viejo[e$inst]
  cat(sprintf("  %-8s antes %2d convergidas -> ahora %2d  (%d celdas cambian de clase)\n",
              algos[[al]], sum(v, na.rm = TRUE), sum(e$conv), sum(v != e$conv, na.rm = TRUE)))
}

# --- sensibilidad: la clasificacion depende del umbral y del criterio? ---
cat("\n=== Sensibilidad: celdas convergidas de 244 ===\n")
cat(sprintf("  %-10s %s\n", "umbral", paste(sprintf("%8s", paste0("frac>=", c(0.5,0.8,0.9,1.0))), collapse="")))
for (u in c(0.05, 0.1, 0.25, 0.5, 1.0)) {
  fila <- sapply(c(0.5,0.8,0.9,1.0), function(fr)
    sum(sapply(names(algos), function(al) {
      m <- datos[[al]]
      a <- aggregate(list(ok = m$mejora < u), by = list(inst = m$inst), FUN = mean)
      sum(a$ok >= fr) })))
  cat(sprintf("  %-10s %s\n", sprintf("%.2f%%", u), paste(sprintf("%8d", fila), collapse="")))
}

cat("\n=== RPD medio: TODAS las instancias vs SOLO las convergidas ===\n")
for (al in names(algos)) {
  r <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  r$rpd <- 100*(r$ecmax - r$lb)/r$lb
  ok <- conv[[al]]$inst[conv[[al]]$conv]
  todo <- sapply(arms, function(a) mean(r$rpd[r$arm == a]))
  solo <- if (length(ok)) sapply(arms, function(a) mean(r$rpd[r$arm == a & r$inst %in% ok])) else rep(NA, 7)
  cat(sprintf("\n  %s  (%d de %d instancias convergen)\n", algos[[al]], length(ok), nrow(conv[[al]])))
  cat(sprintf("    %-12s %s\n", "brazo",       paste(sprintf("%7s", arms), collapse="")))
  cat(sprintf("    %-12s %s\n", "todas",       paste(sprintf("%7.2f", todo), collapse="")))
  cat(sprintf("    %-12s %s\n", "convergidas", paste(sprintf("%7.2f", solo), collapse="")))
  if (length(ok) > 1)
    cat(sprintf("    mejor brazo: %s (todas) / %s (convergidas)\n",
        arms[which.min(todo)], arms[which.min(solo)]))
}

out <- do.call(rbind, lapply(names(conv), function(a)
  data.frame(algo = a, conv[[a]][, c("inst","clase","mejora","frac_ok","conv")])))
write.csv(out, "final/convergencia.csv", row.names = FALSE)
cat(sprintf("\nescrito: final/convergencia.csv (%d filas)\n", nrow(out)))
