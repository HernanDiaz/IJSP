#!/usr/bin/env bash
# Extrae E[Cmax] por run de la Fase 2 para un solver -> final/phase2/results_<algo>.csv
cd "$(dirname "$0")/.."
algo=${1:?uso: extract_phase2.sh <algo>}
out="final/phase2/results_${algo}.csv"
clase() { case "$1" in
  ft10) echo 10x10 ;;
  tai15_15_*) echo 15x15 ;; tai20_20_*) echo 20x20 ;; tai30_15_*) echo 30x15 ;;
  tai30_20_*) echo 30x20 ;; tai50_15_*) echo 50x15 ;; tai50_20_*) echo 50x20 ;;
esac; }
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
echo "algo,inst,clase,arm,run,ecmax" > "$out"
for inst in $INSTS; do
  cls=$(clase "$inst")
  for arm in A0 V2H V2 MOR GT GP MIX; do
    s=$(ls "final/phase2/$algo/$inst/$arm"/*_Sols.csv 2>/dev/null | head -1)
    [ -z "$s" ] && continue
    awk -F';' -v a=$algo -v i=$inst -v c=$cls -v ar=$arm \
      'NR>1&&$3~/\(/{gsub(/[()]/,"",$3);split($3,ab,",");printf "%s,%s,%s,%s,%d,%.1f\n",a,i,c,ar,$1,(ab[1]+ab[2])/2}' "$s" >> "$out"
  done
done
echo "$out : $(( $(wc -l < "$out") - 1 )) filas"