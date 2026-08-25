#!/usr/bin/env bash
# FASE 1 — Calibracion de presupuestos: pre-runs A0 de ga y feabcls sobre las 8
# instancias estratificadas (abce3/tsn2 ya tienen trazas A0 de Fase 0, 30 runs).
# 5 runs, generations=2000 (ga) / 600 (feabcls), techo 1800s. Idempotente, <=14.
cd "$(dirname "$0")/.."
PAR=14
OUT=final/phase1
mkdir -p "$OUT"
INSTS="tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05"

: > "$OUT/joblist.txt"; todo=0
for algo in ga feabcls; do
  for inst in $INSTS; do
    d="$OUT/$algo/$inst"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    [ -n "$csv" ] && continue
    todo=$((todo+1)); rm -rf "$d"; mkdir -p "$d"
    sed -e "s/^runs = .*/runs = 5/" -e "s/^seed = .*/seed = 1/" "repro/setup_${algo}.txt" > "$d/setup.txt"
    if [ "$algo" = "ga" ]; then
      sed -i "s/^noimprovement = .*/generations = 2000/" "$d/setup.txt"
    else
      sed -i "s/^noimprovement = .*/generations = 600/" "$d/setup.txt"
    fi
    grep -q '^timelimit' "$d/setup.txt" || echo "timelimit = 1800" >> "$d/setup.txt"
    echo "./FuzzyFW '$d/setup.txt' 'SelectosYTaillardIntervalos/${inst}.F.15_01.txt' '$d' > '$d/log.txt' 2>&1" >> "$OUT/joblist.txt"
  done
done
echo "Phase1 pendientes: $todo"
echo "Inicio PHASE1: $(date)  |  $todo procesos, <=$PAR, 5 runs" | tee -a "$OUT/run.log"
cat "$OUT/joblist.txt" | xargs -I CMD -P "$PAR" bash -c CMD
echo "PHASE1 DONE: $(date)" | tee -a "$OUT/run.log"
