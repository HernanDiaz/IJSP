#!/usr/bin/env bash
cd "$(dirname "$0")"
declare -A INST=(
  [tai15_15_05]="SelectosYTaillardIntervalos/tai15_15_05.F.15_01.txt"
  [tai20_20_02]="SelectosYTaillardIntervalos/tai20_20_02.F.15_01.txt"
  [tai30_20_04]="SelectosYTaillardIntervalos/tai30_20_04.F.15_01.txt"
)
for inst in tai15_15_05 tai20_20_02 tai30_20_04; do
  for gen in v2 graspmor gtmwkr gp; do
    pool="pools_test/int__${inst}_${gen}_pool.csv"
    [ -f "$pool" ] || { echo "FALTA $pool"; continue; }
    # Captura solo las lineas de resumen
    out=$(./seed_consistency_test "${INST[$inst]}" "$pool" 2>/dev/null)
    comp=$(echo "$out" | grep 'componentwise (repo)' | grep -oE '[0-9]+/[0-9]+')
    lex=$(echo "$out"  | grep 'lex-por-upper'        | grep -oE '[0-9]+/[0-9]+')
    err=$(echo "$out"  | grep 'maxErr'               | grep -oE '[0-9.]+$')
    printf "%-13s %-9s  comp=%-9s lex=%-9s maxErrLower=%s\n" "$inst" "$gen" "$comp" "$lex" "$err"
  done
done
