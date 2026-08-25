#!/usr/bin/env bash
# Construye pools "v2top": las mejores 250 lineas del pool v2 corregido (por E[Cmax]
# = midpoint del intervalo componentwise), orden ascendente. Para el brazo FTOP.
set -e
cd "$(dirname "$0")/.."
for inst in tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do
  src="pools_test/corrected/int__${inst}_v2_repo_pool.csv"
  out="pools_test/corrected/int__${inst}_v2top_repo_pool.csv"
  [ -f "$src" ] || { echo "FALTA $src"; exit 1; }
  awk -F';' '{ s=$2; gsub(/[\[\] ]/,"",s); split(s,ab,","); printf "%.1f\t%s\n", (ab[1]+ab[2])/2, $0 }' "$src" \
    | sort -n | head -250 | cut -f2- > "$out"
  echo "$inst -> $out : $(wc -l < "$out") lineas (best E[Cmax]=$(head -1 "$out" | sed 's/.*;//'))"
done
echo "TOP POOLS OK"
