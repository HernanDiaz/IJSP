#!/usr/bin/env bash
# Vigia de Fase 2 por hitos. Uso: wait_phase2.sh <abce3|ga|feabcls|tsn2|all>
# Espera a que el solver indicado tenga sus 427 configs completas (o 1708 con
# 'all' / marcador PHASE2 DONE), o a que la cadena muera. Loop hasta ~48h.
cd "$(dirname "$0")/.."
M=${1:-abce3}
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
count() { # $1 = algo|all
  local n=0
  for algo in abce3 ga feabcls tsn2; do
    [ "$1" != "all" ] && [ "$algo" != "$1" ] && continue
    for inst in $INSTS; do for arm in A0 V2H V2 MOR GT GP MIX; do
      c=$(ls "final/phase2/$algo/$inst/$arm"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -n "$c" ] && n=$((n+1))
    done; done
  done
  echo $n
}
tgt=427; [ "$M" = "all" ] && tgt=1708
st="?"
for i in $(seq 1 2880); do   # ~48h
  if [ "$M" = "all" ] && grep -q 'PHASE2 DONE' final/phase2/run.log 2>/dev/null; then st="DONE"; break; fi
  n=$(count "$M")
  [ "$n" -ge "$tgt" ] && { st="MILESTONE_$M"; break; }
  pgrep -f 'run_phase2.sh' >/dev/null 2>&1 || { st="CADENA_MUERTA"; break; }
  sleep 60
done
echo "ESTADO: $st   $(date)"
echo "hito '$M': $(count "$M")/$tgt"
for a in abce3 ga feabcls tsn2; do echo "  $a: $(count $a)/427"; done
tail -2 final/phase2/run.log 2>/dev/null