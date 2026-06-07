#!/bin/bash
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_floorsched
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

PAIRS=(
  "0.01 0.01"
  "0.05 0.005"
  "0.05 0.01"
  "0.02 0.005"
  "0.02 0.01"
  "0.005 0.02"
  "0.005 0.05"
)

for pair in "${PAIRS[@]}"; do
  read fs fe <<< "$pair"
  sum=0; n=0
  for inst in ft10_10 ft20_05 la25_04; do
    d="$OUT/s${fs}_e${fe}_${inst}"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
    lb=$(get_lb "$inst")
    re_val=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sum=$(awk -v s="$sum" -v r="$re_val" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v fs="$fs" -v fe="$fe" -v s="$sum" -v n="$n" \
    'BEGIN{ printf "fl_start=%-6s fl_end=%-6s meanAvgRE%%=%6.2f\n", fs, fe, s/n }'
done | sort -t= -k4 -n
