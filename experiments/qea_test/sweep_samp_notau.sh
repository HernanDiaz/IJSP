#!/bin/bash
# T2: on top of notau (best under time, 13.08%), re-tune the samples schedule.
# Without tau, sampling cost dominates; samples_end=400 makes late generations
# 4x as expensive. Test smaller samples_end (more generations) vs the benefit of
# richer sampling. Calibrate G per (variant,instance) single-process to <=4x GA,
# then run and read AvgRE. Sweep on 3 instances.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_samp_notau
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS_CAL=3
RUNS=8

# inst:LB:4xGA
INSTS="ft10_10:930:2.0 ft20_05:1165:2.8 la25_04:977:4.0"

# variant -> "samples_start samples_end"   (everything else = notau)
vparams() {
  case "$1" in
    s100_400) echo "100 400";;   # = notau base
    s100_300) echo "100 300";;
    s100_250) echo "100 250";;
    s100_200) echo "100 200";;
    s250c)    echo "250 250";;
    s50_250)  echo "50 250";;
  esac
}

emit_setup() {  # $1 stopkey $2 stopval $3 runs $4 ss $5 se
  local stopkey=$1 stopval=$2 runs=$3 ss=$4 se=$5
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
qea.samples_start = $ss
qea.samples_end = $se
qea.rotation = 0.04
qea.rot_start = 0.02
qea.rot_end = 0.08
qea.floor = 0.005
qea.floor_start = 0.005
qea.floor_end = 0.005
qea.schedule_type = cosine
qea.tau_start = 1.0
qea.tau_end = 1.0
qea.target_w_start = 0.0
qea.target_w_end = 1.0
qea.anti_step_start = 0.005
qea.anti_step_end = 0.02
$stopkey = $stopval
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = iteration
evolution.span = 100
EOF
}
read_field() { grep -m1 "^$2;" "$1" | tr -d '\r' | cut -d';' -f"$3"; }

for variant in s100_400 s100_300 s100_250 s100_200 s250c s50_250; do
  P=$(vparams "$variant")
  sumre=0; n=0
  for entry in $INSTS; do
    inst=${entry%%:*}; lb=$(echo "$entry" | cut -d: -f2); lim=$(echo "$entry" | cut -d: -f3)
    d="$OUT/${variant}_${inst}"; mkdir -p "$d"
    # calibrate single-process
    emit_setup timelimit "$lim" "$RUNS_CAL" $P > "$d/cal.txt"
    ../FuzzyFW "$d/cal.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/cal.log" 2>&1
    calcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    G=$(read_field "$calcsv" "Number of Generations" 2 | awk '{printf "%d",$1}')
    rm -f "$d"/*.csv
    # run generations=G
    emit_setup generations "$G" "$RUNS" $P > "$d/run.txt"
    ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1
    runcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(read_field "$runcsv" "Best solution" 2)
    re=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sumre=$(awk -v s="$sumre" -v r="$re" 'BEGIN{print s+r}')
    n=$((n+1))
    echo "  $variant $inst G=$G AvgRE=$re" >> "$OUT/log.txt"
  done
  awk -v v="$variant" -v s="$sumre" -v n="$n" 'BEGIN{printf "%-10s meanAvgRE%%=%6.2f\n", v, s/n}'
done
echo SWEEP_SAMP_NOTAU_DONE
