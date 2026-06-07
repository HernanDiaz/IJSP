#!/bin/bash
# Summarises the sweep: mean AvgRE% over the 3 instances per (rotation, floor).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep
get_lb() { case "$1" in ft10_10) echo 930;; ft20_05) echo 1165;; la25_04) echo 977;; esac; }

for rot in 0.02 0.04 0.08 0.16; do
  for flr in 0.0 0.01 0.05; do
    sum=0; n=0
    for inst in ft10_10 ft20_05 la25_04; do
      d="$OUT/r${rot}_f${flr}_${inst}"
      csv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
      avg=$(grep -m1 "^Best solution;" "$csv" | cut -d';' -f2)
      lb=$(get_lb "$inst")
      re=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
      sum=$(awk -v s="$sum" -v r="$re" 'BEGIN{print s+r}')
      n=$((n+1))
    done
    awk -v r="$rot" -v f="$flr" -v s="$sum" -v n="$n" \
      'BEGIN{ printf "rotation=%-5s floor=%-5s  meanAvgRE%%=%6.2f\n", r, f, s/n }'
  done
done | sort -t= -k4 -n
