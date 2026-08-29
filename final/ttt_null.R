# ¿CONTROLA EL TEST DE SIGNOS EL ERROR DE TIPO I CON UN OBJETIVO ENDOGENO?
#
# Objecion (valida) a ttt_paired.R: el objetivo de cada instancia se construye a
# partir del endpoint del PROPIO control, de modo que A0 se evalua contra una
# diana extraida de su propia realizacion y el brazo sembrado no. Que la
# truncatura no invierta signos no basta para justificar el nulo binomial
# P(antes) = P(despues) = 0.5: hay que demostrar la intercambiabilidad, o
# medirla.
#
# Aqui se mide, sin suponer nada, y REPRODUCIENDO LA ESTRUCTURA DEL ANALISIS
# REAL: alli se comparan dos curvas promediadas sobre ejecuciones, no dos
# ejecuciones sueltas, y el promediado cambia el ruido.
#
#   por instancia: las 30 ejecuciones de A0 se parten al azar en dos mitades de
#   15; cada mitad se promedia sobre la misma rejilla; una hace de "control" y
#   fija el objetivo endpoint*1.005 en makespan, la otra de "brazo". Se forma la
#   diferencia emparejada y se aplica el test de signos sobre las 61 instancias.
#
# Ambas mitades proceden del MISMO proceso, luego H0 es cierta por construccion.
# Si el procedimiento es valido, la proporcion de "antes" debe rondar 0.5 y el
# test debe rechazar el 5% de las veces a alfa = 0.05. Toda desviacion es sesgo
# del objetivo endogeno, no efecto de la siembra.
set.seed(20260826)
# DOS TANDAS INDEPENDIENTES. La primera ESTIMA la proporcion nula p0; la
# segunda MIDE con que frecuencia rechaza el test que realmente se usa, es
# decir el que contrasta contra ese p0. La version anterior media el rechazo
# contra 0.5 y luego aplicaba p0 en el analisis: validaba un procedimiento
# distinto del que se ejecutaba. Usar tandas separadas evita ademas estimar y
# validar sobre las mismas replicas.
B_EST <- 500        # replicas para estimar p0
B_VAL <- 500        # replicas independientes para medir el error de tipo I
NG <- 200           # puntos de rejilla por instancia
ALPHA <- 0.05
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")

cat("=== Error de tipo I del test de signos bajo objetivo endogeno ===\n")
cat(sprintf("    %d replicas para estimar p0 + %d independientes para validar\n", B_EST, B_VAL))
cat("    cada replica parte las 30 ejecuciones de A0 en 15+15 y promedia:\n")
cat("    H0 cierta por construccion, ambas mitades salen del mismo proceso\n")
cat("    el procedimiento es valido si el rechazo contra p0 ronda el 5%\n\n")
cat(sprintf("  %-8s %6s %13s %13s %14s %11s\n",
            "solver","n inst","p0 estimado","rechazo vs p0","rechazo vs 0.5","veredicto"))

res <- data.frame()
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/a0_traces_%s.csv", al), stringsAsFactors = FALSE)
  insts <- unique(d$inst)

  # Por instancia: matriz V de 30 ejecuciones x NG puntos de rejilla, con el
  # mejor-hasta-ahora de cada ejecucion evaluado en la rejilla. La rejilla
  # empieza cuando TODAS las ejecuciones han reportado, para que la media no
  # cambie de composicion (mismo criterio que resample_anytime.sh).
  mats <- list()
  for (i in insts) {
    x <- d[d$inst == i, ]
    rs <- split(x[, c("t","v")], x$run)
    if (length(rs) < 4) next
    t0 <- max(sapply(rs, function(z) min(z$t)))
    t1 <- max(sapply(rs, function(z) max(z$t)))
    if (!(t1 > t0)) next
    G <- seq(t0, t1, length.out = NG)
    V <- t(sapply(rs, function(z) {
      z <- z[order(z$t), ]
      z$v[pmax(1, findInterval(G, z$t))]
    }))
    mats[[i]] <- V
  }
  ok_inst <- names(mats)

  # una replica bajo H0: parte las 30 ejecuciones del control en 15+15,
  # promedia cada mitad y aplica la construccion de objetivo endogeno
  replica <- function() {
    dif <- numeric(0)
    for (i in ok_inst) {
      V <- mats[[i]]; n <- nrow(V)
      idx <- sample(n); h1 <- idx[1:floor(n/2)]; h2 <- idx[(floor(n/2)+1):n]
      c1 <- colMeans(V[h1, , drop = FALSE])       # "control"
      c2 <- colMeans(V[h2, , drop = FALSE])       # "brazo"
      obj <- c1[NG] * 1.005
      j1 <- which(c1 <= obj); j2 <- which(c2 <= obj)
      T1 <- if (length(j1)) j1[1]/NG else 1.0
      T2 <- if (length(j2)) j2[1]/NG else 1.0
      dif <- c(dif, T2 - T1)
    }
    c(antes = sum(dif < 0), desp = sum(dif > 0))
  }

  # TANDA 1: estimar p0
  a1 <- 0; d1 <- 0
  for (b in 1:B_EST) { r <- replica(); a1 <- a1 + r["antes"]; d1 <- d1 + r["desp"] }
  pa <- unname(a1 / (a1 + d1))

  # TANDA 2, independiente: con que frecuencia rechaza el test QUE SE USA,
  # es decir el binomial contra p0, y como referencia el ingenuo contra 0.5
  rech <- 0; rech05 <- 0; usadas <- 0
  for (b in 1:B_VAL) {
    r <- replica(); a <- unname(r["antes"]); p <- unname(r["desp"])
    if (a + p > 0) {
      usadas <- usadas + 1
      if (binom.test(a, a + p, p = pa)$p.value < ALPHA) rech <- rech + 1
      if (binom.test(a, a + p)$p.value      < ALPHA) rech05 <- rech05 + 1
    }
  }
  tr <- rech / usadas; tr05 <- rech05 / usadas
  ver <- if (abs(tr - ALPHA) < 0.025) "calibrado" else "SESGADO"
  cat(sprintf("  %-8s %6d %13.1f%% %13.1f%% %12.1f%% %11s\n",
              algos[[al]], length(ok_inst), 100*pa, 100*tr, 100*tr05, ver))
  res <- rbind(res, data.frame(algo = al, n_inst = length(ok_inst),
                               p_antes_nulo = pa, tipo_I = tr, tipo_I_vs_05 = tr05))
}
write.csv(res, "final/ttt_null.csv", row.names = FALSE)
cat("\nescrito: final/ttt_null.csv\n")
