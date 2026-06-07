#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_sched
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

PAIRS=(
  "0.04 0.04"
  "0.01 0.10"
  "0.02 0.08"
  "0.10 0.01"
  "0.08 0.02"
  "0.01 0.04"
  "0.04 0.01"
)

for pair in "${PAIRS[@]}"; do
  read rs re <<< "$pair"
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/s${rs}_e${re}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v rs="$rs" -v re="$re" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "start=%-5s end=%-5s meanAvgRE%%=%6.2f\n", rs, re, s/n }'
done | sort -t= -k4 -n
