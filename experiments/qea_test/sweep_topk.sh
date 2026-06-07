#!/bin/bash
# Sweep for top-K truncation schedule on the tau baseline.
# Instances: ft10_10 (10 jobs), ft20_05 (20 jobs), la25_04 (15 jobs).
# We test combinations where topK truncates to subsets of varying sizes.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_topk
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500
INSTS="ft10_10 ft20_05 la25_04"

# (topk_start, topk_end). 0/0 = no truncation = baseline.
# We try various "narrowing" patterns and one widening.
PAIRS=(
  "0 0"     # baseline (no truncation)
  "20 5"    # wide -> narrow (good for nJobs <= 20)
  "20 7"    # wide -> moderate
  "20 10"   # wide -> half
  "15 5"    # moderate -> narrow
  "15 7"    # moderate -> moderate (always cap)
  "5 20"    # narrow -> wide (reverse direction)
)

for pair in "${PAIRS[@]}"; do
  read ks ke <<< "$pair"
  for inst in $INSTS; do
    d="$OUT/ks${ks}_ke${ke}_${inst}"
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
qea.topk_start = $ks
qea.topk_end = $ke
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
echo SWEEP_TOPK_DONE
