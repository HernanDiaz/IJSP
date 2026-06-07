#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_genstep
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

FACTORS=(1.0 0.3 0.5 0.7 0.85 1.3 1.5)
for gsf in "${FACTORS[@]}"; do
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/gsf${gsf}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v gsf="$gsf" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "gen_step_factor=%-5s meanAvgRE%%=%6.2f\n", gsf, s/n }'
done | sort -t= -k2 -n
