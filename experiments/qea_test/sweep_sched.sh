#!/bin/bash
# Sweep for rotation schedule: rot_start x rot_end (positional, floor=0.01).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_sched
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500; SAMP=250; FLOOR=0.01
INSTS="ft10_10 ft20_05 la25_04"

# Each (start, end) pair tested. Includes baseline (0.04, 0.04) as sanity check.
PAIRS=(
  "0.04 0.04"   # constant baseline
  "0.01 0.10"   # exploration -> exploitation
  "0.02 0.08"   # mild exploration -> exploitation
  "0.10 0.01"   # exploitation -> refinement
  "0.08 0.02"   # mild exploitation -> refinement
  "0.01 0.04"   # warm-up
  "0.04 0.01"   # cool-down
)

for pair in "${PAIRS[@]}"; do
  read rs re <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/s${rs}_e${re}_${inst}"
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
qea.floor = $FLOOR
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
echo SWEEP_SCHED_DONE
