#!/bin/bash
# Sweep for max-probability ceiling schedule on top of the tau baseline.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_ceil
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500
INSTS="ft10_10 ft20_05 la25_04"

# (ceil_start, ceil_end). 1.0/1.0 = baseline. Lower = stronger cap.
PAIRS=(
  "1.0 1.0"   # baseline (no cap)
  "1.0 0.95"  # gentle cap, late
  "1.0 0.90"  # moderate cap, late
  "1.0 0.85"  # strong cap, late
  "0.99 0.85" # gentle->strong cap throughout
  "0.95 0.85" # moderate->strong cap throughout
  "0.95 0.95" # constant moderate cap
)

for pair in "${PAIRS[@]}"; do
  read cs ce <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/cs${cs}_ce${ce}_${inst}"
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
qea.ceil_start = $cs
qea.ceil_end = $ce
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
echo SWEEP_CEIL_DONE
