#!/bin/bash
# Sweep for floor schedule: floor_start x floor_end (over the current baseline:
# positional + rotation schedule 0.02 -> 0.08, floor 0.01 constant).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_floorsched
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500; SAMP=250; ROT_S=0.02; ROT_E=0.08
INSTS="ft10_10 ft20_05 la25_04"

# Floor schedule pairs (start, end). Baseline included for sanity (0.01, 0.01).
PAIRS=(
  "0.01 0.01"   # baseline (constant)
  "0.05 0.005"  # explore strong -> exploit
  "0.05 0.01"   # explore strong -> baseline
  "0.02 0.005"  # explore mild   -> exploit
  "0.02 0.01"   # explore mild   -> baseline
  "0.005 0.02"  # exploit -> explore (reverse)
  "0.005 0.05"  # exploit -> explore (reverse, strong)
)

for pair in "${PAIRS[@]}"; do
  read fs fe <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/s${fs}_e${fe}_${inst}"
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
qea.rot_start = $ROT_S
qea.rot_end = $ROT_E
qea.floor = 0.01
qea.floor_start = $fs
qea.floor_end = $fe
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
echo SWEEP_FLOORSCHED_DONE
