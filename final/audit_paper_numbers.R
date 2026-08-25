# AUDITORIA DE LAS CIFRAS DEL PAPER: recalcula desde los datos crudos cada numero
# que aparece en las tablas y en el texto, y lo compara con lo escrito.
# Un numero copiado a mano de una version anterior del analisis es el error mas
# facil de cometer y el mas dificil de ver.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")

# --- Tabla 1: RPD medio (lo escrito en main.tex) ---
escrito <- rbind(
  GA      = c(7.52, 7.68, 8.07, 9.69, 8.47, 7.99, 7.58),
  ABCE3   = c(10.52, 9.52, 9.40, 10.73, 10.22, 9.69, 9.39),
  fEABCLS = c(3.61, 3.56, 3.60, 5.08, 4.43, 3.96, 3.59),
  TSN2    = c(3.51, 3.48, 3.57, 4.18, 3.95, 3.62, 3.35))
colnames(escrito) <- arms

cat("=== Tabla 1 (RPD medio): escrito vs recalculado ===\n")
malo <- 0
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  d <- merge(d, lb, by = "inst"); d$rpd <- 100*(d$ecmax - d$lb)/d$lb
  ag <- aggregate(rpd ~ inst + arm, data = d, FUN = mean)
  w <- reshape(ag, idvar="inst", timevar="arm", direction="wide")
  ok <- complete.cases(w[, paste0("rpd.", arms)])
  calc <- sapply(arms, function(a) mean(w[ok, paste0("rpd.", a)]))
  dif <- round(calc, 2) - escrito[algos[[al]], ]
  bad <- names(dif)[abs(dif) > 0.005]
  if (length(bad)) { malo <- malo + length(bad)
    for (b in bad) cat(sprintf("  DISCREPANCIA %s/%s: escrito %.2f, real %.2f\n",
                               algos[[al]], b, escrito[algos[[al]], b], calc[b]))
  }
}
cat(sprintf("  celdas con discrepancia: %d de 28\n\n", malo))

# --- Tabla 2: W/T/L y A12 ---
a12 <- function(x, y) { m<-length(x); n<-length(y); r<-rank(c(x,y)); (sum(r[1:m])/m-(m+1)/2)/n }
esc_wtl <- list(
  ga      = list(V2H=c(10,33,18), V2=c(8,26,27), MOR=c(1,3,57), GT=c(5,15,41), GP=c(9,21,31), MIX=c(13,29,19)),
  abce3   = list(V2H=c(32,29,0), V2=c(36,25,0), MOR=c(2,55,4), GT=c(14,46,1), GP=c(29,32,0), MIX=c(35,26,0)),
  feabcls = list(V2H=c(10,42,9), V2=c(11,37,13), MOR=c(1,5,55), GT=c(3,11,47), GP=c(9,24,28), MIX=c(9,41,11)),
  tsn2    = list(V2H=c(7,50,4), V2=c(5,46,10), MOR=c(8,15,38), GT=c(8,22,31), GP=c(8,38,15), MIX=c(12,46,3)))
cat("=== Tabla 2 (victorias/empates/derrotas): escrito vs recalculado ===\n")
malo2 <- 0
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  seeded <- setdiff(arms, "A0")
  W <- setNames(rep(0, length(seeded)), seeded); L <- W; T <- W
  for (ins in unique(d$inst)) {
    # MISMA CONVENCION QUE phase2_stats.R: ordenar por run antes de emparejar,
    # aproximacion normal (exact=FALSE; los makespans discretos generan empates
    # que impiden el test exacto), y p-valor incalculable = EMPATE (si las 30
    # diferencias son cero, los brazos son indistinguibles: eso ES un empate).
    a0 <- d[d$inst == ins & d$arm == "A0", ]; a0 <- a0[order(a0$run), "ecmax"]
    ps <- sapply(seeded, function(a) {
      x <- d[d$inst == ins & d$arm == a, ]; x <- x[order(x$run), "ecmax"]
      if (length(x) != length(a0) || length(a0) < 2) return(NA)
      tryCatch(wilcox.test(x, a0, paired = TRUE, exact = FALSE)$p.value,
               error = function(e) NA) })
    pa <- p.adjust(ps, method = "holm")
    for (a in seeded) {
      x <- d[d$inst == ins & d$arm == a, ]; x <- x[order(x$run), "ecmax"]
      if (length(x) != length(a0)) next
      if (!is.na(pa[a]) && pa[a] < 0.05) {
        if (mean(x) < mean(a0)) W[a] <- W[a]+1 else L[a] <- L[a]+1
      } else T[a] <- T[a]+1
    }
  }
  for (a in seeded) {
    e <- esc_wtl[[al]][[a]]
    if (!identical(as.integer(c(W[a],T[a],L[a])), as.integer(e))) {
      malo2 <- malo2 + 1
      cat(sprintf("  DISCREPANCIA %s/%s: escrito %d/%d/%d, real %d/%d/%d\n",
                  algos[[al]], a, e[1],e[2],e[3], W[a],T[a],L[a]))
    }
  }
}
cat(sprintf("  brazos con discrepancia: %d de 24\n", malo2))
