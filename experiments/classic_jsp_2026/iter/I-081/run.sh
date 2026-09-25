#!/bin/bash
# I-081: ta27, the three machine pairs with the lowest bounds in I-080
# ((15,17) 1673, (0,15) 1676, (0,17) 1676, all below the best known 1680),
# 400 s each; I-079's group.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-081
G=iter/I-079/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-081 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
while read set; do python3 iter/I-068/break_machines.py ta27 $G $K 400 $set >> $LOG 2>&1; done < $I/sets.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-081 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
