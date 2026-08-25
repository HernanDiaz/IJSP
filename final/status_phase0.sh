#!/usr/bin/env bash
cd "$(dirname "$0")/.."
echo "hora: $(date '+%H:%M:%S')"
pgrep -f 'run_phase0.sh' >/dev/null 2>&1 && echo "run_phase0 vivo: SI" || echo "run_phase0 vivo: NO"
echo "FuzzyFW vivos: $(pgrep -c FuzzyFW 2>/dev/null || echo 0)"
done=0
for algo in abce3 tsn2; do
  d2=0
  for inst in tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do
    for arm in F00 F10 F25 F50 F75 F100; do
      c=$(ls "final/phase0/$algo/$inst/$arm"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -n "$c" ] && { done=$((done+1)); d2=$((d2+1)); }
    done
  done
  echo "  $algo: $d2/48"
done
echo "TOTAL COMPLETAS: $done / 96"
echo ""
echo "=== en curso (config : runs hechos) ==="
for p in $(pgrep -f 'FuzzyFW.*phase0' 2>/dev/null); do
  cmd=$(ps -o cmd= -p "$p" 2>/dev/null | grep -oE 'phase0/[a-z0-9]+/tai[0-9_]+/F[0-9]+' | head -1)
  [ -z "$cmd" ] && continue
  d="final/$cmd"; s=$(ls "$d"/*_Sols.csv 2>/dev/null | head -1); nr=0; [ -n "$s" ] && nr=$(( $(wc -l < "$s") - 1 ))
  echo "  ${cmd#phase0/} : $nr/30"
done | sort -u
tail -2 final/phase0/run.log 2>/dev/null