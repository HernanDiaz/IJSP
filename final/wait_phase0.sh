#!/usr/bin/env bash
cd "$(dirname "$0")/.."
st="?"
for i in $(seq 1 3000); do   # ~50h
  if grep -q 'PHASE0 DONE' final/phase0/run.log 2>/dev/null; then st="DONE"; break; fi
  if ! pgrep -f 'run_phase0.sh' >/dev/null 2>&1; then st="MUERTO_SIN_DONE"; break; fi
  sleep 60
done
echo "ESTADO PHASE0: $st   $(date)"
done=0
for algo in abce3 tsn2; do for inst in tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do for arm in F00 F10 F25 F50 F75 F100; do
  c=$(ls "final/phase0/$algo/$inst/$arm"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  [ -n "$c" ] && done=$((done+1))
done; done; done
echo "configs completas: $done / 96"
tail -2 final/phase0/run.log 2>/dev/null
