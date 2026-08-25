# Comprobacion del dato que afirmo el paper: si se excluye el brazo de control,
# ¿sube la concordancia de rangos entre GA y ABCE3?
arms <- c("V2H","V2","MOR","GT","GP","MIX")   # solo brazos sembrados
R <- list()
for (al in c("ga","abce3","feabcls","tsn2")) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  m <- aggregate(ecmax ~ inst + arm, data = d, FUN = mean)
  w <- reshape(m, idvar = "inst", timevar = "arm", direction = "wide")
  M <- as.matrix(w[, paste0("ecmax.", arms)]); colnames(M) <- arms
  M <- M[complete.cases(M), , drop = FALSE]
  R[[al]] <- colMeans(t(apply(M, 1, rank)))
}
T <- do.call(rbind, R)
cat("=== rangos medios SIN el control ===\n"); print(round(T, 2))
s <- cor(t(T), method = "spearman")
cat(sprintf("\nSpearman medio entre solvers: %.3f\n", mean(s[upper.tri(s)])))
cat(sprintf("GA vs ABCE3: %.3f\n", s["ga","abce3"]))
