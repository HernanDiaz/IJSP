#!/usr/bin/env bash
# Verificacion independiente del makespan reportado, sobre configuraciones reales.
cd "$(dirname "$0")/.."
check() {  # $1=algo $2=inst $3=arm
  local base="final/phase2/$1/$2/$3"
  local s=$(ls "$base"/*_Sols.csv 2>/dev/null | head -1)
  [ -z "$s" ] && s=$(ls "$base"/c1/*_Sols.csv 2>/dev/null | head -1)
  [ -z "$s" ] && { echo "  ($1/$2/$3: sin datos)"; return; }
  echo "### $1 / $2 / $3"
  ./verify_solution "SelectosYTaillardIntervalos/$2.F.15_01.txt" "$s" insertion 2>&1 | tail -4
  echo ""
}
check feabcls tai30_20_01 A0
check feabcls tai50_20_01 A0
check abce3   tai30_20_01 A0
check ga      tai30_20_01 V2
