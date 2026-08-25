#!/usr/bin/env bash
# Extension Fase 0 — brazo FTOP: mejores 250 del pool v2 (seleccion elitista, k=250).
# Testea si la seleccion por calidad bate a la muestra i.i.d. (F100), o si la perdida
# de diversidad penaliza (H3 pura / aviso del doc del piloto).
# Nota: pool de 250 lineas y k=250 -> los 30 runs comparten poblacion inicial (la
# definicion del brazo); la varianza viene del RNG del algoritmo (semillas 1..30).
# Misma parada pre-registrada que la Fase 0. Idempotente, <=14.
cd "$(dirname "$0")/.."
RUNS=30; PAR=14
OUT=final/phase0
INSTS="tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05"

bash final/build_top_pools.sh

: > "$OUT/joblist_top.txt"; todo=0
for algo in abce3 tsn2; do
  for inst in $INSTS; do
    d="$OUT/$algo/$inst/FTOP"
    csv=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    [ -n "$csv" ] && continue
    todo=$((todo+1)); rm -rf "$d"; mkdir -p "$d"
    sed -e "s/^runs = .*/runs = $RUNS/" -e "s/^seed = .*/seed = 1/" "repro/setup_${algo}.txt" > "$d/setup.txt"
    pool="pools_test/corrected/int__${inst}_v2top_repo_pool.csv"
    sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" "$d/setup.txt"
    { echo ""; echo "creation.seed.pool = $pool"; echo "creation.seed.count = 250"; } >> "$d/setup.txt"
    if [ "$algo" = "abce3" ]; then
      sed -i "s/^noimprovement = .*/generations = 600/" "$d/setup.txt"
      grep -q '^timelimit' "$d/setup.txt" || echo "timelimit = 1800" >> "$d/setup.txt"
    else
      sed -i "s/^noimprovement = .*/generations = 200/" "$d/setup.txt"
    fi
    echo "./FuzzyFW '$d/setup.txt' 'SelectosYTaillardIntervalos/${inst}.F.15_01.txt' '$d' > '$d/log.txt' 2>&1" >> "$OUT/joblist_top.txt"
  done
done
echo "FTOP pendientes: $todo"
echo "Inicio PHASE0-TOP: $(date)  |  $todo procesos, <=$PAR" | tee -a "$OUT/run.log"
cat "$OUT/joblist_top.txt" | xargs -I CMD -P "$PAR" bash -c CMD
echo "PHASE0-TOP DONE: $(date)" | tee -a "$OUT/run.log"
