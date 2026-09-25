#!/bin/bash
# I-068 to I-070: break the ta18 core two machines at a time. The 27-schedule
# consensus of I-066 with the consensus pairs of a PAIR of machines freed; the
# 105 pairs in lexicographic order split in three batches (1-35, 36-70,
# 71-105), 45 s of CP-SAT each. Usage: run.sh <batch 1|2|3>
set -u
B=${1:?batch}
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-068
LOG=$I/run_batch$B.log
echo "=== I-068 batch $B start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
cp iter/I-066/group_ta18.txt $I/group_ta18.txt
K=$(wc -l < $I/group_ta18.txt)
python3 -c "import itertools; [print('%d,%d' % p) for p in itertools.combinations(range(15), 2)]" > $I/pairs.txt
first=$(( (B - 1) * 35 + 1 )); last=$(( B * 35 ))
sed -n "${first},${last}p" $I/pairs.txt | while read pair; do
  python3 $I/break_machines.py ta18 $I/group_ta18.txt $K 45 $pair >> $LOG 2>&1
done
echo "=== I-068 batch $B exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -c OPTIMAL $LOG; grep -v "OPTIMAL, bound 1405" $LOG
