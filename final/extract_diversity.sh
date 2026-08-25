#!/usr/bin/env bash
# Extrae la DIVERSIDAD ESTRUCTURAL (distancia de Hamming media de la poblacion)
# de las trazas del experimento final/diversity.
# La cabecera es: Step;Runtime;Best;Avg;Best Cmax;Avg Cmax;Avg Hamming;...
# -> col 2 = tiempo, col 7 = Hamming medio (bloque "Average Evolution").
# Salida: final/diversity/hamming.csv  (algo,inst,arm,t,hamming)
cd "$(dirname "$0")/.."
out=final/diversity/hamming.csv
echo "algo,inst,arm,t,hamming" > "$out"
for algo in ga abce3; do
  for inst in tai20_20_01 tai20_20_05 tai30_20_01 tai30_20_05; do
    for arm in A0 V2 MIX; do
      d="final/diversity/$algo/$inst/$arm"
      c=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -z "$c" ] && { echo "  (falta $d)" >&2; continue; }
      awk -F';' -v a="$algo" -v i="$inst" -v ar="$arm" '
        /^Evolution/{f=1; next}
        # OJO: en el bloque "Average Evolution" el paso es DECIMAL (p.ej. 266.2),
        # asi que el patron no puede exigir entero o se pierde casi todo.
        f && $1 ~ /^[0-9]+(\.[0-9]+)?$/ && NF>=7 { h=$7+0; if (h>0) printf "%s,%s,%s,%.1f,%.4f\n", a,i,ar,$2,h }
      ' "$c" >> "$out"
    done
  done
done
echo "$out : $(( $(wc -l < "$out") - 1 )) filas"
