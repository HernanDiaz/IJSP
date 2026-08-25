#!/usr/bin/env bash
# ¿QUE HAY QUE REHACER REALMENTE?
# Tras anadir creation.seed.offset, la unica diferencia con consecuencia
# cientifica entre regimenes desaparece. El criterio pasa a ser: ¿esta celda
# muestreo 30 bloques distintos del pool, o solo 5?
#   - celda MONOLITICA (30 runs, 1 proceso)  -> runCounter 0..29 -> 30 bloques OK
#   - celda POR TROZOS sin offset            -> cada proceso reinicia -> 5 bloques KO
# El brazo A0 no usa pool: nunca necesita rehacerse por este motivo.
cd "$(dirname "$0")/.."
declare -A B=(
 [ga:15x15]=60 [ga:20x20]=60 [ga:30x15]=109 [ga:30x20]=154 [ga:50x15]=233 [ga:50x20]=321
 [abce3:15x15]=60 [abce3:20x20]=60 [abce3:30x15]=60 [abce3:30x20]=60 [abce3:50x15]=60 [abce3:50x20]=60
 [feabcls:15x15]=60 [feabcls:20x20]=67 [feabcls:30x15]=371 [feabcls:30x20]=900 [feabcls:50x15]=900 [feabcls:50x20]=900
 [tsn2:15x15]=60 [tsn2:20x20]=486 [tsn2:30x15]=630 [tsn2:30x20]=900 [tsn2:50x15]=900 [tsn2:50x20]=900 )
clase() { case "$1" in ft10|tai15_15_*) echo 15x15;; tai20_20_*) echo 20x20;; tai30_15_*) echo 30x15;;
  tai30_20_*) echo 30x20;; tai50_15_*) echo 50x15;; tai50_20_*) echo 50x20;; esac; }

printf "%-8s %10s %10s %14s\n" algo "celdas OK" "a rehacer" "coste (h CPU)"
gtot=0
for a in ga abce3 feabcls tsn2; do
  ok=0; ko=0; cost=0
  for d in final/phase2/$a/*/*/; do
    inst=$(basename "$(dirname "$d")"); arm=$(basename "$d")
    chunked=0; [ -n "$(ls -d "$d"c* 2>/dev/null)" ] && chunked=1
    if [ "$arm" = A0 ] || [ "$chunked" -eq 0 ]; then ok=$((ok+1))
    else
      ko=$((ko+1)); cl=$(clase "$inst"); cost=$((cost + 30*${B[$a:$cl]}))
    fi
  done
  h=$((cost/3600)); gtot=$((gtot+h))
  printf "%-8s %10d %10d %14d\n" "$a" "$ok" "$ko" "$h"
done
echo ""
echo "TOTAL a rehacer: $gtot h de CPU  ->  $((gtot/14)) h de reloj con 14 procesos  (~$((gtot/14/24)) dias)"
echo ""
echo "(A0 no usa pool: sus celdas no necesitan rehacerse por el reparto de semillas,"
echo " pero SI si se quiere que todo el solver comparta un unico regimen de proceso.)"
