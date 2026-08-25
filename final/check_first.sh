#!/usr/bin/env bash
# ¿La busqueda local BORRA la ventaja de la siembra en la primera generacion?
# Compara el PRIMER punto de traza (ya con LS aplicada) entre control y sembrado,
# en fEABCLS (LS fuerte) frente a ABCE3 (sin LS), en las mismas instancias.
cd "$(dirname "$0")/.."
first_of() { # $1=algo $2=inst $3=arm -> primer bestcmax de la traza
  local base="final/phase2/$1/$2/$3"
  local c=$(ls "$base"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  [ -z "$c" ] && c=$(ls "$base"/c1/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  [ -z "$c" ] && { echo ""; return; }
  awk -F';' '/^Evolution/{f=1} f && $1 ~ /^[0-9]+$/ { printf "%.1f", $3; exit }' "$c"
}
printf "%-9s %-14s %10s %10s %10s\n" "algo" "instancia" "A0_ini" "V2_ini" "ventaja%"
for algo in abce3 feabcls; do
  for inst in tai30_20_01 tai30_20_05 tai50_15_01 tai50_15_05; do
    a=$(first_of $algo $inst A0); v=$(first_of $algo $inst V2)
    [ -z "$a" -o -z "$v" ] && continue
    awk -v al=$algo -v i=$inst -v a=$a -v v=$v 'BEGIN{printf "%-9s %-14s %10.1f %10.1f %9.1f%%\n", al, i, a, v, 100*(a-v)/a}'
  done
done
echo ""
echo "(ventaja% = cuanto mejor arranca el sembrado en el PRIMER punto registrado;"
echo " si la LS fuerte borra la ventaja, en feabcls sera ~0 y en abce3 grande)"
