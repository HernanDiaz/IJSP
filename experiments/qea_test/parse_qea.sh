#!/bin/bash
# Parses QEA result CSVs and computes RE% per instance.
#   QEA_OUT : output root to parse (default run30)
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUTROOT="${QEA_OUT:-experiments/qea_test/run30}"
for entry in abz7_06:ABZ7:656 abz8_05:ABZ8:645 abz9_10:ABZ9:661 ft10_10:FT10:930 ft20_05:FT20:1165 la21_04:la21:1046 la24_03:la24:935 la25_04:la25:977 la27_09:la27:1235 la29_03:la29:1152 la38_06:la38:1196 la40_05:la40:1222; do
  inst=${entry%%:*}
  name=$(echo "$entry" | cut -d: -f2)
  lb=$(echo "$entry" | cut -d: -f3)
  csv=$(ls "$OUTROOT/$inst"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  line=$(grep -m1 "^Best solution;" "$csv")
  best=$(echo "$line" | cut -d';' -f3)
  avg=$(echo "$line" | cut -d';' -f2)
  echo "$name $lb $best $avg"
done | awk '
BEGIN{ printf "%-6s %5s %9s %9s %8s %8s\n","Inst","LB","BestMid","AvgMid","BestRE%","AvgRE%" }
{ lb=$2; best=$3; avg=$4; bre=(best-lb)/lb*100; are=(avg-lb)/lb*100; sb+=bre; sa+=are; nn++;
  printf "%-6s %5d %9.1f %9.1f %7.2f %7.2f\n",$1,lb,best,avg,bre,are }
END{ printf "%-6s %5s %9s %9s %7.2f %7.2f\n","MEAN","","","",sb/nn,sa/nn }
'
