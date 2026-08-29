# ¿QUE CELDAS SATISFACEN LA REGLA DE MESETA Y CUALES NO?
#
# Regla del diseno: una celda converge si la mejora del objetivo (E[Cmax], no
# del RPD) en el ultimo 10% del presupuesto es < 0.1%.
#
# TRES DEFECTOS DE VERSIONES ANTERIORES, todos corregidos aqui:
#
# 1. Se usaba como valor final el ULTIMO PUNTO DE LA CURVA promediada, que es
#    una instantanea de poblacion. Aqui el valor final es el DEVUELTO por el
#    solver (results_<algo>.csv).
#
# 2. Se aplicaba la regla a la curva PROMEDIADA sobre las 30 ejecuciones, lo
#    que puede ocultar que un subconjunto sigue mejorando. Aqui se evalua
#    EJECUCION A EJECUCION y se agrega con un criterio declarado (>=90%).
#
# 3. Solo se comprobaba el CONTROL. Una instancia en la que converge A0 no es
#    un experimento en el que hayan convergido todos los brazos: el brazo
#    sembrado puede seguir mejorando, que es precisamente lo que predice la
#    explicacion de "consolidacion lenta" del GA. Ademas, condicionar por el
#    comportamiento de A0 selecciona instancias usando el resultado de uno de
#    los dos comparandos. Aqui se comprueba CADA BRAZO y las comparaciones
#    restringidas se hacen sobre la INTERSECCION: instancias donde convergen a
#    la vez el control y el brazo con el que se compara.
#
# SENTIDO DEL SESGO RESIDUAL. v90 es el minimo acumulado en t <= 0.9T, que es
# >= al incumbente real en ese instante. La mejora medida es por tanto una cota
# SUPERIOR de la real: la regla puede declarar "no convergida" una celda que si
# converge, nunca al reves.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
clases <- c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX","MIXH")
seeded <- setdiff(arms, "A0")
UMBRAL <- 0.1     # % de mejora en el ultimo 10% del presupuesto
FRAC   <- 0.90    # fraccion de ejecuciones que deben cumplirla

cl <- unique(read.csv("final/phase2/results_ga.csv", stringsAsFactors = FALSE)[, c("inst","clase")])

# mejora por ejecucion de un (solver, brazo), con el valor final DEVUELTO
por_ejec <- function(al, arm) {
  suf <- if (arm == "A0") "" else paste0("_", arm)
  f <- sprintf("final/phase2/conv_runs_%s%s.csv", al, suf)
  if (!file.exists(f)) return(NULL)
  cv <- read.csv(f, stringsAsFactors = FALSE)
  rs <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  rs <- rs[rs$arm == arm, c("inst","run","ecmax")]
  m <- merge(cv, rs, by = c("inst","run"))
  m$mejora <- 100 * (m$v90 - m$ecmax) / m$ecmax
  m
}
# clasificacion por instancia de un (solver, brazo)
clasifica <- function(m, u = UMBRAL, fr = FRAC) {
  a <- aggregate(list(ok = m$mejora < u), by = list(inst = m$inst), FUN = mean)
  data.frame(inst = a$inst, frac_ok = a$ok, conv = a$ok >= fr,
             stringsAsFactors = FALSE)
}

datos <- list(); conv <- list()
for (al in names(algos)) {
  datos[[al]] <- list(); conv[[al]] <- list()
  for (a in arms) {
    m <- por_ejec(al, a)
    if (is.null(m)) next
    datos[[al]][[a]] <- m
    conv[[al]][[a]] <- clasifica(m)
  }
}

cat("=== Regla de meseta por ejecucion, CONTROL (A0) ===\n")
cat(sprintf("    converge si >= %.0f%% de las 30 ejecuciones mejoran < %.1f%%\n\n", 100*FRAC, UMBRAL))
cat(sprintf("%-8s %-8s %6s %11s %11s   %s\n", "solver","clase","n","mejora med","% ejec ok","veredicto"))
for (al in names(algos)) {
  e <- merge(conv[[al]][["A0"]], cl, by = "inst")
  md <- aggregate(mejora ~ inst, data = datos[[al]][["A0"]], FUN = median)
  e <- merge(e, md, by = "inst")
  for (k in clases) {
    x <- e[e$clase == k, ]; if (nrow(x) == 0) next
    nc <- sum(x$conv)
    cat(sprintf("%-8s %-8s %6d %10.3f%% %10.0f%%   %s\n", algos[[al]], k, nrow(x),
        median(x$mejora), 100*mean(x$frac_ok),
        if (nc == nrow(x)) "converge" else sprintf("*** %d/%d ANYTIME-ONLY ***", nrow(x)-nc, nrow(x))))
  }
  cat("\n")
}

cat("=== celdas anytime-only del control ===\n")
tot <- 0
for (al in names(algos)) {
  n <- sum(!conv[[al]][["A0"]]$conv); tot <- tot + n
  cat(sprintf("  %-8s %2d de 61\n", algos[[al]], n))
}
cat(sprintf("  TOTAL: %d de %d celdas de control\n\n", tot, 4*61))

