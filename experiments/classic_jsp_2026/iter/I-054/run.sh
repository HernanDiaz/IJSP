#!/bin/bash
# I-054: exact recombination between attractors. For ta25 and ta27, the floor
# each CP-SAT hint of I-045/I-046/I-053 reached (floors.py); the lowest floor
# is recombined with each of the next 15, agreeing machine pairs fixed, the
# rest free, 45 s of CP-SAT each (14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-054
LOG=$I/run.log
echo "=== I-054 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
for inst in ta25 ta27; do
  python3 $I/floors.py $inst > $I/floors_$inst.txt
  echo "--- $inst: $(wc -l < $I/floors_$inst.txt) floors, $(cut -d' ' -f3 $I/floors_$inst.txt | head -16 | tr '\n' ' ')" >> $LOG
  read ca ra ma < <(head -1 $I/floors_$inst.txt)
  k=0
  sed -n '2,16p' $I/floors_$inst.txt | while read cb rb mb; do
    k=$((k+1))
    python3 $I/recombine.py $inst $ca $ra $cb $rb 45 pair$k >> $LOG 2>&1
  done
done
echo "=== I-054 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*_(ta[0-9]+)_[0-9]+_Certificate.csv/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
