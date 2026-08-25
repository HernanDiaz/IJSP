#!/usr/bin/env bash
cd "$(dirname "$0")/.."
echo "=== que se esta ejecutando ahora ==="
ps -eo cmd 2>/dev/null | grep -oE 'phase2/[a-z0-9]+/[a-z0-9_]+/[A-Z0-9]+/c[0-9]' | sort | uniq -c | head -6
echo ""
echo "=== trozos completados por algoritmo ==="
for a in ga feabcls tsn2; do
  n=$(find "final/phase2/$a" -path '*/c*' -name '*.csv' 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l)
  # total esperado: configs pendientes x 6
  echo "  $a: $n trozos"
done
echo ""
echo "=== configuraciones COMPLETAS (6/6 trozos) ==="
for a in ga feabcls tsn2; do
  c=0
  for d in final/phase2/$a/*/*/; do
    [ -d "$d" ] || continue
    k=$(ls "$d"c*/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l)
    m=$(ls "$d"*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l)
    { [ "$k" -ge 6 ] || [ "$m" -ge 1 ]; } && c=$((c+1))
  done
  echo "  $a: $c/427"
done
echo ""
echo "=== inicio y tiempo transcurrido ==="
grep 'PHASE2C' final/phase2/run.log | tail -1
date '+ahora: %F %H:%M:%S'
