#!/bin/bash
# I-072: round 1 of the core-breaking descent on ta18. The I-066 group plus the
# 1404 schedule of I-071 (the group's best and the hint); each of the 15
# machines freed on its own, 60 s of CP-SAT each.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-072
LOG=$I/run.log
echo "=== I-072 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
{ echo "iter/I-071/found_m5-8_ta18_1404_Certificate.csv 1 1404"; cat iter/I-066/group_ta18.txt; } > $I/group_ta18.txt
K=$(wc -l < $I/group_ta18.txt)
echo "group: $K schedules, best $(head -1 $I/group_ta18.txt)" >> $LOG
for m in $(seq 0 14); do python3 iter/I-068/break_machines.py ta18 $I/group_ta18.txt $K 60 $m >> $LOG 2>&1; done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-072 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
