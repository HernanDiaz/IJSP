#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_dropout
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

FRACS=(1.0 0.8 0.6 0.4 0.2)
for rf in "${FRACS[@]}"; do
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/rf${rf}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v rf="$rf" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "rot_frac=%-4s meanAvgRE%%=%6.2f\n", rf, s/n }'
done | sort -t= -k2 -n
