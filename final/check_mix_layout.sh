#!/usr/bin/env bash
# ¿Como esta ORDENADO el pool MIX? Critico: con ejecucion por trozos solo se usan
# las primeras 1250 entradas. Si el pool esta por BLOQUES de generador, el brazo
# MIX de fEABCLS y TSN2 no habria sido mixto en absoluto, sino un solo generador.
# Si esta intercalado, las primeras 1250 son una muestra mixta representativa.
cd "$(dirname "$0")/.."
inst=${1:-tai30_20_01}
P=pools_test/corrected
mix="$P/int__${inst}_mix_repo_pool.csv"

# firma de una solucion = su permutacion (sin el makespan)
sig() { cut -d';' -f1 "$1" | sed 's/[[:space:]]*$//'; }

for g in v2 graspmor gtmwkr gp; do
  f="$P/int__${inst}_${g}_repo_pool.csv"
  [ -f "$f" ] || continue
  sig "$f" | sort -u > "/tmp/sig_$g.txt"
done

echo "=== $inst : de que generador procede cada tramo del pool MIX ==="
printf "%-14s %8s %8s %8s %8s %8s\n" "tramo" "v2" "graspmor" "gtmwkr" "gp" "otros"
total=$(wc -l < "$mix")
for start in 1 1251 2501 3751 5001 6251; do
  end=$((start + 1249)); [ "$end" -gt "$total" ] && end=$total
  sed -n "${start},${end}p" "$mix" | cut -d';' -f1 | sed 's/[[:space:]]*$//' | sort -u > /tmp/sig_tramo.txt
  n=$(wc -l < /tmp/sig_tramo.txt)
  line=$(printf "%-14s" "${start}-${end}")
  otros=$n
  for g in v2 graspmor gtmwkr gp; do
    if [ -f "/tmp/sig_$g.txt" ]; then
      c=$(comm -12 /tmp/sig_tramo.txt "/tmp/sig_$g.txt" | wc -l)
    else c=0; fi
    line="$line $(printf '%8d' "$c")"
    otros=$((otros - c))
  done
  echo "$line $(printf '%8d' "$otros")"
done
echo ""
echo "(\"otros\" = soluciones que no coinciden con ningun pool de generador unico:"
echo " son las aleatorias del pool mixto, o duplicados no emparejables)"
