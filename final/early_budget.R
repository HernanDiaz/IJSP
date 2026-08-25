# Calidad a presupuestos TEMPRANOS: RPD medio en fracciones del presupuesto de
# la celda (5%, 10%, 25%, 50%, 100%). Es la forma correcta de evaluar la ventaja
# anytime: si la siembra ayuda, se vera en los cortes tempranos aunque el final
# sea un empate.
# Uso: Rscript early_budget.R <algo>
args <- commandArgs(trailingOnly = TRUE); algo <- args[1]
a <- read.csv(sprintf("final/phase2/anytime_%s.csv", algo), stringsAsFactors = FALSE)
r <- read.csv(sprintf("final/phase2/results_%s.csv", algo), stringsAsFactors = FALSE)
refs <- read.csv("final/refs.csv", stringsAsFactors = FALSE)
cl <- unique(r[, c("inst","clase")])
a <- merge(merge(a, cl, by = "inst"), refs, by = "inst")
a$rpd <- 100 * (a$bestcmax - a$best_known) / a$best_known
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
fracs <- c(0.05, 0.10, 0.25, 0.50, 1.00)

# valor de cada brazo en cada fraccion del presupuesto (ultimo valor <= t objetivo)
val_at <- function(df, tt) { df <- df[order(df$t), ]; i <- which(df$t <= tt); if (!length(i)) return(NA); df$rpd[i[length(i)]] }

out <- data.frame()
for (ins in unique(a$inst)) {
  s <- a[a$inst == ins, ]
  tmax <- max(s$t)
  for (f in fracs) for (ar in arms) {
    x <- s[s$arm == ar, ]; if (!nrow(x)) next
    out <- rbind(out, data.frame(inst = ins, clase = s$clase[1], arm = ar,
                                 frac = f, rpd = val_at(x, f * tmax)))
  }
}
# CORRECCION de composicion: nos quedamos SOLO con las instancias que tienen
# valor valido en TODAS las fracciones y para TODOS los brazos. Sin esto, cada
# columna promedia sobre un subconjunto distinto y la serie sale no monotona
# (p.ej. RPD del control 6.83% al 5% y 11.85% al 10%, lo cual es imposible).
ok <- c()
for (ins in unique(out$inst)) {
  s <- out[out$inst == ins, ]
  if (!any(is.na(s$rpd)) && nrow(s) == length(fracs) * length(arms)) ok <- c(ok, ins)
}
cat(sprintf("instancias con datos completos: %d de %d\n", length(ok), length(unique(out$inst))))
out <- out[out$inst %in% ok, ]

cat(sprintf("=== %s: RPD medio (%%) a fracciones del presupuesto ===\n", toupper(algo)))
cat(sprintf("%-5s", "brazo")); for (f in fracs) cat(sprintf(" %8s", paste0(f*100, "%"))); cat("\n")
for (ar in arms) {
  cat(sprintf("%-5s", ar))
  for (f in fracs) {
    v <- out$rpd[out$arm == ar & out$frac == f]
    cat(sprintf(" %8.2f", mean(v, na.rm = TRUE)))
  }
  cat("\n")
}
cat("\n=== ventaja sobre el control (puntos de RPD; negativo = el sembrado es mejor) ===\n")
cat(sprintf("%-5s", "brazo")); for (f in fracs) cat(sprintf(" %8s", paste0(f*100, "%"))); cat("\n")
for (ar in arms[-1]) {
  cat(sprintf("%-5s", ar))
  for (f in fracs) {
    d <- c()
    for (ins in unique(out$inst)) {
      a0 <- out$rpd[out$inst == ins & out$arm == "A0"  & out$frac == f]
      xx <- out$rpd[out$inst == ins & out$arm == ar    & out$frac == f]
      if (length(a0) && length(xx)) d <- c(d, xx - a0)
    }
    cat(sprintf(" %8.2f", mean(d, na.rm = TRUE)))
  }
  cat("\n")
}
write.csv(out, sprintf("final/tables/%s_early.csv", algo), row.names = FALSE)
