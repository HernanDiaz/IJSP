#!/usr/bin/env bash
# Estado rapido de Fase 2: un solo find, cuenta stats CSV por solver.
cd "$(dirname "$0")/.."
echo "hora: $(date '+%F %H:%M:%S')"
pgrep -f 'run_phase2.sh' >/dev/null 2>&1 && echo "run_phase2 vivo: SI" || echo "run_phase2 vivo: NO"
echo "FuzzyFW vivos: $(pgrep -c FuzzyFW 2>/dev/null)"
find final/phase2 -name '*.csv' 2>/dev/null | grep -v -E 'Sols|Robust|Scenar|results_|joblist' \
  | awk -F'/' '{c[$3]++} END{t=0; for(a in c){printf "  %-8s %4d/427\n", a, c[a]; t+=c[a]}; printf "TOTAL: %d/1708\n", t}'
tail -1 final/phase2/run.log 2>/dev/null