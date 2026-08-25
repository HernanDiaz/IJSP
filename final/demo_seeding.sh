#!/usr/bin/env bash
# DEMOSTRACION de que las semillas se usan como se cree:
# para cada ejecucion r, el operador toma el bloque [250r mod L, +250) del pool.
# Si es cierto, el MEJOR makespan del bloque r debe coincidir con el "Best Cmax"
# del paso 0 de la ejecucion r en la traza. Se comprueba con el propio makespan
# que el pool trae almacenado.
cd "$(dirname "$0")/.."
INST=tai20_20_01
POOL=pools_test/corrected/int__${INST}_mix_repo_pool.csv
T=/tmp/demo_seed; rm -rf $T; mkdir -p $T
sed -e "s/^runs = .*/runs = 8/" -e "s/^seed = .*/seed = 1/" repro/setup_ga.txt > $T/setup.txt
sed -i "s/^noimprovement = .*/generations = 1/" $T/setup.txt
grep -q '^timelimit' $T/setup.txt && sed -i "s/^timelimit = .*/timelimit = 1/" $T/setup.txt || echo "timelimit = 1" >> $T/setup.txt
sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" $T/setup.txt
printf "\ncreation.seed.pool = %s\ncreation.seed.count = 250\ncreation.seed.offset = 0\n" "$POOL" >> $T/setup.txt
./FuzzyFW $T/setup.txt SelectosYTaillardIntervalos/$INST.F.15_01.txt $T > $T/log 2>&1

tr=$(ls $T/*.csv | grep -v Sols | head -1)
L=$(wc -l < "$POOL")
echo "pool: $(basename $POOL)  |  L=$L soluciones  |  k=250 sembradas de 250 de poblacion"
echo ""
printf "%-5s %-14s %14s %14s   %s\n" "run" "bloque usado" "mejor del bloque" "traza paso 0" "coincide"
for r in 0 1 2 3 4 5 6 7; do
  start=$(( (r*250) % L ))
  # mejor E[Cmax] entre las 250 lineas del bloque, segun el makespan del pool
  best=$(sed -n "$((start+1)),$((start+250))p" "$POOL" | \
         sed 's/.*;\[//; s/\]//' | tr -d ' ' | \
         awk -F, '{m=($1+$2)/2; if (b=="" || m<b) b=m} END{printf "%.1f", b}')
  # Best Cmax del paso 0 de la ejecucion r
  obs=$(awk -F';' -v r="$r" '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {c=7+r*6+4; print $c; exit}' "$tr")
  same=$(awk -v a="$best" -v b="$obs" 'BEGIN{print (a==b) ? "SI" : "no"}')
  printf "%-5s %-14s %14s %14s   %s\n" "$r" "[$start, $((start+250)))" "$best" "$obs" "$same"
done
echo ""
echo "Si coinciden, queda probado que la ejecucion r siembra exactamente el bloque"
echo "[250r mod L, +250) del pool, y que las semillas entran decodificadas igual"
echo "que cualquier individuo (mismo SGS)."
