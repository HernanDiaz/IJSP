#!/bin/bash
# ITER 1 (fresh start): linear rotation schedule on top of the SIMPLE positional
# baseline (commit d8f1d66: samples 250, rotation 0.04, floor 0.01, no schedules).
# The schedule adds ~zero cost/gen, so we reuse the bare-baseline G (run_base.sh)
# -> clean comparison at the SAME generation budget (<= 4x GA). Sweep 3 instances,
# runs=10 (deterministic under fixed G/seed=1). Variant "c0404" == baseline.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/iter1_rotsched_sweep
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=10

# inst:LB:Gfixed(bare baseline, run_base.sh)
INSTS="ft10_10:930:194 ft20_05:1165:214 la25_04:977:222"

# variant -> "rot_start rot_end"
vparams() {
  case "$1" in
    c0404) echo "0.04 0.04";;   # constant == baseline (reference)
    s0406) echo "0.04 0.06";;
    s0408) echo "0.04 0.08";;
    s0410) echo "0.04 0.10";;
    s0412) echo "0.04 0.12";;
    s0208) echo "0.02 0.08";;
    s0610) echo "0.06 0.10";;
  esac
}
VARIANTS="c0404 s0406 s0408 s0410 s0412 s0208 s0610"

emit_setup() {  # $1 generations $2 runs $3 rot_start $4 rot_end
  cat <<EOF
algorithm = QEA
seed = 1
runs = $2
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
qea.rotation = 0.04
qea.rot_start = $3
qea.rot_end = $4
qea.floor = 0.01
generations = $1
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = iteration
evolution.span = 100
EOF
}
read_field() { grep -m1 "^$2;" "$1" | tr -d '\r' | cut -d';' -f"$3"; }

for variant in $VARIANTS; do
  P=$(vparams "$variant")
  for entry in $INSTS; do
    inst=${entry%%:*}; g=$(echo "$entry" | cut -d: -f3)
    d="$OUT/${variant}_${inst}"; mkdir -p "$d"
    emit_setup "$g" "$RUNS" $P > "$d/run.txt"
    ( ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1 ) &
  done
done
wait
echo SWEEP_DONE

for variant in $VARIANTS; do
  sumre=0; n=0
  for entry in $INSTS; do
    inst=${entry%%:*}; lb=$(echo "$entry" | cut -d: -f2)
    d="$OUT/${variant}_${inst}"
    runcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(read_field "$runcsv" "Best solution" 2)
    re=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sumre=$(awk -v s="$sumre" -v r="$re" 'BEGIN{print s+r}')
    n=$((n+1))
  done
  awk -v v="$variant" -v s="$sumre" -v n="$n" 'BEGIN{printf "%-7s meanAvgRE%%(3inst,r10)=%6.2f\n", v, s/n}'
done
