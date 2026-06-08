#!/bin/bash
# Re-validate, UNDER THE TIME BUDGET, which subset of iter-17 schedules is best.
# For each variant: calibrate G per instance (single-process timelimit=4xGA),
# then run generations=G and read AvgRE. Sweep on 3 instances.
# Variants toggle the EXPENSIVE schedules (samples schedule, anti-rotation, tau).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/reval_sweep
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS_CAL=3
RUNS=8

# inst:LB:4xGA
INSTS="ft10_10:930:2.0 ft20_05:1165:2.8 la25_04:977:4.0"

# variant -> "SS SE TS TE AS AE"  (samples start/end, tau start/end, anti start/end)
vparams() {
  case "$1" in
    full)          echo "100 400 0.7 1.3 0.005 0.02";;
    nosamp)        echo "250 250 0.7 1.3 0.005 0.02";;
    noanti)        echo "100 400 0.7 1.3 0.0   0.0";;
    nosamp_noanti) echo "250 250 0.7 1.3 0.0   0.0";;
    notau)         echo "100 400 1.0 1.0 0.005 0.02";;
  esac
}

emit_setup() {  # $1 stopkey(generations|timelimit) $2 stopval $3 runs $4..$9 params
  local stopkey=$1 stopval=$2 runs=$3 ss=$4 se=$5 ts=$6 te=$7 as=$8 ae=$9
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
qea.tau_start = $ts
qea.tau_end = $te
qea.target_w_start = 0.0
qea.target_w_end = 1.0
qea.anti_step_start = $as
qea.anti_step_end = $ae
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

for variant in full nosamp noanti nosamp_noanti notau; do
  P=$(vparams "$variant")
  sumre=0; n=0
  for entry in $INSTS; do
    inst=${entry%%:*}; lb=$(echo "$entry" | cut -d: -f2); lim=$(echo "$entry" | cut -d: -f3)
    d="$OUT/${variant}_${inst}"; mkdir -p "$d"
    # 1) calibrate single-process (sequential), timelimit=4xGA
    emit_setup timelimit "$lim" "$RUNS_CAL" $P > "$d/cal.txt"
    ../FuzzyFW "$d/cal.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/cal.log" 2>&1
    calcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    G=$(read_field "$calcsv" "Number of Generations" 2 | awk '{printf "%d",$1}')
    rm -f "$d"/*.csv
    # 2) run generations=G
    emit_setup generations "$G" "$RUNS" $P > "$d/run.txt"
    ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1
    runcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(read_field "$runcsv" "Best solution" 2)
    re=$(awk -v a="$avg" -v l="$lb" 'BEGIN{print (a-l)/l*100}')
    sumre=$(awk -v s="$sumre" -v r="$re" 'BEGIN{print s+r}')
    n=$((n+1))
    echo "  $variant $inst G=$G AvgRE=$re" >> "$OUT/log.txt"
  done
  awk -v v="$variant" -v s="$sumre" -v n="$n" 'BEGIN{printf "%-15s meanAvgRE%%=%6.2f\n", v, s/n}'
done
echo REVAL_SWEEP_DONE
