# ¿Sostiene la diversidad ESTRUCTURAL la hipotesis de incompatibilidad con JOX?
# Hamming medio de la poblacion (0 = identicos, 1 = maxima diferencia posicional).
# Prediccion de la hipotesis: el GA sembrado arranca MUCHO menos diverso y se
# mantiene por debajo del control (padres con subsecuencias comunes -> JOX
# devuelve hijos casi iguales). ABCE3 no depende de recombinacion -> menos efecto.
d <- read.csv("final/diversity/hamming.csv", stringsAsFactors = FALSE)
cat("=== Hamming medio de la poblacion, por algoritmo y brazo ===\n")
cat(sprintf("%-7s %-5s %9s %9s %9s   %s\n", "algo","arm","inicio","medio","final","caida"))
for (al in c("ga","abce3")) for (ar in c("A0","V2","MIX")) {
  s <- d[d$algo == al & d$arm == ar, ]
  if (!nrow(s)) next
  ini <- mean(sapply(unique(s$inst), function(i) { x <- s[s$inst==i,]; x$hamming[which.min(x$t)] }))
  fin <- mean(sapply(unique(s$inst), function(i) { x <- s[s$inst==i,]; x$hamming[which.max(x$t)] }))
  cat(sprintf("%-7s %-5s %9.4f %9.4f %9.4f   %+.1f%%\n", al, ar, ini, mean(s$hamming), fin,
              100*(fin-ini)/ini))
}

cat("\n=== contraste sembrado vs control (mismo instante, misma instancia) ===\n")
for (al in c("ga","abce3")) {
  a0 <- d[d$algo==al & d$arm=="A0", ]
  for (ar in c("V2","MIX")) {
    x <- d[d$algo==al & d$arm==ar, ]
    m <- merge(a0, x, by=c("inst","t"), suffixes=c(".a0",".s"))
    if (!nrow(m)) next
    rel <- 100*(m$hamming.s - m$hamming.a0)/m$hamming.a0
    p <- suppressWarnings(wilcox.test(m$hamming.s, m$hamming.a0, paired=TRUE)$p.value)
    cat(sprintf("  %-6s %-4s vs A0: n=%2d  diferencia media %+6.1f%%  (p=%.4f) %s\n",
                al, ar, nrow(m), mean(rel), p,
                ifelse(p<0.05, ifelse(mean(rel)<0, "<- sembrado MENOS diverso", "<- sembrado MAS diverso"), "")))
  }
}
cat("\n(Hamming en [0,1]: 0 = poblacion identica, 1 = maxima diferencia posicional)\n")
