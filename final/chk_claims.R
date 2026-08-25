lb <- read.csv("final/ta_lb.csv")[,c("inst","lb")]
arms <- c("A0","V2H","V2","MOR","GT","GP","MIX")
cat("=== 1) 'MIX es el mejor brazo SEMBRADO en los cuatro' ===\n")
for (al in c("ga","abce3","feabcls","tsn2")) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv",al)), lb, by="inst")
  d$rpd <- 100*(d$ecmax-d$lb)/d$lb
  m <- tapply(d$rpd, d$arm, mean)
  seeded <- m[setdiff(arms,"A0")]
  cat(sprintf("  %-8s mejor sembrado = %-4s (%.2f)  %s\n", al, names(which.min(seeded)),
      min(seeded), ifelse(names(which.min(seeded))=="MIX","OK","*** NO ES MIX ***")))
}
cat("\n=== 2) 'V2H y V2 siguen, GP intermedio' (orden por RPD) ===\n")
for (al in c("ga","abce3","feabcls","tsn2")) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv",al)), lb, by="inst")
  d$rpd <- 100*(d$ecmax-d$lb)/d$lb
  m <- sort(tapply(d$rpd, d$arm, mean)[setdiff(arms,"A0")])
  cat(sprintf("  %-8s %s\n", al, paste(names(m), collapse=" < ")))
}
cat("\n=== 3) 'MOR y GT son los peores en los cuatro' ===\n")
for (al in c("ga","abce3","feabcls","tsn2")) {
  d <- merge(read.csv(sprintf("final/phase2/results_%s.csv",al)), lb, by="inst")
  d$rpd <- 100*(d$ecmax-d$lb)/d$lb
  m <- sort(tapply(d$rpd, d$arm, mean)[setdiff(arms,"A0")], decreasing=TRUE)
  cat(sprintf("  %-8s peores: %s, %s\n", al, names(m)[1], names(m)[2]))
}
