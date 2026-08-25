# TIME-TO-TARGET, las dos metricas definidas en la seccion del piloto:
#   A) tiempo hasta quedar a <=1% del valor final DEL PROPIO BRAZO
#      -> mide "cuando deja de mejorar", pero un brazo que converge rapido a
#         una meseta peor sale artificialmente bien.
#   B) tiempo hasta alcanzar la calidad final DEL CONTROL (tolerancia +0.5%)
#      -> mide "cuando llega a donde llega el control"; si un brazo nunca la
#         alcanza, se cuenta como fallo, no como tiempo infinito.
# Se expresa en FRACCION DEL PRESUPUESTO para poder agregar clases con
# presupuestos distintos.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
arms  <- c("A0","V2H","V2","MOR","GT","GP","MIX")

cat(sprintf("%-8s %-4s %10s %10s %10s\n", "solver","arm","TTT-A","TTT-B","alcanza B"))
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors = FALSE)
  d <- merge(d, lb, by = "inst"); d$rpd <- 100*(d$bestcmax - d$lb)/d$lb
  insts <- Reduce(intersect, lapply(arms, function(a) unique(d$inst[d$arm == a])))
  # calidad final del control por instancia
  fin <- function(a, i) { s <- d[d$inst==i & d$arm==a, ]; s$rpd[which.max(s$t)] }
  ctl <- sapply(insts, function(i) fin("A0", i))
  names(ctl) <- insts
  res <- list()
  for (a in arms) {
    tA <- c(); tB <- c(); okB <- 0
    for (i in insts) {
      s <- d[d$inst==i & d$arm==a, ]; s <- s[order(s$t), ]
      if (nrow(s) < 3) next
      T <- max(s$t); own <- s$rpd[nrow(s)]
      # A: primer instante con rpd <= own*1.01
      jA <- which(s$rpd <= own * 1.01)
      if (length(jA)) tA <- c(tA, s$t[jA[1]] / T)
      # B: primer instante con rpd <= control_final + 0.5% relativo
      obj <- ctl[[i]] * 1.005
      jB <- which(s$rpd <= obj)
      if (length(jB)) { tB <- c(tB, s$t[jB[1]] / T); okB <- okB + 1 }
    }
    res[[a]] <- c(A = median(tA), B = if (length(tB)) median(tB) else NA,
                  ok = 100*okB/length(insts))
    cat(sprintf("%-8s %-4s %9.0f%% %9s %9.0f%%\n", algos[[al]], a,
                100*median(tA),
                if (length(tB)) sprintf("%.0f%%", 100*median(tB)) else "--",
                100*okB/length(insts)))
  }
  # aceleracion de cada brazo respecto al control en la metrica B
  cat(sprintf("  aceleracion vs control (metrica B, mediana): %s\n",
      paste(sapply(setdiff(arms,"A0"), function(a) {
        r <- res[[a]]["B"] / res[["A0"]]["B"]
        sprintf("%s %.2fx", a, r) }), collapse = "  ")))
  cat("\n")
}
cat("TTT-A: fraccion del presupuesto hasta quedar a <=1% del propio valor final\n")
cat("TTT-B: fraccion hasta alcanzar la calidad final del control (+0.5%)\n")
cat("alcanza B: %% de instancias en que el brazo llega a esa calidad\n")
