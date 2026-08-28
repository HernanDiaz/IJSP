# INTERACCION DEL DISENO 2x2: composicion x fraccion.
#
# Los cuatro brazos forman un factorial completo:
#            fraccion 100%   fraccion 50%
#   v2 puro        V2             V2H
#   mezcla         MIX            MIXH
#
# Que MIXH supere a cada uno de sus componentes NO demuestra que los efectos
# se sumen: es compatible con aditividad, con sinergia y con un umbral. Lo que
# distingue esos casos es la INTERACCION, que aqui se estima y se contrasta en
# vez de afirmarse.
#
#   efecto de fraccion sobre v2   = V2H  - V2
#   efecto de fraccion sobre mezcla = MIXH - MIX
#   interaccion (dif en dif)      = (MIXH - MIX) - (V2H - V2)
#
# Interaccion 0 => los dos factores se suman (modelo aditivo).
# Interaccion < 0 => la combinacion rinde MAS que la suma (sinergia).
# Interaccion > 0 => menos que la suma (rendimientos decrecientes).
#
# Todo emparejado por instancia (n=61), IC por bootstrap remuestreando
# instancias, p por Wilcoxon de rangos con signo sobre las 61 diferencias.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
set.seed(20260828)
B <- 10000

ic <- function(x) {
  n <- length(x)
  M <- matrix(sample(x, n * B, replace = TRUE), nrow = n)
  unname(quantile(colMeans(M), c(0.025, 0.975)))
}

cat("=== Interaccion composicion x fraccion (puntos de RPD) ===\n")
cat("    negativo = mejor. Interaccion ~ 0 => aditivo.\n\n")
cat(sprintf("%-8s %14s %14s %16s %9s\n",
            "solver", "frac|v2", "frac|mezcla", "interaccion", "p"))
res <- data.frame()
for (al in names(algos)) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  d$rpd <- 100 * (d$ecmax - d$lb) / d$lb
  m <- aggregate(rpd ~ inst + arm, d, mean)
  w <- reshape(m, idvar = "inst", timevar = "arm", direction = "wide")
  fv <- w$rpd.V2H  - w$rpd.V2        # efecto de media poblacion sobre v2
  fm <- w$rpd.MIXH - w$rpd.MIX       # efecto de media poblacion sobre la mezcla
  it <- fm - fv                      # interaccion
  ci <- ic(it)
  p  <- suppressWarnings(wilcox.test(it)$p.value)
  cat(sprintf("%-8s %+13.3f %+13.3f %+8.3f [%+.3f,%+.3f] %9.4f%s\n",
              algos[[al]], mean(fv), mean(fm), mean(it), ci[1], ci[2], p,
              ifelse(p < 0.05, " *", "")))
  res <- rbind(res, data.frame(algo = al, frac_v2 = mean(fv), frac_mix = mean(fm),
                               inter = mean(it), lo = ci[1], hi = ci[2], p = p))
}
write.csv(res, "final/interaccion_2x2.csv", row.names = FALSE)
cat("\nLectura: si el IC de la interaccion contiene 0, los datos son")
cat(" compatibles\ncon un modelo aditivo pero no lo demuestran; solo excluyen")
cat(" una interaccion\ngrande. Es lo mas que este diseno puede sostener.\n")
cat("\nescrito: final/interaccion_2x2.csv\n")
