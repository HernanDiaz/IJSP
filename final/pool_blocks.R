# ¿Se ve estructura de 3 bloques (342/341/341) en el pool v2, y no en los otros?
lee <- function(f) {
  x <- readLines(f)
  as.numeric(sub("^.*, *([0-9.]+)\\]$", "\\1", x))     # extremo superior
}
prueba <- function(inst, gen) {
  f <- sprintf("pools_test/corrected/int__%s_%s_repo_pool.csv", inst, gen)
  if (!file.exists(f)) return(invisible(NULL))
  up <- lee(f)
  if (any(is.na(up))) { cat(sprintf("%-14s %-9s parseo fallido\n", inst, gen)); return(invisible(NULL)) }
  b <- c(rep(1,342), rep(2,341), rep(3,341))[seq_along(up)]
  m <- tapply(up, b, mean)
  p <- kruskal.test(up ~ factor(b))$p.value
  cat(sprintf("%-14s %-9s %.1f / %.1f / %.1f   p=%.2e %s\n",
      inst, gen, m[1], m[2], m[3], p,
      ifelse(p < 0.001, "<- BLOQUES DISTINTOS", "")))
}
cat("Media del extremo superior por tercio del fichero (342/341/341)\n")
cat("Si el pool son 3 bloques de checkpoints distintos, los tercios difieren.\n\n")
for (inst in c("tai30_20_01","tai50_20_01","tai15_15_05","tai20_20_07")) {
  for (gen in c("v2","gp","gtmwkr")) prueba(inst, gen)
  cat("\n")
}
