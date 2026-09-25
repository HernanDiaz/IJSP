#!/bin/bash
# I-061: I-060's joint-consensus subspace of ta22 (30 schedules, 847 free
# machine pairs), now with 1500 s of CP-SAT (14 workers, seed 1): the most one
# iteration holds.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-061
LOG=$I/run.log
echo "=== I-061 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
cp iter/I-060/group_ta22.txt $I/group_ta22.txt
python3 iter/I-055/consensus.py ta22 $I/group_ta22.txt 30 1500 >> $LOG 2>&1
for f in iter/I-055/found_K*_ta22_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-061 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta22 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
