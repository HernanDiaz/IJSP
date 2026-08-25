# RPD con la NUEVA referencia: cota inferior publicada del problema crisp.
# Justificacion (verificada en final/check_midpoint.sh sobre 57.450 operaciones):
# los intervalos son exactamente simetricos alrededor de la duracion crisp y las
# rutas coinciden, luego mid(Cmax_int(x)) >= Cmax_crisp(x) para toda x y por tanto
# E[Cmax]* >= optimo crisp >= LB publicada. Es una cota inferior VALIDA, no una
# "mejor conocida" del propio grupo.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)
bk <- read.csv("final/refs.csv",  stringsAsFactors = FALSE)
m  <- merge(lb, bk, by = "inst", all.x = TRUE)

cat("=== la referencia antigua (best_known) frente a la cota inferior publicada ===\n")
m$gap <- (m$best_known - m$lb) / m$lb * 100
cat(sprintf("instancias con ambas referencias: %d\n", sum(!is.na(m$best_known))))
cat(sprintf("best_known por DEBAJO de la LB publicada (imposible -> referencia blanda): %d\n",
            sum(!is.na(m$best_known) & m$best_known < m$lb)))
cat(sprintf("holgura best_known vs LB:  mediana %+.2f%%   rango [%+.2f%%, %+.2f%%]\n\n",
            median(m$gap, na.rm = TRUE), min(m$gap, na.rm = TRUE), max(m$gap, na.rm = TRUE)))

algos <- c("ga", "abce3", "feabcls", "tsn2")
arms  <- c("A0", "V2H", "V2", "MOR", "GT", "GP", "MIX")
cat("=== RPD medio (%) sobre la cota inferior publicada ===\n")
cat(sprintf("%-8s %6s %s\n", "algo", "n_ins", paste(sprintf("%8s", arms), collapse = "")))
for (al in algos) {
  f <- sprintf("final/phase2/results_%s.csv", al); if (!file.exists(f)) next
  r <- read.csv(f, stringsAsFactors = FALSE)
  r <- merge(r, lb[, c("inst", "lb")], by = "inst")
  r$rpd <- (r$ecmax - r$lb) / r$lb * 100
  # media por instancia y brazo, luego media entre instancias comunes
  ag <- aggregate(rpd ~ inst + arm, data = r, FUN = mean)
  wide <- reshape(ag, idvar = "inst", timevar = "arm", direction = "wide")
  ok <- complete.cases(wide[, intersect(paste0("rpd.", arms), names(wide))])
  vals <- sapply(arms, function(a) {
    cn <- paste0("rpd.", a); if (!cn %in% names(wide)) return(NA)
    mean(wide[ok, cn]) })
  cat(sprintf("%-8s %6d %s\n", al, sum(ok), paste(sprintf("%8.2f", vals), collapse = "")))
}
cat("\n(brazos sin datos aparecen como NA; n_ins = instancias con TODOS los brazos)\n")
