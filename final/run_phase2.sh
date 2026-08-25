#!/usr/bin/env bash
# FASE 2 — Experimento final: 61 instancias x 4 solvers x 7 brazos x RUNS runs.
# Presupuesto por celda (ENMIENDA documentada pre-lanzamiento):
#   budget(algo, clase) = clamp(1.5 x mediana t_conv(A0), FLOOR, CAP) segundos CPU
#   (suelo FLOOR=60s por granularidad; CAP=900s regimen Fase B). ft10 usa la
#   columna de su tamano (menor que 15x15). Todos los brazos de una celda
#   reciben identico presupuesto; generations=100000 solo como respaldo.
# Brazos: A0 aleatorio; V2H=v2@125 (contraste 50%); V2/MOR/GT/GP/MIX @250 (p*=100%).
# Pools: corregidos (componentwise), i.i.d. (decision pre-registro; FTOP=sub-estudio).
# Orden de lanzamiento: abce3 -> ga -> feabcls -> tsn2 (resultados incrementales).
# Idempotente; <=PAR procesos.
cd "$(dirname "$0")/.."
RUNS=30
PAR=${PAR:-8}
CAP=900
FLOOR=60
OUT=final/phase2
mkdir -p "$OUT"

# presupuestos por clase (de final/phase1/tconv.csv, regla clamp):
#            15x15 20x20 30x15 30x20 50x15 50x20
declare -A BGA=(  [15x15]=60  [20x20]=60  [30x15]=109 [30x20]=154 [50x15]=233 [50x20]=321 )
declare -A BAB=(  [15x15]=60  [20x20]=60  [30x15]=60  [30x20]=60  [50x15]=60  [50x20]=60 )
declare -A BFL=(  [15x15]=60  [20x20]=67  [30x15]=371 [30x20]=$CAP [50x15]=$CAP [50x20]=$CAP )
declare -A BTS=(  [15x15]=60  [20x20]=486 [30x15]=630 [30x20]=$CAP [50x15]=$CAP [50x20]=$CAP )

budget() { # $1=algo $2=clase
  case "$1" in
    ga)      echo "${BGA[$2]}" ;;
    abce3)   echo "${BAB[$2]}" ;;
    feabcls) echo "${BFL[$2]}" ;;
    tsn2)    echo "${BTS[$2]}" ;;
  esac
}
clase() { case "$1" in
  ft10) echo 15x15 ;;
  tai15_15_*) echo 15x15 ;; tai20_20_*) echo 20x20 ;; tai30_15_*) echo 30x15 ;;
  tai30_20_*) echo 30x20 ;; tai50_15_*) echo 50x15 ;; tai50_20_*) echo 50x20 ;;
esac; }
instfile() { if [ "$1" = "ft10" ]; then echo "SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt";
  else echo "SelectosYTaillardIntervalos/$1.F.15_01.txt"; fi }

INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done

# brazo -> generador,k
armspec() { case "$1" in
  A0)  echo ", 0" ;;
  V2H) echo "v2, 125" ;;
  V2)  echo "v2, 250" ;;
  MOR) echo "graspmor, 250" ;;
  GT)  echo "gtmwkr, 250" ;;
  GP)  echo "gp, 250" ;;
  MIX) echo "mix, 250" ;;
esac; }
ARMS="A0 V2H V2 MOR GT GP MIX"

: > "$OUT/joblist.txt"; skip=0; todo=0
for algo in abce3 ga feabcls tsn2; do
  for inst in $INSTS; do
    cls=$(clase "$inst"); bud=$(budget "$algo" "$cls"); f=$(instfile "$inst")
    for arm in $ARMS; do
      d="$OUT/$algo/$inst/$arm"
      csv=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -n "$csv" ] && { skip=$((skip+1)); continue; }
      todo=$((todo+1)); rm -rf "$d"; mkdir -p "$d"
      sed -e "s/^runs = .*/runs = $RUNS/" -e "s/^seed = .*/seed = 1/" "repro/setup_${algo}.txt" > "$d/setup.txt"
      sed -i "s/^noimprovement = .*/generations = 100000/" "$d/setup.txt"
      if grep -q '^timelimit' "$d/setup.txt"; then sed -i "s/^timelimit = .*/timelimit = $bud/" "$d/setup.txt"
      else echo "timelimit = $bud" >> "$d/setup.txt"; fi
      spec=$(armspec "$arm"); g=${spec%%,*}; k=$(echo "${spec##*,}" | tr -d ' ')
      if [ "$k" -gt 0 ]; then
        pool="pools_test/corrected/int__${inst}_${g}_repo_pool.csv"
        [ -f "$pool" ] || { echo "FALTA pool: $pool"; exit 1; }
        sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" "$d/setup.txt"
        { echo ""; echo "creation.seed.pool = $pool"; echo "creation.seed.count = $k"; } >> "$d/setup.txt"
      fi
      echo "./FuzzyFW '$d/setup.txt' '$f' '$d' > '$d/log.txt' 2>&1" >> "$OUT/joblist.txt"
    done
  done
done
echo "Saltadas: $skip  |  Pendientes: $todo (de $((61*4*7)))"
if [ "${DRYRUN:-0}" = "1" ]; then
  echo "DRYRUN: setups y joblist generados; NO se lanza nada."
  exit 0
fi
echo "Inicio PHASE2: $(date)  |  $todo procesos, <=$PAR, $RUNS runs, CAP=${CAP}s FLOOR=${FLOOR}s" | tee -a "$OUT/run.log"
cat "$OUT/joblist.txt" | xargs -I CMD -P "$PAR" bash -c CMD
echo "PHASE2 DONE: $(date)" | tee -a "$OUT/run.log"
