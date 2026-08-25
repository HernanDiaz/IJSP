# AVISO: la metrica B de este script promedia SOLO sobre las instancias en que
# el brazo alcanza el objetivo, que es precisamente el sesgo que el articulo
# denuncia (acreditar a MOR un 71% del presupuesto usando el 15% de instancias
# donde llega). Se conserva porque es la fuente de la METRICA A y del
# contraejemplo del 71%. El analisis autoritativo de la metrica B es
# final/ttt_paired.R, emparejado por instancia y con los no alcanzadores
# incluidos. No usar la metrica B de aqui para conclusiones.
#
# Velocidad de convergencia en la Fase 2, con las DOS metricas acordadas:
#  (A) t_meseta = tiempo hasta <=1% del valor final PROPIO del brazo
#  (B) t_objA0  = tiempo hasta la calidad final del CONTROL A0 (objetivo comun,
#                 tolerancia +0.5%) -> la metrica justa, solo interpretable
#                 cuando los finales son estadisticamente indistinguibles.
# Ahorro = (t_A0 - t_brazo)/t_A0 * 100  (positivo = el brazo llega antes).
# Uso: Rscript ttt_phase2.R <algo>
args <- commandArgs(trailingOnly = TRUE); algo <- args[1]
a <- read.csv(sprintf("final/phase2/anytime_%s.csv", algo), stringsAsFactors = FALSE)
r <- read.csv(sprintf("final/phase2/results_%s.csv", algo), stringsAsFactors = FALSE)
cl <- unique(r[, c("inst","clase")])
a <- merge(a, cl, by = "inst")
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")

tiempo_hasta <- function(df, umbral) {   # primer t con bestcmax <= umbral
  df <- df[order(df$t), ]
  i <- which(df$bestcmax <= umbral)
  if (!length(i)) return(NA_real_)
  df$t[i[1]]
}

res <- data.frame()
for (ins in unique(a$inst)) {
  s <- a[a$inst == ins, ]
  a0 <- s[s$arm == "A0", ]
  if (!nrow(a0)) next
  fin_a0 <- a0$bestcmax[which.max(a0$t)]
  tA <- tiempo_hasta(a0, fin_a0 * 1.01)         # meseta propia de A0
  tB <- tiempo_hasta(a0, fin_a0 * 1.005)        # objetivo comun
  for (ar in arms) {
    x <- s[s$arm == ar, ]
    if (!nrow(x)) next
    fin <- x$bestcmax[which.max(x$t)]
    res <- rbind(res, data.frame(
      inst = ins, clase = s$clase[1], arm = ar,
      t_meseta = tiempo_hasta(x, fin * 1.01),
      t_objA0  = tiempo_hasta(x, fin_a0 * 1.005),
      ref_meseta = tA, ref_obj = tB, stringsAsFactors = FALSE))
  }
}
res$ahorroA <- 100 * (res$ref_meseta - res$t_meseta) / res$ref_meseta
res$ahorroB <- 100 * (res$ref_obj    - res$t_objA0)  / res$ref_obj

cat(sprintf("=== %s: velocidad de convergencia (mediana sobre instancias) ===\n", toupper(algo)))
cat(sprintf("%-5s %12s %10s %12s %10s %8s\n", "brazo", "t_meseta(s)", "ahorroA", "t_objA0(s)", "ahorroB", "n_alcanza"))
for (ar in arms) {
  x <- res[res$arm == ar, ]
  if (!nrow(x)) next
  cat(sprintf("%-5s %12.1f %9.0f%% %12.1f %9.0f%% %6d/%d\n", ar,
      median(x$t_meseta, na.rm = TRUE), median(x$ahorroA, na.rm = TRUE),
      median(x$t_objA0,  na.rm = TRUE), median(x$ahorroB, na.rm = TRUE),
      sum(!is.na(x$t_objA0)), nrow(x)))
}
cat("\n(ahorroA: llegar a su propia meseta; ahorroB: llegar a la calidad final de A0.\n")
cat(" n_alcanza = en cuantas instancias el brazo llega a alcanzar la calidad de A0)\n")

cat("\n=== ahorroB (%) por clase, mediana ===\n")
clases <- sort(unique(res$clase))
cat(sprintf("%-7s", "clase")); for (ar in arms[-1]) cat(sprintf(" %8s", ar)); cat("\n")
for (cc in clases) {
  cat(sprintf("%-7s", cc))
  for (ar in arms[-1]) {
    v <- res$ahorroB[res$clase == cc & res$arm == ar]
    cat(sprintf(" %8s", ifelse(all(is.na(v)), "-", sprintf("%.0f", median(v, na.rm = TRUE)))))
  }
  cat("\n")
}
write.csv(res, sprintf("final/tables/%s_ttt.csv", algo), row.names = FALSE)
