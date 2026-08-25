# FIGURA DE CONVERGENCIA para el paper: RPD medio frente al tiempo, un panel por
# algoritmo, en la clase 50x20 (donde se concentra el efecto de la siembra).
#
# Dos cuidados metodologicos, ambos aprendidos a base de equivocarse antes:
#  1) SUPERVIVENCIA: si una instancia deja de aportar puntos al final de la
#     rejilla, la media SUBE artificialmente. Cada curva se extiende con su
#     ultimo valor hasta el final de la rejilla comun.
#  2) Solo se usan instancias presentes en TODOS los brazos comparados, para que
#     la media no cambie de composicion entre curvas.
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga = "GA", abce3 = "ABCE3", feabcls = "fEABCLS", tsn2 = "TSN2")
arms <- c(A0="Unseeded (A0)", V2H="Learned, half (V2H)", V2="Learned (V2)", MOR="MOR-eps", GT="GT-eps", GP="GP-eps", MIX="Mixed pool (MIX)")
cols <- c(A0="#000000", V2H="#0072B2", V2="#56B4E9", MOR="#E69F00", GT="#D55E00", GP="#009E73", MIX="#CC0000")
lwds <- c(A0=3.2, V2H=1.8, V2=1.8, MOR=1.8, GT=1.8, GP=1.8, MIX=3.2)
ltys <- c(A0=1, V2H=2, V2=1, MOR=4, GT=5, GP=3, MIX=1)
CLASS <- "50x20"
insts <- sprintf("tai50_20_%02d", 1:10)

png("final/figs/anytime_50x20.png", width = 1700, height = 1250, res = 150)
par(mfrow = c(2, 2), mar = c(4.0, 4.3, 2.6, 1.0))
for (al in names(algos)) {
  f <- sprintf("final/phase2/anytime_%s.csv", al)
  if (!file.exists(f)) { plot.new(); title(paste(algos[[al]], "- sin datos")); next }
  d <- read.csv(f, stringsAsFactors = FALSE)
  d <- d[d$inst %in% insts & d$arm %in% names(arms), ]
  if (!nrow(d)) { plot.new(); title(paste(algos[[al]], "- sin datos")); next }
  d <- merge(d, lb, by = "inst")
  d$rpd <- 100 * (d$bestcmax - d$lb) / d$lb
  ok <- Reduce(intersect, lapply(names(arms), function(a) unique(d$inst[d$arm == a])))
  d <- d[d$inst %in% ok, ]
  grid_t <- sort(unique(d$t))
  curva <- function(a) {                       # media entre instancias, con extension
    sapply(grid_t, function(tt) {
      mean(sapply(ok, function(i) {
        s <- d[d$inst == i & d$arm == a & d$t <= tt, ]
        if (!nrow(s)) NA else s$rpd[which.max(s$t)]   # mejor conocido hasta tt
      }), na.rm = TRUE)
    })
  }
  M <- sapply(names(arms), curva)
  keep <- grid_t > 0 & is.finite(rowSums(M))
  plot(NA, xlim = range(grid_t[keep]), ylim = range(M[keep, ]),
       xlab = "CPU time (s)", ylab = "Mean RPD vs. lower bound (%)",
       main = sprintf("%s  (%s, n=%d)", algos[[al]], CLASS, length(ok)))
  grid(col = "grey90")
  for (a in names(arms)) lines(grid_t[keep], M[keep, a], col = cols[[a]], lty = ltys[[a]],
                               lwd = lwds[[a]])
  legend("topright", legend = unname(arms), col = cols[names(arms)], lty = ltys[names(arms)],
         lwd = lwds[names(arms)], bty = "n", cex = 0.72)
}
dev.off()
cat("figura: final/figs/anytime_50x20.png\n")
