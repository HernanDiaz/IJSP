#!/bin/bash
# I-047: close the loop. The ABC seeded with the schedules CP-SAT left in
# I-045 and I-046, then CP-SAT on the ABC's results.
# Fixed before running: ta25 and ta27; ref_I-021 with creation = jsp.seeded,
# 25 seeds of 247 from the pool, run p takes pool lines from offset p; 28 runs
# per instance, seeds 7101-7128, 40 s each (the class budget); then the 20
# lowest distinct schedules of those runs get 20 s of CP-SAT each.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-047
A=/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026/$I
LOG=$I/run.log
echo "=== I-047 start at $(date) ===" > $LOG
mkdir -p $I/setups
: > $I/jobs.tsv
for inst in ta25 ta27; do
  for p in $(seq 1 28); do
    pp=$(printf "%02d" $p)
    f=$A/setups/${inst}_seeded_p$pp.txt
    sed -e "s/^creation = jsp.random$/creation = jsp.seeded/" -e "s/^runs = .*/runs = 1/" \
        -e "s/^seed = .*/seed = $((7100 + p))/" -e "s/^timelimit = .*/timelimit = 40/" setup/ref_I-021.txt > $f
    printf "\n# I-047: seeded from the CP-SAT schedules of I-045 and I-046\ncreation.seed.pool = $A/pool_$inst.csv\ncreation.seed.count = 25\ncreation.seed.offset = $p\n" >> $f
    printf "%s\tI-047_p%s_seeded\t%s\n" $f $pp $inst >> $I/jobs.tsv
  done
done
echo "setups: $(grep -l '^creation = jsp.seeded$' $I/setups/*.txt | wc -l) seeded of $(ls $I/setups/*.txt | wc -l)" >> $LOG
SLOTS=14 bash scripts/queue_jobs.sh $I/jobs.tsv >> $LOG 2>&1
echo "=== ABC part done at $(date), infeasible reports: $(grep -c INFEASIBLE $LOG) ===" >> $LOG
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
for inst in ta25 ta27; do
  python3 iter/I-043/top_schedules.py $inst "results/I-047_*/${inst}_*_Certificate.csv" 20 > $I/hints_$inst.txt
  echo "--- $inst ABC best: $(head -1 $I/hints_$inst.txt)" >> $LOG
  k=0
  while read cert run ms; do
    k=$((k+1))
    echo "--- $inst hint $k: $cert run $run makespan $ms" >> $LOG
    python3 iter/I-042/cpsat_probe.py $inst $cert $run 20 >> $LOG 2>&1
    collect $inst hint$k
  done < $I/hints_$inst.txt
done
echo "=== I-047 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*found_(ta[0-9]+)_.*/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "ABC best|^ta2[57]: status|INFEAS|anomal" $LOG
