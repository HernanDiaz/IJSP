# TIME-TO-TARGET como ANALISIS DE SUPERVIVENCIA (objecion del revisor 2).
#
# La mediana calculada solo entre los exitos es un estimador sesgado: descarta
# precisamente las instancias donde el brazo tarda mas, o no llega. Aqui las
# instancias que no alcanzan el objetivo dentro del presupuesto se tratan como
# CENSURADAS por la derecha en 1.0 (fraccion de presupuesto), que es lo que son.
# Se estima la curva de Kaplan-Meier y se contrasta con el test de log-rank.
suppressPackageStartupMessages(library(survival))
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")

cat("=== Tiempo hasta la calidad final del control (+0.5%), con censura ===\n")
cat("    tiempo = fraccion del presupuesto; censurado en 1.0 si no se alcanza\n")
cat("    mediana KM = NA cuando mas de la mitad de las instancias no llegan\n\n")
todo <- data.frame()
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors = FALSE)
  d <- merge(d, lb, by = "inst"); d$rpd <- 100*(d$bestcmax - d$lb)/d$lb
  insts <- Reduce(intersect, lapply(arms, function(a) unique(d$inst[d$arm == a])))
  fin <- function(a, i) { s <- d[d$inst==i & d$arm==a, ]; s$rpd[which.max(s$t)] }
  ctl <- sapply(insts, function(i) fin("A0", i)); names(ctl) <- insts
  ev <- data.frame()
  for (a in arms) for (i in insts) {
    s <- d[d$inst==i & d$arm==a, ]; s <- s[order(s$t), ]
    if (nrow(s) < 3) next
    T <- max(s$t); obj <- ctl[[i]] * 1.005
    j <- which(s$rpd <= obj)
    if (length(j)) ev <- rbind(ev, data.frame(arm=a, inst=i, t=s$t[j[1]]/T, ok=1))
    else           ev <- rbind(ev, data.frame(arm=a, inst=i, t=1.0,          ok=0))
  }
  ev$arm <- factor(ev$arm, levels = arms)
  km <- survfit(Surv(t, ok) ~ arm, data = ev)
  s <- summary(km)$table
  cat(sprintf("%s\n", algos[[al]]))
  cat(sprintf("  %-5s %8s %10s %14s\n", "brazo", "alcanza", "mediana KM", "IC 95%"))
  for (a in arms) {
    fila <- s[paste0("arm=", a), ]
    e <- ev[ev$arm == a, ]
    med <- fila[["median"]]
    lo <- fila[["0.95LCL"]]; hi <- fila[["0.95UCL"]]
    cat(sprintf("  %-5s %7.0f%% %10s %14s\n", a, 100*mean(e$ok),
        ifelse(is.na(med), "no alcanzada", sprintf("%.2f", med)),
        ifelse(is.na(lo)|is.na(hi), "--", sprintf("[%.2f, %.2f]", lo, hi))))
  }
  # log-rank de cada brazo sembrado frente al control
  cat("  log-rank vs A0 (Holm sobre los 6 brazos):")
  ps <- sapply(setdiff(arms,"A0"), function(a) {
    sub <- ev[ev$arm %in% c("A0", a), ]; sub$arm <- factor(sub$arm)
    1 - pchisq(survdiff(Surv(t, ok) ~ arm, data = sub)$chisq, 1) })
  ph <- p.adjust(ps, method = "holm")
  cat("\n")
  for (a in names(ph))
    cat(sprintf("     %-5s p=%.4g %s\n", a, ph[a],
        ifelse(ph[a] < 0.05, "*", "")))
  todo <- rbind(todo, data.frame(algo=al, arm=names(ph), p_holm=as.numeric(ph)))
  cat("\n")
}
write.csv(todo, "final/ttt_logrank.csv", row.names = FALSE)
cat("escrito: final/ttt_logrank.csv\n")
