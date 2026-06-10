#!/bin/bash
# ITER 1 isolation: is the rotation-schedule gain from the SCHEDULE gradient, or
# merely from a higher MEAN rotation? Compare constants {0.06,0.08,0.10} against
# the two best schedules (mean rotation ~0.08) on the same 3 instances, runs=10,
# bare-baseline G. If a constant matches the schedule, the gradient adds nothing.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/iter1_rotsched_isolate
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=10
INSTS="ft10_10:930:194 ft20_05:1165:214 la25_04:977:222"

vparams() {
  case "$1" in
    c0404) echo "0.04 0.04";;   # baseline
    c0606) echo "0.06 0.06";;
    c0808) echo "0.08 0.08";;   # constant at the schedule mean
    c1010) echo "0.10 0.10";;
    s0610) echo "0.06 0.10";;   # best schedule (mean 0.08)
    s0412) echo "0.04 0.12";;   # 2nd schedule (mean 0.08)
  esac
}
VARIANTS="c0404 c0606 c0808 c1010 s0610 s0412"

emit_setup() {
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
echo ISOLATE_DONE

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
