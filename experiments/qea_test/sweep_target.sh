#!/bin/bash
# Sweep for rotation target-choice schedule: P(use bestSoFar) varies over the budget.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_target
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500
INSTS="ft10_10 ft20_05 la25_04"

# (target_w_start, target_w_end). 1.0/1.0 = baseline (always bestSoFar).
PAIRS=(
  "1.0 1.0"   # baseline
  "0.5 1.0"   # half-mix start -> always bestSoFar at end
  "0.7 1.0"   # mild mix start
  "0.3 1.0"   # strong mix at start
  "0.5 0.9"   # half-mix start, mostly bestSoFar end
  "0.0 1.0"   # always genBest at start -> always bestSoFar at end (extreme)
  "1.0 0.5"   # reverse direction: more genBest at end
)

for pair in "${PAIRS[@]}"; do
  read ws we <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/ws${ws}_we${we}_${inst}"
    mkdir -p "$d"
    cat > "$d/setup.txt" <<EOF
algorithm = QEA
seed = 1
runs = $RUNS
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
qea.target_w_start = $ws
qea.target_w_end = $we
generations = $GEN
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = iteration
evolution.span = 100
EOF
    ( ../FuzzyFW "$d/setup.txt" \
        "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" \
        "$d" > "$d/out.log" 2>&1 ) &
  done
done
wait
echo SWEEP_TARGET_DONE
