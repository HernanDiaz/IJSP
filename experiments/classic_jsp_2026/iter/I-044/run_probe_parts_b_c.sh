#!/bin/bash
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-044
LOG=$I/probe_run.log
echo "=== I-044 parts b and c restart at $(date): the hint selector skipped no malformed certificate rows before ===" >> $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
for inst in ta29 ta30; do
  python3 iter/I-043/top_schedules.py $inst "results/I-0*/${inst}_*_Certificate.csv" 10 > $I/hints_$inst.txt
  cat $I/hints_$inst.txt >> $LOG
  k=0
  while read cert run ms; do
    k=$((k+1))
    echo "--- $inst hint $k: $cert run $run makespan $ms" >> $LOG
    python3 iter/I-042/cpsat_probe.py $inst $cert $run 60 >> $LOG 2>&1
    collect $inst hint$k
  done < $I/hints_$inst.txt
done
echo "=== I-044 probe exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*found_(ta[0-9]+)_.*/\1/')
  echo "verify $f" >> $LOG
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "^--- |status|RECORD|verified" $LOG
