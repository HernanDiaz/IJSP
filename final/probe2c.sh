#!/usr/bin/env bash
cd "$(dirname "$0")/.."
echo "=== arbol de procesos relevante ==="
ps -eo pid,ppid,etimes,cmd | grep -E '[r]un_phase2c|[x]args|[F]uzzyFW' | head -8
echo "=== joblist de trozos (debe llegar a 5796) ==="
wc -l final/phase2/joblist_chunk.txt 2>/dev/null
echo "=== ya aparecio 'Inicio PHASE2C' en run.log? ==="
grep -c 'PHASE2C' final/phase2/run.log 2>/dev/null
echo "=== trozos con setup ya generado ==="
find final/phase2 -name 'setup.txt' -path '*/c*' 2>/dev/null | wc -l
echo "=== trozos ya ejecutados (con stats CSV) ==="
find final/phase2 -path '*/c*' -name '*.csv' 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | wc -l
