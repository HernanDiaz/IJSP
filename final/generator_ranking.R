# ¿CUAL ES LA MEJOR ESTRATEGIA DE SIEMBRA, Y DEPENDE DEL ALGORITMO?
# Protocolo prometido en la seccion de estadistica: Friedman sobre los 7 brazos
# con las 61 instancias como bloques, mas post-hoc de Holm contra el mejor brazo.
# Y la pregunta clave: ¿es el mismo orden en los cuatro solvers? -> W de Kendall
# sobre los cuatro vectores de rangos medios.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
arms  <- c("A0","V2H","V2","MOR","GT","GP","MIX","MIXH")

R <- list()
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  m <- aggregate(ecmax ~ inst + arm, data = d, FUN = mean)
  w <- reshape(m, idvar = "inst", timevar = "arm", direction = "wide")
  cn <- paste0("ecmax.", arms)
  M  <- as.matrix(w[, cn]); colnames(M) <- arms
  M  <- M[complete.cases(M), , drop = FALSE]
  fr <- friedman.test(M)
  rk <- colMeans(t(apply(M, 1, rank)))            # rango medio (1 = mejor)
  R[[al]] <- rk
  cat(sprintf("\n=== %s  (n=%d instancias) ===\n", algos[[al]], nrow(M)))
  cat(sprintf("Friedman: chi2=%.1f, df=%d, p=%.3g\n", fr$statistic, fr$parameter, fr$p.value))
  o <- order(rk)
  cat(sprintf("%-5s %8s   %s\n", "arm", "rango", "post-hoc vs mejor (Wilcoxon pareado, Holm)"))
  best <- names(rk)[o[1]]
  pv <- sapply(arms, function(a) if (a == best) NA else
                 suppressWarnings(wilcox.test(M[, a], M[, best], paired = TRUE)$p.value))
  pa <- p.adjust(pv[!is.na(pv)], method = "holm")
  for (a in names(rk)[o]) {
    tag <- if (a == best) "<- MEJOR" else sprintf("p=%.4f %s", pa[a],
              ifelse(pa[a] < 0.05, "(peor, significativo)", "(indistinguible del mejor)"))
    cat(sprintf("%-5s %8.2f   %s\n", a, rk[a], tag))
  }
}

cat("\n\n=== ¿DEPENDE DEL ALGORITMO? rangos medios lado a lado ===\n")
T <- do.call(rbind, R); rownames(T) <- unname(algos)
print(round(T, 2))
w <- cor(t(T), method = "spearman")
cat(sprintf("\nconcordancia entre solvers (Spearman medio entre pares): %.3f\n",
            mean(w[upper.tri(w)])))
cat("correlacion de rangos por pares:\n"); print(round(w, 2))
cat(sprintf("\nmejor brazo por solver: %s\n",
            paste(sprintf("%s=%s", unname(algos), colnames(T)[apply(T, 1, which.min)]), collapse = ", ")))
