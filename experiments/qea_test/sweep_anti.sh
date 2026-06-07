#!/bin/bash
# Sweep for anti-rotation step schedule on the target baseline.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_anti
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500
INSTS="ft10_10 ft20_05 la25_04"

# (anti_step_start, anti_step_end). 0/0 = disabled (baseline).
# Anti-step is a fraction of the rotation step (which goes 0.02 -> 0.08).
PAIRS=(
  "0 0"           # baseline
  "0.005 0.005"   # mild constant
  "0.01 0.01"     # moderate constant
  "0 0.01"        # ramp up to moderate
  "0 0.02"        # ramp up to strong
  "0.005 0.02"    # mild -> strong
  "0.02 0"        # strong at start, taper off
)

for pair in "${PAIRS[@]}"; do
  read as ae <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/as${as}_ae${ae}_${inst}"
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
qea.target_w_start = 0.0
qea.target_w_end = 1.0
qea.anti_step_start = $as
qea.anti_step_end = $ae
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
echo SWEEP_ANTI_DONE
