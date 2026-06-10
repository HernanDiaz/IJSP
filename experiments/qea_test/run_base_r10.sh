#!/bin/bash
# NEW time-budget baseline after ITER 1: the SIMPLE positional engine (commit
# d8f1d66, no schedules) with the rotation constant RE-TUNED 0.04 -> 0.10 (the
# only validated win of iter 1; the rotation *schedule* gave no gain over a
# constant). Same per-instance G as run_base.sh (rotation magnitude does not
# change cost/gen, so runtime stays <= 4x GA). Expected AvgRE ~= 15.94%
# (cf. run_base.sh = 16.54% at rotation 0.04).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUTROOT="${1:-experiments/qea_test/run30_base_r10}"
RUNS="${2:-30}"
INSTS="abz7_06 abz8_05 abz9_10 ft10_10 ft20_05 la21_04 la24_03 la25_04 la27_09 la29_03 la38_06 la40_05"

gens_for() {  # G calibrated to <= 4x GA for the simple baseline (== run_base.sh)
  case "$1" in
    abz7_06) echo 175;; abz8_05) echo 173;; abz9_10) echo 210;;
    ft10_10) echo 194;; ft20_05) echo 214;;
    la21_04) echo 255;; la24_03) echo 182;; la25_04) echo 222;;
    la27_09) echo 195;; la29_03) echo 155;; la38_06) echo 198;; la40_05) echo 172;;
  esac
}

emit_setup() {  # $1 generations, $2 runs
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
qea.rotation = 0.10
qea.floor = 0.01
generations = $1
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = iteration
evolution.span = 50
EOF
}

rm -rf "$OUTROOT"
for inst in $INSTS; do
  ld="$OUTROOT/$inst"; mkdir -p "$ld"
  emit_setup "$(gens_for "$inst")" "$RUNS" > "$ld/setup.txt"
  ( ../FuzzyFW "$ld/setup.txt" \
      "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" \
      "$ld" > "$ld/stdout.log" 2>&1 ) &
done
wait
echo RUN_BASE_R10_DONE
QEA_OUT="$OUTROOT" bash experiments/qea_test/parse_qea.sh
