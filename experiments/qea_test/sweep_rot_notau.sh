#!/bin/bash
# T3: re-tune the rotation schedule on top of notau, using the FIXED notau G per
# instance (no recalibration -> no calibration noise; rotation does not change
# cost/gen, so time stays ~4x GA). Parallel + runs=10 (AvgRE is deterministic
# under fixed G/seed, independent of contention).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/sweep_rot_notau
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=10

# inst:LB:Gfixed(notau, single-process calibration)
INSTS="ft10_10:930:320 ft20_05:1165:352 la25_04:977:374"

# variant -> "rot_start rot_end"
vparams() {
  case "$1" in
    r02_08) echo "0.02 0.08";;   # = notau base
    r02_12) echo "0.02 0.12";;
    r02_10) echo "0.02 0.10";;
    r02_06) echo "0.02 0.06";;
    r04_08) echo "0.04 0.08";;
    r04c)   echo "0.04 0.04";;
  esac
}

emit_setup() {  # $1 generations $2 runs $3 rs $4 re
  local g=$1 runs=$2 rs=$3 re=$4
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
qea.rot_start = $rs
qea.rot_end = $re
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

# Launch all (variant x instance) in parallel
for variant in r02_08 r02_12 r02_10 r02_06 r04_08 r04c; do
  P=$(vparams "$variant")
  for entry in $INSTS; do
    inst=${entry%%:*}; g=$(echo "$entry" | cut -d: -f3)
    d="$OUT/${variant}_${inst}"; mkdir -p "$d"
    emit_setup "$g" "$RUNS" $P > "$d/run.txt"
    ( ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1 ) &
  done
done
wait
echo SWEEP_ROT_NOTAU_DONE

for variant in r02_08 r02_12 r02_10 r02_06 r04_08 r04c; do
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
