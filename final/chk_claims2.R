lb <- read.csv("final/ta_lb.csv")[,c("inst","lb")]
cat("=== 'MIX mejora al mas debil en 35 de 61 sin perder ninguna' ===\n")
d <- read.csv("final/phase2/results_abce3.csv")
W<-0;L<-0;T<-0
for (i in unique(d$inst)) {
  a0 <- d[d$inst==i & d$arm=="A0",]; a0<-a0[order(a0$run),"ecmax"]
  ps <- sapply(c("V2H","V2","MOR","GT","GP","MIX"), function(a){
    x<-d[d$inst==i & d$arm==a,]; x<-x[order(x$run),"ecmax"]
    tryCatch(wilcox.test(x,a0,paired=TRUE,exact=FALSE)$p.value,error=function(e)NA)})
  ph <- p.adjust(ps,method="holm")["MIX"]
  x<-d[d$inst==i & d$arm=="MIX",]; x<-x[order(x$run),"ecmax"]
  if(!is.na(ph)&&ph<0.05){ if(mean(x)<mean(a0)) W<-W+1 else L<-L+1 } else T<-T+1
}
cat(sprintf("  ABCE3 MIX: %d victorias / %d empates / %d derrotas  -> %s\n",W,T,L,
    ifelse(W==35&&L==0,"OK","*** REVISAR ***")))

cat("\n=== 'fEABCLS estadisticamente sin cambio' (MIX vs A0) ===\n")
d <- read.csv("final/phase2/results_feabcls.csv")
d2 <- merge(d, lb, by="inst"); d2$rpd <- 100*(d2$ecmax-d2$lb)/d2$lb
m <- tapply(d2$rpd, d2$arm, mean)
cat(sprintf("  A0=%.2f MIX=%.2f  dif=%+.3f puntos\n", m["A0"], m["MIX"], m["MIX"]-m["A0"]))

cat("\n=== 'en 50x20 TODOS los brazos sembrados baten al control (TSN2)' ===\n")
d <- merge(read.csv("final/phase2/results_tsn2.csv"), lb, by="inst")
s <- d[d$clase=="50x20",]
a0 <- mean(s$ecmax[s$arm=="A0"])
for (a in c("V2H","V2","MOR","GT","GP","MIX")) {
  v <- mean(s$ecmax[s$arm==a])
  cat(sprintf("  %-4s %8.1f  vs A0 %8.1f  %s\n", a, v, a0, ifelse(v<a0,"bate","*** NO BATE ***")))
}
