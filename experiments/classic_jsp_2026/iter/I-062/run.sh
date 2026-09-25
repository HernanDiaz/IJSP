#!/bin/bash
# I-062: search against the ta22 core. The 30-schedule core of I-060/I-061;
# a schedule of makespan <= 1612 flipping at least D core pairs, D = 10, 50,
# 150, 480 s of CP-SAT each (14 workers, seed 1), no hint.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-062
LOG=$I/run.log
echo "=== I-062 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
for D in 10 50 150; do python3 $I/anticore.py ta22 iter/I-061/group_ta22.txt 30 1612 $D 480 >> $LOG 2>&1; done
echo "=== I-062 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta22 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
