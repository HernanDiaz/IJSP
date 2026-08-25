#!/usr/bin/env bash
# Proxy de diversidad SIN cómputo nuevo: la traza ya registra el mejor (col 3) y
# la MEDIA de la poblacion (col 4). La brecha relativa (media-mejor)/mejor es un
# indicador estandar de dispersion en el espacio objetivo: si la poblacion
# colapsa sobre el optimo local sembrado, la brecha se cierra.
# Salida: final/phase2/diversity_<algo>.csv  (algo,inst,arm,t,best,avg,spread)
cd "$(dirname "$0")/.."
algo=${1:?uso: diversity_proxy.sh <algo>}
out="final/phase2/diversity_${algo}.csv"
echo "algo,inst,arm,t,best,avg,spread" > "$out"
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
trace() {  # imprime t,best,avg de un CSV de estadisticas
  awk -F';' -v a="$2" -v i="$3" -v ar="$4" '
    /^Evolution/{f=1}
    f && $1 ~ /^[0-9]+$/ {
      b=$3+0; av=$4+0;
      if (b>0) printf "%s,%s,%s,%.1f,%.2f,%.2f,%.4f\n", a, i, ar, $2, b, av, (av-b)/b*100
    }' "$1"
}
for inst in $INSTS; do
  for arm in A0 V2 MIX MOR; do
    base="final/phase2/$algo/$inst/$arm"
    c=$(ls "$base"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    if [ -n "$c" ]; then trace "$c" "$algo" "$inst" "$arm" >> "$out"
    else
      c1=$(ls "$base"/c1/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -n "$c1" ] && trace "$c1" "$algo" "$inst" "$arm" >> "$out"
    fi
  done
done
echo "$out: $(( $(wc -l < "$out") - 1 )) filas"
