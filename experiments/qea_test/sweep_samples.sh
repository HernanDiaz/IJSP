#!/bin/bash
# Sweep for samples schedule: samples_start x samples_end on top of the
# current baseline (cosine rot 0.02->0.08 + floor const 0.005).
# Pairs chosen so that average ≈ 250 (same total budget as baseline 500 gens).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_samples
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500
INSTS="ft10_10 ft20_05 la25_04"

PAIRS=(
  "250 250"   # baseline (constant)
  "100 400"   # ramp up: few-many
  "150 350"   # mild ramp up
  "400 100"   # ramp down: many-few
  "350 150"   # mild ramp down
  "200 300"   # subtle ramp up
  "300 200"   # subtle ramp down
)

for pair in "${PAIRS[@]}"; do
  read ss se <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/s${ss}_e${se}_${inst}"
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
qea.samples_start = $ss
qea.samples_end = $se
qea.rotation = 0.04
qea.rot_start = 0.02
qea.rot_end = 0.08
qea.floor = 0.005
qea.floor_start = 0.005
qea.floor_end = 0.005
qea.schedule_type = cosine
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
echo SWEEP_SAMPLES_DONE
