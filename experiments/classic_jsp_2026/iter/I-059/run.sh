#!/bin/bash
# I-059: CP-SAT cold on ta22, no hint, seeds 1 to 10, 120 s each (14 workers);
# each final schedule compared with the ABC's 1613 attractor (the lowest
# feasible ta22 schedule of the line) as the share of machine pairs ordered
# alike. For scale: floors of distinct hints agree 86 to 97 % with the
# attractor (I-054).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-059
LOG=$I/run.log
echo "=== I-059 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
read ref rrun rms < <(python3 iter/I-043/top_schedules.py ta22 "results/I-0*/ta22_*_Certificate.csv" 1)
echo "reference: $ref run $rrun makespan $rms" >> $LOG
for seed in $(seq 1 10); do
  python3 $I/cold_probe.py ta22 $ref $rrun 120 $seed $ref $rrun >> $LOG 2>&1
done
echo "=== I-059 exit at $(date) ===" >> $LOG
for f in $I/cold_*_Certificate.csv; do
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta22 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
