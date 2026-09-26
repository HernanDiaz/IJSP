#!/bin/bash
# I-087: ta27 from 1680, 30 machine triples drawn at random (random.Random(1)
# over the 1140 triples, sets.txt), 60 s each; I-085's group.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-087
G=iter/I-085/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-087 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
while read set; do python3 iter/I-068/break_machines.py ta27 $G $K 60 $set >> $LOG 2>&1; done < $I/sets.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-087 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of 30"
grep -v "OPTIMAL, bound 1680" $LOG | grep -v "^==="
