#!/usr/bin/env bash
# Espera a que fEABCLS complete sus 427 configuraciones (6/6 trozos cada una).
cd "$(dirname "$0")/.."
for i in $(seq 1 2880); do   # hasta ~48h
  c=0
  for d in final/phase2/feabcls/*/*/; do
    [ -d "$d" ] || continue
    k=$(ls "$d"c*/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l)
    m=$(ls "$d"*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l)
    { [ "$k" -ge 6 ] || [ "$m" -ge 1 ]; } && c=$((c+1))
  done
  [ "$c" -ge 427 ] && { echo "FEABCLS COMPLETO: $c/427  $(date)"; exit 0; }
  pgrep -f run_phase2c >/dev/null 2>&1 || { echo "RUNNER MUERTO (feabcls $c/427)"; exit 1; }
  sleep 60
done
echo "TIMEOUT"
