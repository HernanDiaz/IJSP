#!/usr/bin/env bash
cd "$(dirname "$0")/.."
fin=$(ls final/phase2/tsn2/*/*/c*/*_Sols.csv 2>/dev/null | wc -l)
echo "TSN2: $fin / 2562 chunks ($(( fin*100/2562 ))%)"
echo "  procesos FuzzyFW vivos: $(ps -eo comm --no-headers | grep -c '^FuzzyFW$')"
echo "  runner run_phase2c vivo: $(ps -eo args --no-headers | grep -c '[r]un_phase2c')"
tot=0; inc=""
for d in final/phase2/tsn2/*/; do
  c=$(ls "$d"*/c*/*_Sols.csv 2>/dev/null | wc -l)
  if [ "$c" -eq 42 ]; then tot=$((tot+1)); else inc="$inc $(basename "$d")($c/42)"; fi
done
echo "  instancias cerradas: $tot / 61"
[ -n "$inc" ] && echo "  incompletas:$inc"
echo ""
echo "=== en curso ahora ==="
ps -eo args --no-headers | grep '[F]uzzyFW' | grep -v 'bash -c' \
  | grep -o 'final/phase2/tsn2/[^ ]*/setup.txt' | sed 's#final/phase2/tsn2/##;s#/setup.txt##' \
  | cut -d/ -f1 | sort | uniq -c
echo "ahora: $(date '+%m-%d %H:%M')"
echo ""
echo "=== cola del log del runner ==="
tail -3 final/phase2/run.log 2>/dev/null || echo "(sin run.log)"
