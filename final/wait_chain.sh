#!/usr/bin/env bash
# Espera al final de la cadena completa: FTOP -> Fase 1 (PHASE1 DONE).
cd "$(dirname "$0")/.."
st="?"
for i in $(seq 1 1400); do   # ~23h
  if grep -q 'PHASE1 DONE' final/phase1/run.log 2>/dev/null; then st="DONE"; break; fi
  alive=0
  pgrep -f 'run_phase0_top.sh' >/dev/null 2>&1 && alive=1
  pgrep -f 'auto_phase1.sh' >/dev/null 2>&1 && alive=1
  pgrep -f 'run_phase1_calib.sh' >/dev/null 2>&1 && alive=1
  [ "$alive" = "0" ] && { st="CADENA_MUERTA_SIN_DONE"; break; }
  sleep 60
done
echo "ESTADO CADENA (FTOP+Fase1): $st   $(date)"
ftop=0
for algo in abce3 tsn2; do for inst in tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do
  c=$(ls "final/phase0/$algo/$inst/FTOP"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  [ -n "$c" ] && ftop=$((ftop+1))
done; done
echo "FTOP completas: $ftop/16"
p1=0
for algo in ga feabcls; do for inst in tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do
  c=$(ls "final/phase1/$algo/$inst"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  [ -n "$c" ] && p1=$((p1+1))
done; done
echo "Fase1 (calibracion ga/feabcls) completas: $p1/16"
tail -2 final/phase0/run.log 2>/dev/null; tail -2 final/phase1/run.log 2>/dev/null
