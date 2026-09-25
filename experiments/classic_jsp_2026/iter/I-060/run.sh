#!/bin/bash
# I-060: the joint consensus of two generators on ta22. The group is every
# per-hint CP-SAT floor of I-045/I-058 plus the ten cold CP-SAT schedules of
# I-059 (group.py); only machine pairs ALL of them order alike are fixed;
# 600 s of CP-SAT (14 workers, seed 1), hinted with the group's best.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-060
LOG=$I/run.log
echo "=== I-060 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
# the ABC floors of ta22 were only kept when better than their hint; a hint
# that never moved (1613 -> 1613) has no found file, so the lowest feasible
# stored schedule is added as the group's best
python3 iter/I-043/top_schedules.py ta22 "results/I-0*/ta22_*_Certificate.csv" 1 > $I/group_ta22.txt
python3 $I/group.py >> $I/group_ta22.txt
echo "group: $(wc -l < $I/group_ta22.txt) schedules, $(cut -d' ' -f3 $I/group_ta22.txt | sort -n | uniq -c | tr -s ' ' | tr '\n' ';')" >> $LOG
K=$(wc -l < $I/group_ta22.txt)
python3 iter/I-055/consensus.py ta22 $I/group_ta22.txt $K 600 >> $LOG 2>&1
for f in iter/I-055/found_K*_ta22_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-060 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta22 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
