load("irace.log")
m <- iraceResults$experiments
elites <- iraceResults$iterationElites
b <- as.character(elites[length(elites)])
v <- m[, b]
cat("best config id:", b, "\n")
cat("mean -HV (best config):", round(mean(v, na.rm = TRUE)), "\n")
cat("min  -HV (best config):", round(min(v, na.rm = TRUE)), "\n")
cat("evaluations of best config:", sum(!is.na(v)), "\n")
cat("total irace evaluations:", sum(!is.na(m)), "\n")
cat("configs generated:", ncol(m), "\n")
# best config parameters
bc <- iraceResults$allConfigurations
bc <- bc[bc$.ID. == as.integer(b), ]
print(bc)
