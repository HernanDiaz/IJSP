#!/usr/bin/env bash
# ¿Se repiten de verdad las poblaciones iniciales entre trozos?
# Prediccion de la lectura del codigo: runCounter se reinicia por proceso, luego
# la ejecucion j de CADA trozo arranca del MISMO bloque del pool -> mismo
# Best Cmax y mismo Avg Cmax en el paso 0. Si es asi, las 30 ejecuciones solo
# muestrean 5 poblaciones iniciales distintas, repetidas 6 veces.
#
# En la traza, el bloque por-ejecucion empieza en la columna 8 y cada ejecucion
# ocupa 7 columnas: Generation;Runtime;Best;Avg;Best Cmax;Avg Cmax;Avg Hamming
cd "$(dirname "$0")/.."
algo=${1:-tsn2}; inst=${2:-tai30_20_01}; arm=${3:-MIX}
echo "=== $algo / $inst / $arm : Best Cmax inicial (paso 0) de cada ejecucion ==="
printf "%-6s" "trozo"; for r in 1 2 3 4 5; do printf "%12s" "run$r"; done; echo
for c in 1 2 3 4 5 6; do
  f=$(ls final/phase2/$algo/$inst/$arm/c$c/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  [ -z "$f" ] && continue
  printf "c%-5s" "$c"
  awk -F';' '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {
      for (r=0; r<5; r++) { col = 7 + r*6 + 4; printf "%12s", $col }   # Best Cmax de cada run
      exit }' "$f"
  echo
done
echo ""
echo "Si las seis filas son IDENTICAS columna a columna, las 30 ejecuciones"
echo "parten de solo 5 poblaciones iniciales distintas (repetidas 6 veces)."
echo ""
echo "=== contraste: un algoritmo MONOLITICO (30 ejecuciones en un proceso) ==="
f=$(ls final/phase2/abce3/$inst/$arm/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
if [ -n "$f" ]; then
  echo "abce3 / $inst / $arm : Best Cmax inicial de las 30 ejecuciones"
  awk -F';' '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {
      for (r=0; r<30; r++) { col = 7 + r*6 + 4; printf "%s ", $col }
      print ""; exit }' "$f" | tr ' ' '\n' | grep -v '^$' | sort -u | wc -l | \
      xargs -I{} echo "  valores iniciales DISTINTOS entre las 30 ejecuciones: {} de 30"
fi
