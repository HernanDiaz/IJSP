#!/usr/bin/env bash
# EXPERIMENTO DE MECANISMO: diversidad ESTRUCTURAL de la poblacion a lo largo del
# tiempo, para contrastar la hipotesis del colapso de diversidad.
#
# Decisiones de coste (medidas sobre el codigo):
#  - Se usa HAMMING, no Kendall-tau: kendallTauDistance es O(n^2) POR PAREJA
#    (Individual.cpp:233) y con 250 individuos (31.125 parejas) y hasta 1000 genes
#    serian ~1e10 operaciones por muestreo. Hamming es O(n) -> ~3e7, asumible.
#  - Muestreo cada 10 s (evolution.span=10) para que el coste de instrumentacion
#    no robe tiempo apreciable a la busqueda.
#  - Contraste minimo suficiente: GA (se estanca) vs ABCE3 (se beneficia),
#    brazos A0 / V2 / MIX, 4 instancias medianas, 10 runs.
#
# ATENCION: lanza hasta PAR procesos; NO ejecutar mientras corre la Fase 2
# (limite de 14 procesos del usuario). Pensado para cuando TSN2 termine.
cd "$(dirname "$0")/.."
RUNS=${RUNS:-10}; PAR=${PAR:-12}
OUT=final/diversity
mkdir -p "$OUT"
INSTS="tai20_20_01 tai20_20_05 tai30_20_01 tai30_20_05"
ARMS="A0 V2 MIX"
declare -A BUD=( [tai20_20_01]=60 [tai20_20_05]=60 [tai30_20_01]=154 [tai30_20_05]=154 )
declare -A BUDA=( [tai20_20_01]=60 [tai20_20_05]=60 [tai30_20_01]=60 [tai30_20_05]=60 )

: > "$OUT/joblist.txt"
for algo in ga abce3; do
  for inst in $INSTS; do
    b=${BUD[$inst]}; [ "$algo" = "abce3" ] && b=${BUDA[$inst]}
    for arm in $ARMS; do
      d="$OUT/$algo/$inst/$arm"; rm -rf "$d"; mkdir -p "$d"
      sed -e "s/^runs = .*/runs = $RUNS/" -e "s/^seed = .*/seed = 1/" "repro/setup_${algo}.txt" > "$d/setup.txt"
      sed -i "s/^noimprovement = .*/generations = 100000/" "$d/setup.txt"
      if grep -q '^timelimit' "$d/setup.txt"; then sed -i "s/^timelimit = .*/timelimit = $b/" "$d/setup.txt"
      else echo "timelimit = $b" >> "$d/setup.txt"; fi
      sed -i "s/^evolution.span = .*/evolution.span = 10/" "$d/setup.txt"
      # estadistica 3: diversidad estructural media de la poblacion (Hamming)
      grep -q '^statistics.3' "$d/setup.txt" || printf '\nstatistics.3.metric = average\nstatistics.3.value = hamming\n' >> "$d/setup.txt"
      case "$arm" in
        V2)  pool="pools_test/corrected/int__${inst}_v2_repo_pool.csv";  k=250 ;;
        MIX) pool="pools_test/corrected/int__${inst}_mix_repo_pool.csv"; k=250 ;;
        *)   pool=""; k=0 ;;
      esac
      if [ "$k" -gt 0 ]; then
        sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" "$d/setup.txt"
        { echo ""; echo "creation.seed.pool = $pool"; echo "creation.seed.count = $k"; } >> "$d/setup.txt"
      fi
      echo "./FuzzyFW '$d/setup.txt' 'SelectosYTaillardIntervalos/${inst}.F.15_01.txt' '$d' > '$d/log.txt' 2>&1" >> "$OUT/joblist.txt"
    done
  done
done
N=$(wc -l < "$OUT/joblist.txt")
echo "Inicio DIVERSITY: $(date) | $N procesos, <=$PAR, $RUNS runs, Hamming cada 10s" | tee "$OUT/run.log"
cat "$OUT/joblist.txt" | xargs -I CMD -P "$PAR" bash -c CMD
echo "DIVERSITY DONE: $(date)" | tee -a "$OUT/run.log"
