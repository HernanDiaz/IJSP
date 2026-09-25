#!/bin/bash
# I-064: the hybrid on ta18 (20x15, the only open instance of its class, never
# polished) and ta50 (the last 30x20). Fixed before running: ta18, its 20
# lowest distinct feasible schedules, 30 s each; ta50, 6 lowest, 60 s each
# (I-042 model, 14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-064
LOG=$I/probe_run.log
echo "=== I-064 probe start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
while read inst k secs; do
  python3 iter/I-043/top_schedules.py $inst "results/I-0*/${inst}_*_Certificate.csv" $k > $I/hints_$inst.txt
  n=0
  while read cert run ms; do
    n=$((n+1))
    echo "--- $inst hint $n: $cert run $run makespan $ms" >> $LOG
    python3 iter/I-042/cpsat_probe.py $inst $cert $run $secs >> $LOG 2>&1
    collect $inst hint$n
  done < $I/hints_$inst.txt
done <<LIST
ta18 20 30
ta50 6 60
LIST
echo "=== I-064 probe exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*found_(ta[0-9]+)_.*/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "^ta[0-9]+: status|INFEAS|anomal" $LOG
