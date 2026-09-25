#!/bin/bash
# I-077: the 7 open machine pairs of round 3 (group with the 1401), 180 s each.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-077
G=iter/I-074/group_ta18.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-077 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
while read set; do python3 iter/I-068/break_machines.py ta18 $G $K 180 $set >> $LOG 2>&1; done < $I/sets_open.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-077 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
