#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_samples
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

PAIRS=(
  "250 250"
  "100 400"
  "150 350"
  "400 100"
  "350 150"
  "200 300"
  "300 200"
)

for pair in "${PAIRS[@]}"; do
  read ss se <<< "$pair"
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/s${ss}_e${se}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v ss="$ss" -v se="$se" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "samples_start=%-4s samples_end=%-4s meanAvgRE%%=%6.2f\n", ss, se, s/n }'
done | sort -t= -k3 -n
