#!/bin/bash
# I-071: the 12 ta18 machine pairs I-069/I-070 could not close in 45 s, now
# 120 s each (same consensus, same script).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-071
LOG=$I/run.log
echo "=== I-071 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
K=$(wc -l < iter/I-068/group_ta18.txt)
while read pair; do
  python3 iter/I-068/break_machines.py ta18 iter/I-068/group_ta18.txt $K 120 $pair >> $LOG 2>&1
done < $I/open_pairs.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-071 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
