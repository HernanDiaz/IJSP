#!/usr/bin/env bash
# Extrae E[Cmax] por run de la Fase 2 soportando AMBOS formatos:
#   - monolitico:  <arm>/*_Sols.csv         (runs 1..30 en un fichero)
#   - chunked:     <arm>/c<j>/*_Sols.csv    (se concatenan y renumeran a 1..30)
# Uso: extract_phase2c.sh <algo>   -> final/phase2/results_<algo>.csv
cd "$(dirname "$0")/.."
algo=${1:?uso: extract_phase2c.sh <algo>}
BASEDIR=${OUT_OVERRIDE:-final/phase2}
out="$BASEDIR/results_${algo}.csv"
clase() { case "$1" in
  ft10) echo 10x10 ;; tai15_15_*) echo 15x15 ;; tai20_20_*) echo 20x20 ;; tai30_15_*) echo 30x15 ;;
  tai30_20_*) echo 30x20 ;; tai50_15_*) echo 50x15 ;; tai50_20_*) echo 50x20 ;;
esac; }
if [ -n "$INSTS_OVERRIDE" ]; then INSTS="$INSTS_OVERRIDE"; else
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
fi
echo "algo,inst,clase,arm,run,ecmax" > "$out"
for inst in $INSTS; do
  cls=$(clase "$inst")
  for arm in A0 V2H V2 MOR GT GP MIX MIXH; do
    base="$BASEDIR/$algo/$inst/$arm"
    # PRIORIDAD A LOS CHUNKS. Las corridas monoliticas abortadas del 26-jul
    # dejaron <arm>/*_Sols.csv con runs incompletos (p.ej. 21 en vez de 30);
    # si existen chunks, ellos son la corrida buena y completa.
    mono=$(ls "$base"/*_Sols.csv 2>/dev/null | head -1)
    [ -n "$(ls -d "$base"/c* 2>/dev/null)" ] && mono=""
    if [ -n "$mono" ]; then
      awk -F';' -v a=$algo -v i=$inst -v c=$cls -v ar=$arm \
        'NR>1&&$3~/\(/{gsub(/[()]/,"",$3);split($3,ab,",");printf "%s,%s,%s,%s,%d,%.1f\n",a,i,c,ar,$1,(ab[1]+ab[2])/2}' "$mono" >> "$out"
    else
      # chunked: concatena c1,c2,... renumerando runs de forma global
      r=0
      for d in $(ls -d "$base"/c* 2>/dev/null | sort -V); do
        s=$(ls "$d"/*_Sols.csv 2>/dev/null | head -1); [ -z "$s" ] && continue
        while IFS= read -r line; do
          r=$((r+1))
          echo "$line" | awk -F';' -v a=$algo -v i=$inst -v c=$cls -v ar=$arm -v rr=$r \
            '$3~/\(/{gsub(/[()]/,"",$3);split($3,ab,",");printf "%s,%s,%s,%s,%d,%.1f\n",a,i,c,ar,rr,(ab[1]+ab[2])/2}'
        done < <(tail -n +2 "$s")
      done >> "$out"
    fi
  done
done
echo "$out : $(( $(wc -l < "$out") - 1 )) filas"
