#!/usr/bin/env bash
# Espera a que terminen los 14 trabajos en vuelo (los que sobrevivieron al kill
# del coordinador). Termina cuando no queda ningun FuzzyFW vivo.
cd "$(dirname "$0")/.."
for i in $(seq 1 240); do   # hasta ~2h
  n=$(ps -eo comm | grep -c '^FuzzyFW$')
  [ "$n" -eq 0 ] && { echo "TODOS TERMINADOS  $(date '+%H:%M:%S')"; break; }
  sleep 30
done
echo "workers vivos: $(ps -eo comm | grep -c '^FuzzyFW$')"
echo "=== configs GA completas ahora ==="
find final/phase2/ga -name '*.csv' 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l
echo "=== progreso final de las 14 en vuelo ==="
for i in 03 04; do
  for a in A0 V2H V2 MOR GT GP MIX; do
    d="final/phase2/ga/tai50_15_$i/$a"
    c=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    s=$(ls "$d"/*_Sols.csv 2>/dev/null | head -1); n=0; [ -n "$s" ] && n=$(( $(wc -l < "$s") - 1 ))
    echo "  tai50_15_$i/$a : $n/30 $([ -n "$c" ] && echo '(COMPLETA)' || echo '(sin stats)')"
  done
done
