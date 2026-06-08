#!/bin/bash
# T5: exhaust the remaining cheap levers (anti-step, floor) on the T3 base
# (notau + rot_start 0.04). Fixed notau G, parallel, runs=10.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_misc
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=10
INSTS="ft10_10:930:320 ft20_05:1165:352 la25_04:977:374"

# variant -> "anti_start anti_end floor"
vparams() {
  case "$1" in
    base)       echo "0.005 0.02  0.005";;
    anti0)      echo "0.0   0.0   0.005";;
    anti005_04) echo "0.005 0.04  0.005";;
    anti01_03)  echo "0.01  0.03  0.005";;
    floor002)   echo "0.005 0.02  0.002";;
    floor0075)  echo "0.005 0.02  0.0075";;
    floor01)    echo "0.005 0.02  0.01";;
  esac
}

emit_setup() {  # $1 gens $2 runs $3 as $4 ae $5 floor
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
qea.samples_start = 100
qea.samples_end = 400
qea.rotation = 0.04
qea.rot_start = 0.04
qea.rot_end = 0.08
qea.floor = $5
qea.floor_start = $5
qea.floor_end = $5
qea.schedule_type = cosine
qea.tau_start = 1.0
qea.tau_end = 1.0
qea.target_w_start = 0.0
qea.target_w_end = 1.0
qea.anti_step_start = $3
qea.anti_step_end = $4
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

VARIANTS="base anti0 anti005_04 anti01_03 floor002 floor0075 floor01"
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
echo SWEEP_MISC_DONE

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
  awk -v v="$variant" -v s="$sumre" -v n="$n" 'BEGIN{printf "%-11s meanAvgRE%%=%6.2f\n", v, s/n}'
done
