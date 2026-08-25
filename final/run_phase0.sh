#!/usr/bin/env bash
# FASE 0 — Sub-estudio de fraccion de siembra (fija p* del experimento final).
# PRE-REGISTRADO:
#   Brazos: F00=A0(random), F10=k25, F25=k62, F50=k125, F75=k187, F100=k250 (v2, pop 250)
#   Solvers: abce3 (criterio calidad final) y tsn2 (criterio time-to-target)
#   Parada: abce3 generations=600 (3x su meseta observada) + timelimit=1800s de techo;
#           tsn2 generations=200 + timelimit=900s (regimen Fase B del paper)
#   30 runs, semillas 1..30 pareadas. 8 instancias estratificadas (ninguna marcada
#   por tuning del TS): tai15_15_01, tai20_20_02, tai30_15_01, tai30_20_04,
#   tai50_15_01, tai50_15_05, tai50_20_01, tai50_20_05.
#   Regla de decision p*: mejor rango medio combinando (i) avg E[Cmax] final en
#   abce3 y (ii) TTT a la calidad final de A0 en tsn2, sobre las 8 instancias;
#   empate -> p menor.
# Idempotente (salta configs con stats CSV). <=14 procesos.
cd "$(dirname "$0")/.."
RUNS=30; PAR=14
OUT=final/phase0
mkdir -p "$OUT"
INSTS="tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05"
ARMS="F00 F10 F25 F50 F75 F100"
declare -A K=( [F00]=0 [F10]=25 [F25]=62 [F50]=125 [F75]=187 [F100]=250 )

: > "$OUT/joblist.txt"; skip=0; todo=0
for algo in abce3 tsn2; do
  for inst in $INSTS; do
    for arm in $ARMS; do
      d="$OUT/$algo/$inst/$arm"
      csv=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      if [ -n "$csv" ]; then skip=$((skip+1)); continue; fi
      todo=$((todo+1)); rm -rf "$d"; mkdir -p "$d"
      sed -e "s/^runs = .*/runs = $RUNS/" -e "s/^seed = .*/seed = 1/" "repro/setup_${algo}.txt" > "$d/setup.txt"
      k=${K[$arm]}
      if [ "$k" -gt 0 ]; then
        pool="pools_test/corrected/int__${inst}_v2_repo_pool.csv"
        [ -f "$pool" ] || { echo "FALTA pool: $pool"; exit 1; }
        sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" "$d/setup.txt"
        { echo ""; echo "creation.seed.pool = $pool"; echo "creation.seed.count = $k"; } >> "$d/setup.txt"
      fi
      if [ "$algo" = "abce3" ]; then
        sed -i "s/^noimprovement = .*/generations = 600/" "$d/setup.txt"
        grep -q '^timelimit' "$d/setup.txt" || echo "timelimit = 1800" >> "$d/setup.txt"
      else
        sed -i "s/^noimprovement = .*/generations = 200/" "$d/setup.txt"   # timelimit=900 ya en base
      fi
      echo "./FuzzyFW '$d/setup.txt' 'SelectosYTaillardIntervalos/${inst}.F.15_01.txt' '$d' > '$d/log.txt' 2>&1" >> "$OUT/joblist.txt"
    done
  done
done
echo "Saltadas: $skip  |  Pendientes: $todo"
echo "Inicio PHASE0: $(date)  |  $todo procesos, <=$PAR, $RUNS runs" | tee -a "$OUT/run.log"
cat "$OUT/joblist.txt" | xargs -I CMD -P "$PAR" bash -c CMD
echo "PHASE0 DONE: $(date)" | tee -a "$OUT/run.log"
