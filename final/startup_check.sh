#!/usr/bin/env bash
# Espera hasta que los workers de Fase 2 arranquen (o 20 min) y reporta.
cd "$(dirname "$0")/.."
for i in $(seq 1 40); do
  n=$(pgrep -c FuzzyFW 2>/dev/null || echo 0)
  [ "$n" -gt 0 ] && { echo "ARRANCADO: $n workers a las $(date '+%H:%M:%S')"; head -2 final/phase2/run.log; exit 0; }
  pgrep -f 'run_phase2.sh' >/dev/null 2>&1 || { echo "run_phase2 MUERTO antes de arrancar workers"; exit 1; }
  sleep 30
done
echo "TIMEOUT: 20 min sin workers (revisar final/phase2_nohup.log)"