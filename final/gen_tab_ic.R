# Genera el cuerpo de la tabla de tamanos de efecto del articulo.
#
# TODO sale de ficheros: los recuentos de wl_<algo>.csv (phase2_stats.R) y los
# efectos, intervalos y p de efectos_ic.csv (stats_robusto.R). Nada codificado
# a mano. Dos veces se colaron valores obsoletos en esa tabla al anadir un
# brazo, porque las filas antiguas se habian escrito a mano: el Holm dentro de
# cada instancia depende del numero de brazos sembrados, asi que anadir uno
# cambia todos los recuentos y todos los p ajustados.
#
# Salida: final/paper/tab_ic.inc, para empalmar en main.tex.
BS <- "\\"
ic <- read.csv("final/efectos_ic.csv", stringsAsFactors = FALSE)
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
# La familia confirmatoria se marca aparte: su p lleva Holm sobre los seis,
# la del brazo exploratorio va sin ajustar y se senala como tal en la tabla.
confirmatorio <- c("V2H","V2","MOR","GT","GP","MIX")
arms <- c(confirmatorio, "MIXH")

out <- c()
for (al in names(algos)) {
  wl <- read.csv(sprintf("final/wl_%s.csv", al), stringsAsFactors = FALSE)
  sub <- ic[ic$algo == al, ]
  stopifnot(setequal(sub$arm, arms), setequal(wl$arm, arms))
  # La negrita marca al mejor de la familia CONFIRMATORIA. Un brazo elegido
  # despues de ver los resultados no debe decidir quien gana una tabla
  # preespecificada, aunque su cifra sea mejor.
  best <- min(sub$dif[sub$arm %in% confirmatorio])
  out <- c(out, sprintf("%smultirow{%d}{*}{%s}", BS, length(arms), algos[[al]]))
  for (a in arms) {
    r <- sub[sub$arm == a, ]; w <- wl[wl$arm == a, ]
    st <- if (r$p_holm < 0.001) "***" else if (r$p_holm < 0.01) "**" else
          if (r$p_holm < 0.05) "*" else ""
    pv <- if (r$p_holm < 0.001) sprintf("$<$0.001%s", st) else sprintf("%.3f%s", r$p_holm, st)
    dif <- sprintf("%+.2f", r$dif)
    if (abs(r$dif - best) < 1e-9) dif <- sprintf("%stextbf{%s}", BS, dif)
    etq <- if (a %in% confirmatorio) a else sprintf("%s$^{%sdagger}$", a, BS)
    out <- c(out, sprintf(" & %-14s & %d/%d/%d & %.2f & %s & $[%+.2f,%+.2f]$ & %s %s%s",
                          etq, w$W, w$T, w$L, w$A12, dif, r$lo, r$hi, pv, BS, BS))
  }
  if (al != names(algos)[length(algos)]) out <- c(out, paste0(BS, "midrule"))
}
writeLines(out, "final/paper/tab_ic.inc")
cat(sprintf("escrito final/paper/tab_ic.inc (%d lineas)\n", length(out)))
