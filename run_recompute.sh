#!/usr/bin/env bash
cd "$(dirname "$0")"
mkdir -p pools_test/corrected
declare -A INST=(
  [tai15_15_05]="SelectosYTaillardIntervalos/tai15_15_05.F.15_01.txt"
  [tai20_20_02]="SelectosYTaillardIntervalos/tai20_20_02.F.15_01.txt"
  [tai30_20_04]="SelectosYTaillardIntervalos/tai30_20_04.F.15_01.txt"
)
declare -A LB=( [tai15_15_05]=1224 [tai20_20_02]=1561 [tai30_20_04]=1948 )

for inst in tai15_15_05 tai20_20_02 tai30_20_04; do
  for gen in v2 graspmor gtmwkr gp; do
    pool="pools_test/int__${inst}_${gen}_pool.csv"
    [ -f "$pool" ] || { echo "FALTA $pool"; continue; }
    out="pools_test/corrected/int__${inst}_${gen}_repo_pool.csv"
    echo "=================================================================="
    ./seed_consistency_test "${INST[$inst]}" "$pool" --rewrite "$out" --lb "${LB[$inst]}" 2>/dev/null
  done
done
