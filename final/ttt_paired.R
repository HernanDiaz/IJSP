# TIME-TO-TARGET EMPAREJADO POR INSTANCIA.
#
# Sustituye al analisis log-rank anterior (ttt_survival.R), que era invalido
# para este diseno por DOS motivos, no uno:
#
#   1. Emparejamiento. survdiff() trata el brazo sembrado y el control como dos
#      muestras independientes. No lo son: comparten instancia, presupuesto y
#      objetivo, y se ejecutan sobre las mismas 61 instancias.
#
#   2. Asimetria estructural del grupo de referencia. El objetivo de cada
#      instancia se define como el endpoint del PROPIO control (+0.5%), luego
#      A0 lo alcanza en 61 de 61 instancias por construccion: su muestra NO
#      PUEDE censurarse. Los brazos sembrados si. Un log-rank contrasta
#      entonces una muestra censurada contra otra que es incensurable por
#      definicion, y su p-valor no significa lo que aparenta.
#
# Dato base: T_brazo(i) = fraccion del presupuesto en que el brazo alcanza el
# objetivo en la instancia i, restringida en 1.0 si no lo alcanza. La diferencia
# emparejada es
#
#     d_i = T_brazo(i) - T_A0(i),   i = 1..61     (negativo = el brazo llega antes)
#
# QUE CONTRASTE ES VALIDO AQUI. Un brazo que no alcanza el objetivo tiene un
# tiempo real MAYOR que el presupuesto, pero se registra como 1.0: la truncatura
# SUBESTIMA su tiempo. Como A0 alcanza siempre, solo se truncan los brazos
# sembrados, de modo que el sesgo va A FAVOR de la siembra. Por tanto:
#
#   - La MEDIA de d esta sesgada hacia mostrar aceleracion y NO es un estimador
#     honesto cuando hay muchos no alcanzadores. Se reporta como secundaria y
#     debe leerse como cota inferior del tiempo del brazo sembrado.
#   - El SIGNO de d si es correcto siempre: un no alcanzador registra
#     T = 1.0 >= T_A0, luego d >= 0 y se cuenta como "no mas rapido", que es lo
#     que corresponde. El TEST DE SIGNOS es por tanto valido bajo la truncatura
#     y se adopta como CONTRASTE PRINCIPAL.
#
# Inferencia, toda emparejada por instancia:
#   - PRINCIPAL: test de signos exacto sobre las 61 diferencias (binomial),
#     equivalente a contar en cuantas instancias el brazo llega antes que su
#     control. Holm sobre los 6 brazos de cada solver.
#   - SECUNDARIO: diferencia de tiempo medio restringido (RMTT) con IC 95% por
#     bootstrap percentil remuestreando INSTANCIAS (B=10000) y p por permutacion
#     de signos (B=20000).
#   - Cuando principal y secundario discrepan, el efecto no es un desplazamiento
#     general sino una mejora concentrada en un subconjunto de instancias. Se
#     reporta explicitamente.
#
# Las curvas de Kaplan-Meier se conservan SOLO como descripcion.
suppressPackageStartupMessages(library(survival))
lb <- read.csv("final/ta_lb.csv", stringsAsFactors = FALSE)[, c("inst","lb")]
algos <- c(ga="GA", abce3="ABCE3", feabcls="fEABCLS", tsn2="TSN2")
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
seeded <- setdiff(arms, "A0")
set.seed(20260825)
BPERM <- 20000
BBOOT <- 10000

# p de permutacion de signos sobre las diferencias emparejadas (bilateral)
perm_p <- function(x, B = BPERM) {
  n <- length(x); obs <- abs(mean(x))
  S <- matrix(sample(c(-1, 1), n * B, replace = TRUE), nrow = n)
  (1 + sum(abs(colMeans(S * x)) >= obs - 1e-12)) / (B + 1)
}
# IC percentil remuestreando instancias
boot_ci <- function(x, B = BBOOT) {
  n <- length(x)
  M <- matrix(sample(x, n * B, replace = TRUE), nrow = n)
  unname(quantile(colMeans(M), c(0.025, 0.975)))
}

cat("=== Tiempo hasta la calidad final del control (+0.5%), EMPAREJADO ===\n")
cat("    T = fraccion del presupuesto de la celda; no alcanzadores restringidos en 1.0\n")
cat("    dif = RMTT(brazo) - RMTT(A0); negativo = el brazo llega antes\n")
cat("    p = permutacion de signos (B=20000), Holm sobre los 6 brazos de cada solver\n\n")

