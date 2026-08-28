#!/usr/bin/env bash
# ¿GANA MIX PORQUE SUS SEMILLAS SON MEJORES, O PORQUE SON VARIADAS?
# Se mide la CALIDAD DE LA POBLACION INICIAL que realmente recibe cada brazo,
# leida del paso 0 de las trazas (evaluacion del propio repositorio, no la que
# trae el pool). Dos numeros por brazo:
#   mejor  = E[Cmax] del mejor individuo inicial
#   media  = E[Cmax] medio de los 250 individuos iniciales
# Si MIX arranca IGUAL o PEOR que v2/gp y aun asi termina mejor, la ventaja no
# es calidad de semilla: es composicion del pool.
cd "$(dirname "$0")/.."
algo=${1:-tsn2}
INSTS="tai20_20_05 tai30_15_07 tai30_20_01 tai50_15_04 tai50_20_09"
ARMS="A0 V2H V2 MOR GT GP MIX MIXH"
printf "%-14s %-5s %12s %12s\n" instancia brazo "mejor inicial" "media inicial"
for inst in $INSTS; do
  for arm in $ARMS; do
    base="final/phase2/$algo/$inst/$arm"
    if [ -n "$(ls -d "$base"/c* 2>/dev/null)" ]; then
      fs=$(ls "$base"/c*/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar')
    else
      fs=$(ls "$base"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar')
    fi
    [ -z "$fs" ] && continue
    # paso 0 de cada ejecucion: col 7+r*6+4 = Best Cmax, +5 = Avg Cmax
    read -r b a <<< "$(for f in $fs; do
      awk -F';' '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {
        for (r=0;r<40;r++){ c=7+r*6+4; if (c+1<=NF && $c!="") printf "%s %s\n", $c, $(c+1) } exit }' "$f"
      done | awk '{sb+=$1; sa+=$2; n++} END{ if(n) printf "%.1f %.1f", sb/n, sa/n }')"
    printf "%-14s %-5s %12s %12s\n" "$inst" "$arm" "$b" "$a"
  done
  echo ""
done
