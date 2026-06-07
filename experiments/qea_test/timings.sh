#!/bin/bash
# Extracts the mean per-run runtime from each result CSV and estimates the
# per-instance time (x30 runs, sequential inside one process).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT="${QEA_OUT:-experiments/qea_test/run30_lam}"
printf "%-8s %10s %14s\n" "Inst" "s/run" "s/inst (x30)"
for inst in abz7_06 abz8_05 abz9_10 ft10_10 ft20_05 la21_04 la24_03 la25_04 la27_09 la29_03 la38_06 la40_05; do
  csv=$(ls "$OUT/$inst"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  t=$(grep -m1 "^Total runtime;" "$csv" | cut -d';' -f2)
  awk -v i="$inst" -v t="$t" 'BEGIN{ printf "%-8s %10.2f %14.1f\n", i, t, t*30 }'
done
