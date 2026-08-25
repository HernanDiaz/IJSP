#!/usr/bin/env bash
cd "$(dirname "$0")/.."
d=final/validate
s=$(ls "$d"/*_Sols.csv 2>/dev/null | head -1)
c=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
echo "runs completados: $([ -n "$s" ] && echo $(( $(wc -l < "$s") - 1 )) || echo 0)/5"
if [ -n "$c" ]; then
  echo "generaciones (media): $(grep -i 'Number of Generations' "$c" | awk -F';' '{print $2}')"
  echo "lineas de traza: $(awk '/^Evolution/{f=1} f&&/^[0-9]/{n++} END{print n+0}' "$c")"
  echo "tamano CSV: $(du -h "$c" | cut -f1)"
  echo "--- comparativa ---"
  echo "  ANTES (traza por generacion): 1480 lineas de traza, CSV 3.7M"
fi
