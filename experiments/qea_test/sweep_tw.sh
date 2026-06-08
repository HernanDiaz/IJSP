#!/bin/bash
# T4: re-tune target_w on the T3 base (notau + rot_start 0.04). target_w does not
# change cost/gen -> reuse FIXED notau G (clean comparison). Parallel, runs=10.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_tw
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=10

# inst:LB:Gfixed(notau)
INSTS="ft10_10:930:320 ft20_05:1165:352 la25_04:977:374"

# variant -> "tw_start tw_end"
vparams() {
  case "$1" in
    tw0_1)  echo "0.0 1.0";;   # = base
    tw1c)   echo "1.0 1.0";;   # always bestSoFar
    tw0c)   echo "0.0 0.0";;   # always genBest
    tw0_05) echo "0.0 0.5";;
    tw05_1) echo "0.5 1.0";;
    tw0_08) echo "0.0 0.8";;
  esac
}

emit_setup() {  # $1 generations $2 runs $3 tws $4 twe
  local g=$1 runs=$2 tws=$3 twe=$4
  cat <<EOF
algorithm = QEA
seed = 1
runs = $runs
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
qea.rot_start = 0.04
qea.rot_end = 0.08
qea.floor = 0.005
qea.floor_start = 0.005
qea.floor_end = 0.005
qea.schedule_type = cosine
qea.tau_start = 1.0
qea.tau_end = 1.0
qea.target_w_start = $tws
qea.target_w_end = $twe
qea.anti_step_start = 0.005
qea.anti_step_end = 0.02
generations = $g
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = iteration
evolution.span = 100
EOF
}
read_field() { grep -m1 "^$2;" "$1" | tr -d '\r' | cut -d';' -f"$3"; }

for variant in tw0_1 tw1c tw0c tw0_05 tw05_1 tw0_08; do
  P=$(vparams "$variant")
  for entry in $INSTS; do
    inst=${entry%%:*}; g=$(echo "$entry" | cut -d: -f3)
    d="$OUT/${variant}_${inst}"; mkdir -p "$d"
    emit_setup "$g" "$RUNS" $P > "$d/run.txt"
    ( ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1 ) &
  done
done
wait
echo SWEEP_TW_DONE

for variant in tw0_1 tw1c tw0c tw0_05 tw05_1 tw0_08; do
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
  awk -v v="$variant" -v s="$sumre" -v n="$n" 'BEGIN{printf "%-8s meanAvgRE%%=%6.2f\n", v, s/n}'
done
