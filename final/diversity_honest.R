# DIVERSIDAD ESTRUCTURAL: lo que estos datos pueden y no pueden sostener.
#
# El analisis anterior (diversity_struct.R) aplicaba un Wilcoxon pareado sobre
# los PUNTOS TEMPORALES de la traza. Eso es pseudorreplicacion por partida
# doble: los puntos de una misma ejecucion estan autocorrelacionados, y las
# trazas ya venian promediadas sobre las 10 ejecuciones. El p<10^-4 que
# producia no mide nada interpretable.
#
# La unidad experimental honesta es la INSTANCIA, y solo hay cuatro. Con n=4
# el p minimo alcanzable por un test de signos bilateral es 0.125, de modo que
# NINGUN contraste puede dar significacion con estos datos. Lo que si puede
# reportarse es la magnitud del efecto y su CONSISTENCIA entre instancias.
#
# Salida: final/diversity_por_instancia.csv
d <- read.csv("final/diversity/hamming.csv", stringsAsFactors = FALSE)

cat("=== Diversidad estructural: diferencia frente al control, por instancia ===\n")
cat("    Hamming medio de la traza (0 = poblacion identica, 1 = maxima)\n")
cat("    positivo = el brazo sembrado es MAS diverso que su control\n\n")

res <- data.frame()
for (al in unique(d$algo)) {
  cat(sprintf("%s\n", toupper(al)))
  ins <- sort(unique(d$inst[d$algo == al]))
  for (ar in setdiff(unique(d$arm[d$algo == al]), "A0")) {
    difs <- sapply(ins, function(i) {
      a0 <- mean(d$hamming[d$algo == al & d$inst == i & d$arm == "A0"])
      xx <- mean(d$hamming[d$algo == al & d$inst == i & d$arm == ar])
      if (is.nan(a0) || is.nan(xx)) NA else 100 * (xx - a0) / a0
    })
    difs <- difs[!is.na(difs)]
    if (!length(difs)) next
    mismo <- sum(difs > 0)
    # test de signos exacto sobre las instancias: lo maximo que permite n=4
    p <- if (length(difs) > 0) binom.test(mismo, length(difs))$p.value else NA
    cat(sprintf("  %-4s  %s   media %+6.1f%%   %d/%d en la misma direccion, p=%.3f\n",
                ar, paste(sprintf("%+6.1f", difs), collapse = " "),
                mean(difs), mismo, length(difs), p))
    res <- rbind(res, data.frame(algo = al, arm = ar, n_inst = length(difs),
                                 media = mean(difs), mismo_signo = mismo, p_signos = p))
  }
  cat("\n")
}
write.csv(res, "final/diversity_por_instancia.csv", row.names = FALSE)
cat("Con n=4 instancias el p minimo posible es 0.125: la evidencia disponible\n")
cat("es el TAMANO del efecto y que las cuatro instancias coincidan en signo,\n")
cat("no una prueba de significacion.\n")
cat("\nescrito: final/diversity_por_instancia.csv\n")
