#!/bin/bash
# Parameter sweep for the positional QEA: rotation x floor on 3 fast instances.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=8; GEN=500; SAMP=250
INSTS="ft10_10 ft20_05 la25_04"

for rot in 0.02 0.04 0.08 0.16; do
  for flr in 0.0 0.01 0.05; do
    for inst in $INSTS; do
      d="$OUT/r${rot}_f${flr}_${inst}"
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
qea.rotation = $rot
qea.floor = $flr
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
done
wait
echo SWEEP_DONE