todo <- data.frame()
for (al in names(algos)) {
  d <- read.csv(sprintf("final/phase2/anytime_%s.csv", al), stringsAsFactors = FALSE)
  d <- merge(d, lb, by = "inst"); d$rpd <- 100 * (d$bestcmax - d$lb) / d$lb
  insts <- Reduce(intersect, lapply(arms, function(a) unique(d$inst[d$arm == a])))

  # objetivo por instancia: endpoint del control +0.5%
  fin <- function(a, i) { s <- d[d$inst == i & d$arm == a, ]; s$rpd[which.max(s$t)] }
  ctl <- sapply(insts, function(i) fin("A0", i)); names(ctl) <- insts

  # T por (brazo, instancia), restringido en 1.0
  Tm <- matrix(NA_real_, nrow = length(insts), ncol = length(arms),
               dimnames = list(insts, arms))
  Rm <- Tm                                            # 1 si alcanza, 0 si no
  for (a in arms) for (i in insts) {
    s <- d[d$inst == i & d$arm == a, ]; s <- s[order(s$t), ]
    if (nrow(s) < 3) next
    Tmax <- max(s$t); j <- which(s$rpd <= ctl[[i]] * 1.005)
    if (length(j)) { Tm[i, a] <- s$t[j[1]] / Tmax; Rm[i, a] <- 1 }
    else           { Tm[i, a] <- 1.0;              Rm[i, a] <- 0 }
  }
  ok <- stats::complete.cases(Tm)
  Tm <- Tm[ok, , drop = FALSE]; Rm <- Rm[ok, , drop = FALSE]
  n <- nrow(Tm)

  cat(sprintf("%s  (n = %d instancias emparejadas)\n", algos[[al]], n))
  cat(sprintf("  A0: alcanza %.0f%% (por construccion), RMTT = %.3f\n",
              100 * mean(Rm[, "A0"]), mean(Tm[, "A0"])))
  cat(sprintf("  %-5s %7s %9s %10s %8s %17s %9s\n",
              "brazo", "alcanza", "antes/desp", "p signos", "RMTT", "dif RMTT IC95%", "p perm"))

  psig <- numeric(0); pperm <- numeric(0); filas <- list()
  for (a in seeded) {
    dif <- Tm[, a] - Tm[, "A0"]
    antes <- sum(dif < 0); desp <- sum(dif > 0); emp <- sum(dif == 0)
    # test de signos exacto: valido bajo la truncatura (el signo nunca se invierte)
    psig[a] <- if (antes + desp > 0) binom.test(antes, antes + desp)$p.value else 1
    pperm[a] <- perm_p(dif); ci <- boot_ci(dif)
    filas[[a]] <- data.frame(algo = al, arm = a, n = n,
                             reach = mean(Rm[, a]), antes = antes, desp = desp, emp = emp,
                             med_dif = median(dif), rmtt = mean(Tm[, a]),
                             rmtt_a0 = mean(Tm[, "A0"]), dif = mean(dif),
                             lo = ci[1], hi = ci[2],
                             p_signos = psig[a], p_perm = pperm[a])
  }
  hsig <- p.adjust(psig, method = "holm"); hperm <- p.adjust(pperm, method = "holm")
  for (a in seeded) {
    f <- filas[[a]]; f$p_signos_holm <- hsig[a]; f$p_perm_holm <- hperm[a]
    st <- function(p) if (p < 0.001) "***" else if (p < 0.01) "**" else if (p < 0.05) "*" else ""
    cat(sprintf("  %-5s %6.0f%% %5d/%-4d %7.4f%-3s %8.3f  [%+6.3f,%+6.3f] %7.4f%-3s\n",
                a, 100 * f$reach, f$antes, f$desp, hsig[a], st(hsig[a]),
                f$rmtt, f$lo, f$hi, hperm[a], st(hperm[a])))
    todo <- rbind(todo, f)
  }

  # Kaplan-Meier, SOLO descriptivo (sin contraste)
  ev <- do.call(rbind, lapply(arms, function(a)
    data.frame(arm = a, t = Tm[, a], ok = Rm[, a])))
  ev$arm <- factor(ev$arm, levels = arms)
  km <- summary(survfit(Surv(t, ok) ~ arm, data = ev))$table
  cat("  mediana KM (descriptiva):")
  for (a in arms) {
    m <- km[paste0("arm=", a), "median"]
    cat(sprintf("  %s=%s", a, ifelse(is.na(m), "n.a.", sprintf("%.2f", m))))
  }
  cat("\n\n")
}
write.csv(todo, "final/ttt_paired.csv", row.names = FALSE)

cat("=== Principal (signos) frente a secundario (RMTT): donde discrepan ===\n")
cat("    discrepancia = la media dice aceleracion y el signo no la respalda:\n")
cat("    la mejora esta concentrada en un subconjunto, no es un desplazamiento general\n\n")
for (al in names(algos)) {
  s <- todo[todo$algo == al, ]
  a <- s$p_signos_holm < 0.05; b <- s$p_perm_holm < 0.05
  disc <- s$arm[b & !a]
  cat(sprintf("  %-8s signos %d/6 | RMTT %d/6   %s\n", algos[[al]], sum(a), sum(b),
              ifelse(length(disc), paste("concentrado en:", paste(disc, collapse=" ")),
                     "(coinciden)")))
}
cat("\nescrito: final/ttt_paired.csv\n")
