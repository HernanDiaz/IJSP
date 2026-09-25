#!/bin/bash
# I-078: round 4 of the descent from 1401: 30 machine triples drawn at random
# (random.Random(1) over the 455 triples, sets.txt), 50 s each.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-078
G=iter/I-074/group_ta18.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-078 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
while read set; do python3 iter/I-068/break_machines.py ta18 $G $K 50 $set >> $LOG 2>&1; done < $I/sets.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-078 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of 30"
grep -v "OPTIMAL, bound 1401" $LOG | grep -v "^==="
