#!/bin/bash
# Full 12x30 validation of a variant under the time budget.
#   $1 = variant name (full|nosamp|noanti|nosamp_noanti|notau)
# Step 1: calibrate G per instance single-process (timelimit=4xGA).
# Step 2: run generations=G, 12x30 (parallel).
# Step 3: print AvgRE per instance + mean.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
VARIANT="${1:-notau}"
OUT="experiments/qea_test/reval_${VARIANT}"
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS_CAL=5
RUNS=30

# inst:LB:4xGA
ENTRIES="abz7_06:656:7.2 abz8_05:645:7.2 abz9_10:661:8.8 ft10_10:930:2.0 \
ft20_05:1165:2.8 la21_04:1046:4.4 la24_03:935:3.2 la25_04:977:4.0 \
la27_09:1235:5.2 la29_03:1152:4.4 la38_06:1196:5.6 la40_05:1222:4.8"

vparams() {
  case "$1" in
    full)          echo "100 400 0.7 1.3 0.005 0.02";;
    nosamp)        echo "250 250 0.7 1.3 0.005 0.02";;
    noanti)        echo "100 400 0.7 1.3 0.0   0.0";;
    nosamp_noanti) echo "250 250 0.7 1.3 0.0   0.0";;
    notau)         echo "100 400 1.0 1.0 0.005 0.02";;
  esac
}

emit_setup() {  # $1 stopkey $2 stopval $3 runs $4..$9 params
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
evolution.span = 50
EOF
}
read_field() { grep -m1 "^$2;" "$1" | tr -d '\r' | cut -d';' -f"$3"; }
P=$(vparams "$VARIANT")

# Step 1: calibrate G per instance, single-process (sequential)
echo "Calibrating ($VARIANT)..."
for entry in $ENTRIES; do
  inst=${entry%%:*}; lim=$(echo "$entry" | cut -d: -f3)
  d="$OUT/$inst"; mkdir -p "$d"
  emit_setup timelimit "$lim" "$RUNS_CAL" $P > "$d/cal.txt"
  ../FuzzyFW "$d/cal.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/cal.log" 2>&1
  calcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  G=$(read_field "$calcsv" "Number of Generations" 2 | awk '{printf "%d",$1}')
  echo "$G" > "$d/G.txt"
  rm -f "$d"/*.csv
done

# Step 2: run generations=G, 12x30 (parallel)
echo "Running 12x30 ($VARIANT)..."
for entry in $ENTRIES; do
  inst=${entry%%:*}
  d="$OUT/$inst"; G=$(cat "$d/G.txt")
  emit_setup generations "$G" "$RUNS" $P > "$d/run.txt"
  ( ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1 ) &
done
wait

# Step 3: parse
echo "VARIANT=$VARIANT"
printf "%-9s %5s %6s %8s\n" Inst LB G AvgRE%
for entry in $ENTRIES; do
  inst=${entry%%:*}; lb=$(echo "$entry" | cut -d: -f2)
  d="$OUT/$inst"; G=$(cat "$d/G.txt")
  runcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  avg=$(read_field "$runcsv" "Best solution" 2)
  echo "$inst $lb $G $avg"
done | awk '
BEGIN{ printf "%-9s %5s %6s %8s\n","Inst","LB","G","AvgRE%" }
{ inst=$1; lb=$2; g=$3; avg=$4; are=(avg-lb)/lb*100; s+=are; n++;
  printf "%-9s %5d %6d %8.2f\n",inst,lb,g,are }
END{ printf "%-9s %5s %6s %8.2f\n","MEAN","","",s/n }'
echo REVAL_FULL_DONE
