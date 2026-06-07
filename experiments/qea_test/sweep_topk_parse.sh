#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_topk
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

PAIRS=(
  "0 0"
  "20 5"
  "20 7"
  "20 10"
  "15 5"
  "15 7"
  "5 20"
)

for pair in "${PAIRS[@]}"; do
  read ks ke <<< "$pair"
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/ks${ks}_ke${ke}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v ks="$ks" -v ke="$ke" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "topk_start=%-3s topk_end=%-3s meanAvgRE%%=%6.2f\n", ks, ke, s/n }'
done | sort -t= -k3 -n
