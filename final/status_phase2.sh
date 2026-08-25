#!/usr/bin/env bash
# Estado de la Fase 2: completadas por solver, procesos vivos, ultima actividad.
cd "$(dirname "$0")/.."
echo "hora: $(date '+%F %H:%M:%S')"
pgrep -f 'run_phase2.sh' >/dev/null 2>&1 && echo "run_phase2 vivo: SI" || echo "run_phase2 vivo: NO"
echo "FuzzyFW vivos: $(pgrep -c FuzzyFW 2>/dev/null || echo 0)"
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
total=0; gdone=0
for algo in abce3 ga feabcls tsn2; do
  d2=0; t2=0
  for inst in $INSTS; do for arm in A0 V2H V2 MOR GT GP MIX; do
    t2=$((t2+1)); total=$((total+1))
    c=$(ls "final/phase2/$algo/$inst/$arm"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    [ -n "$c" ] && { d2=$((d2+1)); gdone=$((gdone+1)); }
  done; done
  printf "  %-8s %4d/%d\n" "$algo" "$d2" "$t2"
done
echo "TOTAL: $gdone / $total"
tail -2 final/phase2/run.log 2>/dev/null