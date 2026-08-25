# REANALISIS ESTADISTICO segun las objeciones del revisor 2.
#
# Problema del analisis anterior: se hacian 6 contrastes por instancia con
# correccion de Holm SOLO dentro de cada instancia (244 familias separadas,
# hasta 1464 contrastes elementales), y despues se contaban "victorias" sin
# controlar el error global ni cuantificar si ese recuento excede lo esperable
# bajo la hipotesis nula.
#
# Aqui la UNIDAD EXPERIMENTAL es la instancia, de forma consistente:
#   1. por instancia se resume cada brazo con la media de sus 30 ejecuciones;
#   2. la diferencia brazo - control por instancia es el dato (n = 61);
#   3. un unico contraste por (solver, brazo) sobre esas 61 diferencias, con
#      Holm sobre los 6 brazos de cada solver;
#   4. intervalo de confianza por bootstrap remuestreando INSTANCIAS.
# Asi no se mezcla la variabilidad entre ejecuciones con la variabilidad entre
# instancias, que es la que sostiene las conclusiones.
suppressPackageStartupMessages(library(boot))
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
seeded <- c("V2H","V2","MOR","GT","GP","MIX")
set.seed(20260825)
B <- 10000

cat("=== Efecto de la siembra por instancia: diferencia de RPD frente al control ===\n")
cat("    (n=61 instancias; IC 95% por bootstrap percentil remuestreando instancias;\n")
cat("     p de Wilcoxon de rangos con signo sobre las 61 diferencias, Holm por solver)\n\n")
res_all <- list()
for (al in names(algos)) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE),
             lb, by = "inst")
  d$rpd <- 100 * (d$ecmax - d$lb) / d$lb
  m <- aggregate(rpd ~ inst + arm, data = d, FUN = mean)
  w <- reshape(m, idvar = "inst", timevar = "arm", direction = "wide")
  base <- w[["rpd.A0"]]
  difs <- sapply(seeded, function(a) w[[paste0("rpd.", a)]] - base)
  p <- apply(difs, 2, function(x) suppressWarnings(wilcox.test(x)$p.value))
  ph <- p.adjust(p, method = "holm")
  cat(sprintf("%s  (n = %d instancias)\n", algos[[al]], nrow(w)))
  cat(sprintf("  %-5s %9s %20s %10s %10s  %s\n",
              "brazo", "dif media", "IC 95%", "p", "p (Holm)", ""))
  fila <- data.frame()
  for (a in seeded) {
    x <- difs[, a]
    bt <- boot(x, function(v, i) mean(v[i]), R = B)
    ci <- boot.ci(bt, type = "perc")$percent[4:5]
    sig <- if (ph[a] < 0.001) "***" else if (ph[a] < 0.01) "**" else if (ph[a] < 0.05) "*" else ""
    cat(sprintf("  %-5s %+9.3f   [%+7.3f, %+7.3f] %10.4f %10.4f  %s\n",
                a, mean(x), ci[1], ci[2], p[a], ph[a], sig))
    fila <- rbind(fila, data.frame(algo = al, arm = a, dif = mean(x),
                                   lo = ci[1], hi = ci[2], p = p[a], p_holm = ph[a]))
  }
  res_all[[al]] <- fila
  cat("\n")
}
write.csv(do.call(rbind, res_all), "final/efectos_ic.csv", row.names = FALSE)

cat("=== Control de multiplicidad a nivel de estudio ===\n")
todo <- do.call(rbind, res_all)
todo$p_global <- p.adjust(todo$p, method = "holm")          # 24 contrastes
sig_solver <- sum(todo$p_holm < 0.05)
sig_global <- sum(todo$p_global < 0.05)
cat(sprintf("  contrastes: %d (4 solvers x 6 brazos)\n", nrow(todo)))
cat(sprintf("  significativos con Holm por solver (familia de 6): %d\n", sig_solver))
cat(sprintf("  significativos con Holm sobre los %d contrastes:    %d\n", nrow(todo), sig_global))
cat("  brazos que pierden significacion al corregir globalmente:")
perd <- todo[todo$p_holm < 0.05 & todo$p_global >= 0.05, ]
if (nrow(perd)) cat(sprintf(" %s/%s", perd$algo, perd$arm), "\n") else cat(" ninguno\n")
write.csv(todo, "final/efectos_ic.csv", row.names = FALSE)
cat("\nescrito: final/efectos_ic.csv\n")
