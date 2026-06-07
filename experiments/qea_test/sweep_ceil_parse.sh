#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_ceil
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

PAIRS=(
  "1.0 1.0"
  "1.0 0.95"
  "1.0 0.90"
  "1.0 0.85"
  "0.99 0.85"
  "0.95 0.85"
  "0.95 0.95"
)

for pair in "${PAIRS[@]}"; do
  read cs ce <<< "$pair"
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/cs${cs}_ce${ce}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v cs="$cs" -v ce="$ce" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "ceil_start=%-5s ceil_end=%-5s meanAvgRE%%=%6.2f\n", cs, ce, s/n }'
done | sort -t= -k3 -n
