#!/bin/bash
# ITER 1 full validation (12 inst x 30 runs, bare-baseline G <= 4x GA). Runs TWO
# configs head-to-head so we can isolate the rotation SCHEDULE from a higher
# CONSTANT rotation:
#   sched_s0610  -> rot_start 0.06, rot_end 0.10  (linear schedule, mean 0.08)
#   const_c1010  -> rot 0.10 constant             (best constant from isolation)
# Baseline reference = run30_base (rotation 0.04 constant) -> AvgRE 16.54%.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT=experiments/qea_test/iter1_rotsched_full
rm -rf "$OUT"; mkdir -p "$OUT"
RUNS=30

# inst:LB:Gfixed(bare baseline, run_base.sh)
ENTRIES="abz7_06:656:175 abz8_05:645:173 abz9_10:661:210 ft10_10:930:194 \
ft20_05:1165:214 la21_04:1046:255 la24_03:935:182 la25_04:977:222 \
la27_09:1235:195 la29_03:1152:155 la38_06:1196:198 la40_05:1222:172"

# config -> "rot_start rot_end"
cparams() {
  case "$1" in
    sched_s0610) echo "0.06 0.10";;
    const_c1010) echo "0.10 0.10";;
  esac
}
CONFIGS="sched_s0610 const_c1010"

emit_setup() {  # $1 generations $2 rot_start $3 rot_end
  cat <<EOF
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
qea.rotation = 0.04
qea.rot_start = $2
qea.rot_end = $3
qea.floor = 0.01
generations = $1
statistics.1.metric = best
statistics.1.value = ijsp.makespan
statistics.2.metric = average
statistics.2.value = ijsp.makespan
evolution.unit = iteration
evolution.span = 50
EOF
}
read_field() { grep -m1 "^$2;" "$1" | tr -d '\r' | cut -d';' -f"$3"; }

for cfg in $CONFIGS; do
  P=$(cparams "$cfg")
  for entry in $ENTRIES; do
    inst=${entry%%:*}; g=$(echo "$entry" | cut -d: -f3)
    d="$OUT/${cfg}/${inst}"; mkdir -p "$d"
    emit_setup "$g" $P > "$d/run.txt"
    ( ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1 ) &
  done
done
wait
echo FULL_DONE

for cfg in $CONFIGS; do
  echo "===== $cfg ====="
  for entry in $ENTRIES; do
    inst=${entry%%:*}; lb=$(echo "$entry" | cut -d: -f2)
    d="$OUT/${cfg}/${inst}"
    runcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
    avg=$(read_field "$runcsv" "Best solution" 2)
    best=$(read_field "$runcsv" "Best solution" 3)
    echo "$inst $lb $best $avg"
  done | awk '
    BEGIN{ printf "%-9s %5s %9s %9s %8s %8s\n","Inst","LB","BestMid","AvgMid","BestRE%","AvgRE%" }
    { lb=$2; best=$3; avg=$4; bre=(best-lb)/lb*100; are=(avg-lb)/lb*100; sb+=bre; sa+=are; n++;
      printf "%-9s %5d %9.1f %9.1f %7.2f %7.2f\n",$1,lb,best,avg,bre,are }
    END{ printf "%-9s %5s %9s %9s %7.2f %7.2f\n","MEAN","","","",sb/n,sa/n }'
done
