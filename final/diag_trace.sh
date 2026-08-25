#!/usr/bin/env bash
# Diagnostico: la seccion "Average Evolution" del framework, ¿promedia siempre
# sobre los 30 runs, o va descartando los que terminan? (sesgo de supervivencia
# a nivel de RUN, dentro del propio framework).
cd "$(dirname "$0")/.."
c=$(ls final/phase2/abce3/tai50_20_01/A0/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
echo "CSV: $c"
echo ""
echo "=== cabecera de la seccion Evolution (primeras 2 lineas) ==="
awk '/^Evolution/{f=1} f{c++; if(c>=3 && c<=4) print substr($0,1,220)} c>4{exit}' "$c"
echo ""
echo "=== nº de campos NO vacios por fila, al principio y al final ==="
awk -F';' '/^Evolution/{f=1} f && $1 ~ /^[0-9]+$/ {
  nz=0; for(i=1;i<=NF;i++) if($i!="") nz++;
  n++; if(n<=3 || n>=maxn-2) printf "  fila %d: t=%s bestCmax=%s campos_no_vacios=%d de %d\n", n, $2, $5, nz, NF
} END{}' "$c" | head -3
echo "  ..."
awk -F';' '/^Evolution/{f=1} f && $1 ~ /^[0-9]+$/ { nz=0; for(i=1;i<=NF;i++) if($i!="") nz++; lines[++n]=sprintf("  fila %d: t=%s bestCmax=%s campos_no_vacios=%d de %d", n, $2, $5, nz, NF) } END{ for(i=n-2;i<=n;i++) print lines[i] }' "$c"
