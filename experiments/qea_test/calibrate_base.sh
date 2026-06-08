#!/bin/bash
# Calibrate G (generations fitting in 4x GA) for the SIMPLE positional baseline
# (commit d8f1d66: samples 250, rotation 0.04, floor 0.01, no schedules).
# Run SINGLE-PROCESS (sequential, no contention) so the time is the reportable,
# GA-comparable single-process time. timelimit is valid here since there are no
# schedules to leave incomplete.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/calibrate_base
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
qea.rotation = 0.04
qea.floor = 0.01
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
# SEQUENTIAL (no &) => single process => clean time
for inst in $INSTS; do
  d="$OUT/$inst"; mkdir -p "$d"
  emit_setup "$(tl_for "$inst")" "$RUNS" > "$d/setup.txt"
  ../FuzzyFW "$d/setup.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/out.log" 2>&1
done
echo CALIBRATE_BASE_DONE

printf "%-9s %8s %10s\n" INST 4xGA G
for inst in $INSTS; do
  csv=$(ls "$OUT/$inst"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  g=$(grep -m1 "^Number of Generations;" "$csv" | tr -d '\r' | cut -d';' -f2)
  gf=$(awk -v x="$g" 'BEGIN{printf "%d", x}')
  printf "%-9s %8s %10s\n" "$inst" "$(tl_for "$inst")" "$gf"
done
