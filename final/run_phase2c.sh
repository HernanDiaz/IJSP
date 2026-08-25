#!/usr/bin/env bash
# FASE 2 (version CHUNKED) — igual que run_phase2.sh pero partiendo los 30 runs de
# cada configuracion en trozos independientes, para acotar la FUGA DE MEMORIA del
# solver (medida 2026-07-26: ~400MB -> ~1GB por proceso a lo largo de 30 runs).
#
# Cada trozo es un proceso nuevo (muere y libera). Semillas: el framework usa
# seed+run, asi que trozo j con seed=1+j*CH y runs=CH reproduce EXACTAMENTE las
# mismas 30 semillas (1..30) que un unico runs=30 -> identico estadisticamente.
#
# Salida: final/phase2/<algo>/<inst>/<arm>/c<j>/   (un subdir por trozo)
# Una configuracion se considera terminada cuando TODOS sus trozos tienen stats CSV.
# Idempotente a nivel de trozo (se puede cortar y reanudar sin perder nada).
#
# Variables: PAR (default 10), CH (runs por trozo; default 5 para ga/abce3,
#            3 para feabcls/tsn2 que consumen mas por run).
cd "$(dirname "$0")/.."
RUNS=30
PAR=${PAR:-10}
CAP=900
OUT=${OUT_OVERRIDE:-final/phase2}
mkdir -p "$OUT"

declare -A BGA=(  [15x15]=60  [20x20]=60  [30x15]=109 [30x20]=154 [50x15]=233 [50x20]=321 )
declare -A BAB=(  [15x15]=60  [20x20]=60  [30x15]=60  [30x20]=60  [50x15]=60  [50x20]=60 )
declare -A BFL=(  [15x15]=60  [20x20]=67  [30x15]=371 [30x20]=$CAP [50x15]=$CAP [50x20]=$CAP )
declare -A BTS=(  [15x15]=60  [20x20]=486 [30x15]=630 [30x20]=$CAP [50x15]=$CAP [50x20]=$CAP )
# FORCEBUD: presupuesto forzado, SOLO para el smoke test. Permite recorrer el
# pipeline completo en minutos por el MISMO camino de codigo que la corrida real.
budget() { if [ -n "$FORCEBUD" ]; then echo "$FORCEBUD"; return; fi
  case "$1" in ga) echo "${BGA[$2]}";; abce3) echo "${BAB[$2]}";; feabcls) echo "${BFL[$2]}";; tsn2) echo "${BTS[$2]}";; esac; }
chunk_size() { echo "${CH:-5}"; }   # 5 runs por trozo -> pico ~325 MB/worker (medido)
clase() { case "$1" in
  ft10|tai15_15_*) echo 15x15 ;; tai20_20_*) echo 20x20 ;; tai30_15_*) echo 30x15 ;;
  tai30_20_*) echo 30x20 ;; tai50_15_*) echo 50x15 ;; tai50_20_*) echo 50x20 ;;
esac; }
instfile() { if [ "$1" = "ft10" ]; then echo "SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt";
  else echo "SelectosYTaillardIntervalos/$1.F.15_01.txt"; fi }
armspec() { case "$1" in
  A0) echo ", 0";; V2H) echo "v2, 125";; V2) echo "v2, 250";; MOR) echo "graspmor, 250";;
  GT) echo "gtmwkr, 250";; GP) echo "gp, 250";; MIX) echo "mix, 250";;
esac; }

# INSTS_OVERRIDE y OUT_OVERRIDE: para el smoke test, mismo codigo pero
# pocas instancias y directorio aparte, sin tocar los datos reales.
if [ -n "$INSTS_OVERRIDE" ]; then
  INSTS="$INSTS_OVERRIDE"
else
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
fi
ARMS="A0 V2H V2 MOR GT GP MIX"
ALGOS=${ALGOS:-"ga feabcls tsn2"}   # abce3 ya completo; ga parcialmente

: > "$OUT/joblist_chunk.txt"; skipc=0; skipm=0; todo=0
for algo in $ALGOS; do
  ch=$(chunk_size "$algo")
  nch=$(( (RUNS + ch - 1) / ch ))
  for inst in $INSTS; do
    cls=$(clase "$inst"); bud=$(budget "$algo" "$cls"); f=$(instfile "$inst")
    for arm in $ARMS; do
      base="$OUT/$algo/$inst/$arm"
      # si la config ya se completo en el formato antiguo (runs=30 monolitico), saltar
      mono=$(ls "$base"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -n "$mono" ] && { skipm=$((skipm+1)); continue; }
      spec=$(armspec "$arm"); g=${spec%%,*}; k=$(echo "${spec##*,}" | tr -d ' ')
      for j in $(seq 1 $nch); do
        d="$base/c$j"
        csv=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
        [ -n "$csv" ] && { skipc=$((skipc+1)); continue; }
        todo=$((todo+1)); rm -rf "$d"; mkdir -p "$d"
        seed=$(( 1 + (j-1)*ch ))
        sed -e "s/^runs = .*/runs = $ch/" -e "s/^seed = .*/seed = $seed/" "repro/setup_${algo}.txt" > "$d/setup.txt"
        sed -i "s/^noimprovement = .*/generations = 100000/" "$d/setup.txt"
        if grep -q '^timelimit' "$d/setup.txt"; then sed -i "s/^timelimit = .*/timelimit = $bud/" "$d/setup.txt"
        else echo "timelimit = $bud" >> "$d/setup.txt"; fi
        if [ "$k" -gt 0 ]; then
          pool="pools_test/corrected/int__${inst}_${g}_repo_pool.csv"
          [ -f "$pool" ] || { echo "FALTA pool: $pool"; exit 1; }
          sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" "$d/setup.txt"
          # INDICE GLOBAL de la primera ejecucion de este trozo. Sin el, cada
          # proceso reinicia su contador y los 6 trozos repiten los mismos
          # bloques del pool: 5 poblaciones iniciales en vez de 30.
          { echo ""; echo "creation.seed.pool = $pool"; echo "creation.seed.count = $k";
            echo "creation.seed.offset = $(( (j-1)*ch ))"; } >> "$d/setup.txt"
        fi
        echo "./FuzzyFW '$d/setup.txt' '$f' '$d' > '$d/log.txt' 2>&1" >> "$OUT/joblist_chunk.txt"
      done
    done
  done
done
echo "configs ya completas (monoliticas): $skipm | trozos ya hechos: $skipc | trozos pendientes: $todo"
if [ "${DRYRUN:-0}" = "1" ]; then echo "DRYRUN: nada lanzado."; exit 0; fi
echo "Inicio PHASE2C: $(date) | $todo trozos, <=$PAR, chunk ga/abce3=${CH:-5} feabcls/tsn2=${CH:-3}" | tee -a "$OUT/run.log"
cat "$OUT/joblist_chunk.txt" | xargs -I CMD -P "$PAR" bash -c CMD
echo "PHASE2C DONE: $(date)" | tee -a "$OUT/run.log"
