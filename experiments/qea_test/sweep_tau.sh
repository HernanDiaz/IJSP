#!/bin/bash
# Sweep for sampling temperature schedule: tau_start x tau_end on top of the
# current baseline (cosine rot + floor const + samples 100->400).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_tau
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500
INSTS="ft10_10 ft20_05 la25_04"

PAIRS=(
  "1.0 1.0"   # baseline (off)
  "0.5 1.5"   # explore -> exploit (in sampling)
  "0.5 2.0"   # stronger explore -> exploit
  "1.0 2.0"   # baseline at start, sharpen at end
  "1.0 0.5"   # baseline -> more exploration (inverse)
  "0.7 1.3"   # mild explore -> exploit
  "1.5 0.5"   # exploit -> explore (inverse, strong)
)

for pair in "${PAIRS[@]}"; do
  read ts te <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/ts${ts}_te${te}_${inst}"
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
qea.tau_start = $ts
qea.tau_end = $te
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
echo SWEEP_TAU_DONE
