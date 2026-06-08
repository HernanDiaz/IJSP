#!/bin/bash
# Calibrate, per instance, how many GENERATIONS the iter-17 baseline completes
# within 4x the GA median runtime (timelimit used here ONLY as a measuring
# instrument). The reported G is what we will then fix as generations=G for the
# reproducible, by-generations runs.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/calibrate
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=5

tl_for() {  # 4x GA median runtime (s)
  case "$1" in
    abz7_06) echo 7.2;; abz8_05) echo 7.2;; abz9_10) echo 8.8;;
    ft10_10) echo 2.0;; ft20_05) echo 2.8;;
    la21_04) echo 4.4;; la24_03) echo 3.2;; la25_04) echo 4.0;;
    la27_09) echo 5.2;; la29_03) echo 4.4;; la38_06) echo 5.6;; la40_05) echo 4.8;;
  esac
}

emit_setup() {  # $1 timelimit, $2 runs
  cat <<EOF
algorithm = QEA
seed = 1
runs = $2
problem.hasbounds = no
objective = ijsp.makespan
evaluation.interval.comparison = LEX2
evaluation.interval.maximum = Component
evaluation.lamarckism = no
codification = ijsp.job-order
decodification = ijsp.job-order
decode.sgs = insertion
sgs.interval.comparison = LEX2
qea.scheme = positional
qea.samples = 250
qea.samples_start = 100
qea.samples_end = 400
qea.rotation = 0.04
qea.rot_start = 0.02
qea.rot_end = 0.08
qea.floor = 0.005
qea.floor_start = 0.005
qea.floor_end = 0.005
qea.schedule_type = cosine
qea.tau_start = 0.7
qea.tau_end = 1.3
qea.target_w_start = 0.0
qea.target_w_end = 1.0
qea.anti_step_start = 0.005
qea.anti_step_end = 0.02
timelimit = $1
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = time
evolution.span = 1.0
EOF
}

INSTS="abz7_06 abz8_05 abz9_10 ft10_10 ft20_05 la21_04 la24_03 la25_04 la27_09 la29_03 la38_06 la40_05"
for inst in $INSTS; do
  d="$OUT/$inst"; mkdir -p "$d"
  emit_setup "$(tl_for "$inst")" "$RUNS" > "$d/setup.txt"
  ( ../FuzzyFW "$d/setup.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/out.log" 2>&1 ) &
done
wait
echo CALIBRATE_DONE

printf "%-9s %8s %12s %12s\n" INST 4xGA gens_avg gens_floor
for inst in $INSTS; do
  csv=$(ls "$OUT/$inst"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  g=$(grep -m1 "^Number of Generations;" "$csv" | tr -d '\r' | cut -d';' -f2)
  gf=$(awk -v x="$g" 'BEGIN{printf "%d", x}')
  printf "%-9s %8s %12s %12s\n" "$inst" "$(tl_for "$inst")" "$g" "$gf"
done
