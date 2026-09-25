#!/bin/bash
# I-067: break the ta18 core one machine at a time. The 27-schedule consensus
# of I-066 (1405 proven optimal inside it, 16 s) with the consensus pairs of
# one machine freed, for each of the 15 machines; 60 s of CP-SAT each.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-067
LOG=$I/run.log
echo "=== I-067 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
cp iter/I-066/group_ta18.txt $I/group_ta18.txt
K=$(wc -l < $I/group_ta18.txt)
for m in $(seq 0 14); do python3 iter/I-057/break_machine.py ta18 $I/group_ta18.txt $K 60 $m >> $LOG 2>&1; done
for f in iter/I-057/found_m*_ta18_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-067 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
