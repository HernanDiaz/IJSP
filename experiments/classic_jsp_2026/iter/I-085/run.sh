#!/bin/bash
# I-085: round 3 of the ta27 descent, from 1680 (= best known). The I-083 group
# plus the 1680 schedule of I-084; each of the 20 machines freed, 60 s. Any
# schedule below 1680 is a record and goes to verify_certificate.py.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-085
G=$I/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-085 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
for m in $(seq 0 19); do python3 iter/I-068/break_machines.py ta27 $G $K 60 $m >> $LOG 2>&1; done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-085 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of 20"
grep -v "OPTIMAL, bound 1680" $LOG | grep -v "^==="
