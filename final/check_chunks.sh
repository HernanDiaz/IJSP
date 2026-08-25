#!/usr/bin/env bash
cd "$(dirname "$0")/.."
echo "=== semillas de los 6 trozos (tsn2/tai50_20_01/V2) — deben ser 1,6,11,16,21,26 ==="
for j in 1 2 3 4 5 6; do
  f="final/phase2/tsn2/tai50_20_01/V2/c$j/setup.txt"
  [ -f "$f" ] || { echo "  c$j: NO existe"; continue; }
  s=$(grep '^seed' "$f" | awk '{print $3}'); r=$(grep '^runs' "$f" | awk '{print $3}')
  echo "  c$j: seed=$s runs=$r  -> semillas $s..$((s+r-1))"
done
echo ""
echo "=== parametros clave del trozo c1 ==="
grep -E '^(algorithm|generations|timelimit|creation|creation.seed.count|evolution)' final/phase2/tsn2/tai50_20_01/V2/c1/setup.txt 2>/dev/null
