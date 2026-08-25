#!/usr/bin/env bash
cd "$(dirname "$0")/.."
echo "=== estado procesos (R=corriendo, D=bloqueado en I/O, S=durmiendo) ==="
ps -eo stat,comm | grep FuzzyFW | awk '{print substr($1,1,1)}' | sort | uniq -c
echo "=== progreso de las configs en curso ==="
for d in $(ps -eo cmd | grep -oE 'phase2/[a-z0-9]+/[a-z0-9_]+/[A-Z0-9]+' | sort -u); do
  s=$(ls "final/$d"/*_Sols.csv 2>/dev/null | head -1)
  n=0; [ -n "$s" ] && n=$(( $(wc -l < "$s") - 1 ))
  m=""; [ -n "$s" ] && m=$(date -r "$s" '+%H:%M:%S')
  echo "  ${d#phase2/} : $n/30 runs  (ultima escritura $m)"
done
echo "=== hora actual ==="; date '+%H:%M:%S'
echo "=== carga ==="; uptime
