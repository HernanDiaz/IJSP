#!/usr/bin/env bash
# Genera los pools v2 corregidos (componentwise) para las 6 instancias nuevas de Fase 0
# y registra el test de consistencia de cada uno.
set -e
cd "$(dirname "$0")/.."
mkdir -p pools_test/corrected
for inst in tai15_15_01 tai30_15_01 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do
  f="SelectosYTaillardIntervalos/${inst}.F.15_01.txt"
  p="pools_test/int__${inst}_v2_pool.csv"
  o="pools_test/corrected/int__${inst}_v2_repo_pool.csv"
  [ -f "$f" ] || { echo "FALTA instancia $f"; exit 1; }
  [ -f "$p" ] || { echo "FALTA pool $p"; exit 1; }
  echo "=== $inst ==="
  ./seed_consistency_test "$f" "$p" --rewrite "$o" 2>/dev/null | grep -E 'lineas=|coincide|escrito'
done
echo "PHASE0 POOLS OK"
