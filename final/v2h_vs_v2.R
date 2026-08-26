lb <- read.csv("final/ta_lb.csv")[,c("inst","lb")]
set.seed(1)
cat(sprintf("%-8s %8s %8s %9s %18s %9s\n","solver","V2H","V2","dif","IC 95%","p"))
for (al in c("ga","abce3","feabcls","tsn2")) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv", al)), lb, by="inst")
  d$rpd <- 100*(d$ecmax-d$lb)/d$lb
  m <- aggregate(rpd ~ inst + arm, d, mean)
  w <- reshape(m, idvar="inst", timevar="arm", direction="wide")
  x <- w$rpd.V2H - w$rpd.V2                      # negativo = V2H mejor
  B <- matrix(sample(x, length(x)*10000, TRUE), nrow=length(x))
  ci <- quantile(colMeans(B), c(.025,.975))
  p <- suppressWarnings(wilcox.test(x)$p.value)
  cat(sprintf("%-8s %8.2f %8.2f %+9.3f  [%+6.3f,%+6.3f] %9.4f%s\n", al,
      mean(w$rpd.V2H), mean(w$rpd.V2), mean(x), ci[1], ci[2], p,
      ifelse(p<0.05," *","")))
}
