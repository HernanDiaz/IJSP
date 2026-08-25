# FIGURA PRINCIPAL del paper: el efecto de la siembra en funcion del TAMANO de la
# instancia, por algoritmo. Es el hallazgo que estructura el articulo: para ABCE3 y
# TSN2 la ganancia CRECE con el tamano, para el GA el perjuicio tambien crece.
# Eje y: mejora relativa de MIX frente al control A0, en % de E[Cmax] (negativo = mejor).
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst", "lb")]
algos <- c(ga = "GA", abce3 = "ABCE3", feabcls = "fEABCLS", tsn2 = "TSN2")
clases <- c("10x10","15x15","20x20","30x15","30x20","50x15","50x20")

eff <- function(al, arm) {
  d <- read.csv(sprintf("final/phase2/results_%s.csv", al), stringsAsFactors = FALSE)
  m <- aggregate(ecmax ~ inst + clase + arm, data = d, FUN = mean)
  a0 <- m[m$arm == "A0", c("inst","clase","ecmax")]; names(a0)[3] <- "base"
  s  <- merge(m[m$arm == arm, ], a0, by = c("inst","clase"))
  s$rel <- 100 * (s$ecmax - s$base) / s$base
  sapply(clases, function(cl) mean(s$rel[s$clase == cl]))
}

png("final/figs/main_effect_by_size.png", width = 1500, height = 900, res = 150)
par(mar = c(4.2, 4.6, 2.2, 8.4), xpd = FALSE)
cols <- c(GA = "#c0392b", ABCE3 = "#2980b9", fEABCLS = "#27ae60", TSN2 = "#8e44ad")
M <- sapply(names(algos), function(a) eff(a, "MIX"))
ylim <- range(M) + c(-0.15, 0.15)
plot(NA, xlim = c(1, length(clases)), ylim = ylim, xaxt = "n",
     xlab = "Instance class (jobs x machines)", ylab = "Relative change in E[Cmax] vs. control (%)",
     main = "Effect of MIX seeding as a function of instance size")
axis(1, at = seq_along(clases), labels = clases)
abline(h = 0, lty = 2, col = "grey40"); grid(nx = NA, ny = NULL, col = "grey88")
for (i in seq_along(algos)) {
  nm <- algos[[i]]
  lines(seq_along(clases), M[, i], col = cols[[nm]], lwd = 2.6, type = "b", pch = 15 + i)
}
par(xpd = TRUE)
legend(length(clases) + 0.25, mean(ylim), legend = unname(algos), col = cols[unname(algos)],
       lwd = 2.6, pch = 16:19, bty = "n", yjust = 0.5)
dev.off()

cat("=== mejora relativa (%) de MIX vs A0, por clase ===\n")
print(round(t(M), 2))
cat("\nfigura: final/figs/main_effect_by_size.png\n")
