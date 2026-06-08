#!/bin/bash
# Full 12x30 with the FIXED notau G per instance (rotation does not change
# cost/gen, so reusing notau's G keeps runtime ~4x GA). Config = notau + the
# rotation tweak under test (rot_start 0.04). Compare against notau 13.08%.
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP
OUT="${1:-experiments/qea_test/reval_r04_08}"
RUNS=30
RS="${2:-0.04}"   # rot_start
RE="${3:-0.08}"   # rot_end

# inst:LB:Gfixed(notau)
ENTRIES="abz7_06:656:292 abz8_05:645:297 abz9_10:661:335 ft10_10:930:320 \
ft20_05:1165:352 la21_04:1046:403 la24_03:935:324 la25_04:977:374 \
la27_09:1235:324 la29_03:1152:249 la38_06:1196:335 la40_05:1222:289"

emit_setup() {  # $1 generations $2 rs $3 re
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
qea.samples_start = 100
qea.samples_end = 400
qea.rotation = 0.04
qea.rot_start = $2
qea.rot_end = $3
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

rm -rf "$OUT"; mkdir -p "$OUT"
for entry in $ENTRIES; do
  inst=${entry%%:*}; g=$(echo "$entry" | cut -d: -f3)
  d="$OUT/$inst"; mkdir -p "$d"
  emit_setup "$g" "$RS" "$RE" > "$d/run.txt"
  ( ../FuzzyFW "$d/run.txt" "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" "$d" > "$d/run.log" 2>&1 ) &
done
wait
echo RUN_FIXEDG_DONE

printf "%-9s %5s %6s %8s\n" Inst LB G AvgRE%
for entry in $ENTRIES; do
  inst=${entry%%:*}; lb=$(echo "$entry" | cut -d: -f2); g=$(echo "$entry" | cut -d: -f3)
  d="$OUT/$inst"
  runcsv=$(ls "$d"/*.csv 2>/dev/null | grep -vE "_Sols|_Robustness|_Scenarios" | head -1)
  avg=$(read_field "$runcsv" "Best solution" 2)
  echo "$inst $lb $g $avg"
done | awk '
BEGIN{ printf "%-9s %5s %6s %8s\n","Inst","LB","G","AvgRE%" }
{ are=($4-$2)/$2*100; s+=are; n++; printf "%-9s %5d %6d %8.2f\n",$1,$2,$3,are }
END{ printf "%-9s %5s %6s %8.2f\n","MEAN","","",s/n }'
