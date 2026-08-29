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
# SEPARACION CONFIRMATORIO / EXPLORATORIO.
# Los seis primeros brazos sembrados se fijaron antes de ejecutar el
# experimento y forman la familia confirmatoria: Holm corrige sobre ELLOS.
# MIXH se especifico despues de analizarlos, asi que es exploratorio y NO debe
# entrar en esa familia: incluirlo penalizaba los contrastes preespecificados
# por una decision tomada despues de verlos (GA/V2 pasaba de 0.012 a 0.016 por
# la mera presencia de MIXH). Se reporta aparte, con su p sin corregir, que es
# lo que corresponde a un unico contraste de seguimiento.
confirmatorio <- c("V2H","V2","MOR","GT","GP","MIX")
exploratorio  <- c("MIXH")
seeded <- c(confirmatorio, exploratorio)
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
  # Holm SOLO sobre la familia confirmatoria; el brazo exploratorio conserva su
  # p sin ajustar y no altera la de los demas.
  ph <- p
  ph[confirmatorio] <- p.adjust(p[confirmatorio], method = "holm")
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
# La familia global tambien es SOLO confirmatoria: 4 solvers x 6 brazos.
conf <- todo$arm %in% confirmatorio
todo$p_global <- NA
todo$p_global[conf] <- p.adjust(todo$p[conf], method = "holm")
sig_solver <- sum(todo$p_holm[conf] < 0.05)
sig_global <- sum(todo$p_global[conf] < 0.05)
cat(sprintf("  familia confirmatoria: %d contrastes (%d solvers x %d brazos)\n",
            sum(conf), length(algos), length(confirmatorio)))
cat(sprintf("  significativos con Holm por solver:            %d\n", sig_solver))
cat(sprintf("  significativos con Holm sobre los %d globales: %d\n", sum(conf), sig_global))
cat("  pierden significacion al corregir globalmente:")
perd <- todo[conf & todo$p_holm < 0.05 & todo$p_global >= 0.05, ]
if (nrow(perd)) cat(sprintf(" %s/%s", perd$algo, perd$arm), "\n") else cat(" ninguno\n")
cat(sprintf("\n  exploratorio (fuera de toda familia): MIXH, p sin ajustar\n"))
for (al in names(algos)) {
  r <- todo[todo$algo == al & todo$arm == "MIXH", ]
  if (nrow(r)) cat(sprintf("    %-8s dif=%+.3f [%+.3f,%+.3f] p=%.4f\n",
                           algos[[al]], r$dif, r$lo, r$hi, r$p))
}
write.csv(todo, "final/efectos_ic.csv", row.names = FALSE)
cat("\nescrito: final/efectos_ic.csv\n")
