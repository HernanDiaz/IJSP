#!/bin/bash
# I-080: ta27, the two machines I-079 could not close (0 and 15) at 180 s, then
# the 37 machine pairs that contain 0 or 15 at 40 s each; I-079's group.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-080
G=iter/I-079/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-080 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
while read set; do python3 iter/I-068/break_machines.py ta27 $G $K 180 $set >> $LOG 2>&1; done < $I/sets_singles.txt
while read set; do python3 iter/I-068/break_machines.py ta27 $G $K 40 $set >> $LOG 2>&1; done < $I/sets_pairs.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-080 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of 39"
grep -v "OPTIMAL, bound 1685" $LOG | grep -v "^==="