cat("=== Convergencia de CADA BRAZO, no solo del control ===\n")
cat(sprintf("  %-8s %s\n", "solver", paste(sprintf("%7s", arms), collapse="")))
for (al in names(algos)) {
  v <- sapply(arms, function(a) if (is.null(conv[[al]][[a]])) NA else sum(conv[[al]][[a]]$conv))
  cat(sprintf("  %-8s %s\n", algos[[al]], paste(sprintf("%7s", v), collapse="")))
}
cat("  (numero de las 61 instancias en que ese brazo cumple la regla)\n\n")

cat("=== Comparacion restringida a la INTERSECCION (convergen A0 y el brazo) ===\n")
inter <- data.frame()
for (al in names(algos)) {
  r <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  r$rpd <- 100*(r$ecmax - r$lb)/r$lb
  cA <- conv[[al]][["A0"]]
  cat(sprintf("\n  %s\n", algos[[al]]))
  cat(sprintf("    %-5s %6s %10s %10s %9s\n", "brazo", "n ambos", "A0", "brazo", "dif"))
  for (a in seeded) {
    cb <- conv[[al]][[a]]; if (is.null(cb)) next
    ok <- intersect(cA$inst[cA$conv], cb$inst[cb$conv])
    if (!length(ok)) { cat(sprintf("    %-5s %6d %10s\n", a, 0, "---")); next }
    v0 <- mean(r$rpd[r$arm == "A0" & r$inst %in% ok])
    v1 <- mean(r$rpd[r$arm == a    & r$inst %in% ok])
    cat(sprintf("    %-5s %6d %10.2f %10.2f %+9.2f\n", a, length(ok), v0, v1, v1-v0))
    inter <- rbind(inter, data.frame(algo=al, arm=a, n=length(ok), a0=v0, brazo=v1, dif=v1-v0))
  }
}
write.csv(inter, "final/convergencia_interseccion.csv", row.names = FALSE)

cat("\n\n=== Sensibilidad de las CONCLUSIONES, sobre la INTERSECCION ===\n")
cat("    Las dos conclusiones que dependen de la regla se enuncian sobre la\n")
cat("    interseccion (convergen el control Y el brazo), asi que su\n")
cat("    sensibilidad debe medirse ahi y no sobre el subconjunto del control:\n")
cat("      GA   -> la reversion, MIX y MIXH mejores que A0\n")
cat("      TSN2 -> la ventaja desaparece, MIX y MIXH ya no baten a A0\n")
cat("    Cada celda es la diferencia brazo - A0 (negativo = el brazo gana) y\n")
cat("    entre parentesis el tamano de la interseccion.\n\n")
cat(sprintf("  %-8s %-6s %-8s %s\n", "solver", "brazo", "umbral",
            paste(sprintf("%14s", paste0("frac>=", c(0.5,0.8,0.9,1.0))), collapse="")))
sens <- data.frame()
for (al in c("ga","tsn2")) {
  r <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  r$rpd <- 100*(r$ecmax - r$lb)/r$lb
  for (a in c("MIX","MIXH")) {
    for (u in c(0.05, 0.1, 0.25)) {
      fila <- sapply(c(0.5,0.8,0.9,1.0), function(fr) {
        cA <- clasifica(datos[[al]][["A0"]], u, fr)
        cB <- clasifica(datos[[al]][[a]],   u, fr)
        ok <- intersect(cA$inst[cA$conv], cB$inst[cB$conv])
        if (length(ok) < 3) return("      --     ")
        dd <- mean(r$rpd[r$arm == a & r$inst %in% ok]) -
              mean(r$rpd[r$arm == "A0" & r$inst %in% ok])
        sens <<- rbind(sens, data.frame(algo=al, arm=a, umbral=u, frac=fr,
                                        n=length(ok), dif=dd))
        sprintf("%+.2f (%d)", dd, length(ok))
      })
      cat(sprintf("  %-8s %-6s %-8s %s\n",
                  if (u == 0.05 && a == "MIX") algos[[al]] else "",
                  if (u == 0.05) a else "",
                  sprintf("%.2f%%", u), paste(sprintf("%14s", fila), collapse="")))
    }
  }
}
write.csv(sens, "final/convergencia_sensibilidad.csv", row.names = FALSE)

cat("\n=== Aviso: mejoras negativas ===\n")
for (al in names(algos)) {
  m <- datos[[al]][["A0"]]
  n <- sum(m$mejora < 0)
  if (n > 0) cat(sprintf("  %-8s %4d de %d ejecuciones con mejora NEGATIVA (min %.2f%%)\n",
                         algos[[al]], n, nrow(m), min(m$mejora)))
}
cat("  Causa: StatisticsIJSP toma el mejor con isBetterThan, que en TS-N2 es\n")
cat("  LEX2. La traza guarda su incumbente en LEX2, cuyo midpoint no es\n")
cat("  monotono, asi que la 'mejora' medida en E[Cmax] puede salir negativa.\n")
cat("  Esas ejecuciones satisfacen trivialmente una regla que pide mejora\n")
cat("  < 0.1%, por una razon ajena a la convergencia. Se declara en el texto.\n")

out <- do.call(rbind, lapply(names(conv), function(al)
  do.call(rbind, lapply(names(conv[[al]]), function(a)
    data.frame(algo = al, arm = a, conv[[al]][[a]])))))
out <- merge(out, cl, by = "inst")
write.csv(out, "final/convergencia.csv", row.names = FALSE)
cat(sprintf("\nescrito: final/convergencia.csv (%d filas) y final/convergencia_interseccion.csv\n", nrow(out)))
