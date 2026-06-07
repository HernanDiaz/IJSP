#!/bin/bash
# Sweep for cosine annealing: linear (baseline) vs cosine on top of the
# current best (rot 0.02->0.08, floor 0.02->0.005).
# Since the cosine shape has no new tunable, we just compare LINEAR vs COSINE
# at the same endpoints and on three instances. Also test wider endpoints
# that might benefit from a smoother curve.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_cos
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500; SAMP=250
INSTS="ft10_10 ft20_05 la25_04"

# tag : sched_type : rot_start rot_end floor_start floor_end
CONFIGS=(
  "lin_baseline:linear:0.02 0.08 0.02 0.005"   # baseline (current)
  "cos_baseline:cosine:0.02 0.08 0.02 0.005"   # cosine at same endpoints
  "cos_wider:cosine:0.01 0.10 0.05 0.002"      # wider endpoints, cosine
  "cos_rot_only:cosine:0.02 0.08 0.005 0.005"  # cosine only on rotation
  "cos_floor_only:cosine:0.04 0.04 0.02 0.005" # cosine only on floor (constant rot)
)

for cfg in "${CONFIGS[@]}"; do
  IFS=':' read -r tag sched params <<< "$cfg"
  read rs re fs fe <<< "$params"
  for inst in $INSTS; do
    d="$OUT/${tag}_${inst}"
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
qea.samples = $SAMP
qea.rotation = 0.04
qea.rot_start = $rs
qea.rot_end = $re
qea.floor = 0.01
qea.floor_start = $fs
qea.floor_end = $fe
qea.schedule_type = $sched
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
echo SWEEP_COS_DONE
