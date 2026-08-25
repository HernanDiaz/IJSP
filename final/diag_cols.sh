#!/usr/bin/env bash
# Compara la monotonia de la columna 3 ("Best" = mejor histórico) frente a la
# columna 5 ("Best Cmax" = estadistica 1, que puede ser el mejor de la POBLACION
# actual y por tanto NO monotona).
cd "$(dirname "$0")/.."
c=$(ls final/phase2/abce3/tai50_20_01/A0/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
awk -F';' '
  /^Evolution/{f=1}
  f && $1 ~ /^[0-9]+$/ {
    n++; c3=$3+0; c5=$5+0;
    if (n>1) { if (c3 > p3 + 1e-9) up3++; if (c5 > p5 + 1e-9) up5++ }
    p3=c3; p5=c5;
    if (n<=2 || n>=284) printf "  fila %d: col3(Best)=%.2f  col5(BestCmax)=%.2f\n", n, c3, c5
  }
  END{
    printf "\n  subidas en col3 (Best):      %d de %d\n", up3, n-1
    printf "  subidas en col5 (Best Cmax): %d de %d\n", up5, n-1
    print "\n  (una serie de mejor-historico NO debe subir nunca)"
  }' "$c"
