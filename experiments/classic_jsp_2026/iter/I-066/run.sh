#!/bin/bash
# I-066: the consensus core of ta18 (floors of I-064/I-065) and ta26 (floors
# of I-045/I-058), plus each one's lowest feasible stored schedule; all of
# them in the consensus; 600 s of CP-SAT each (14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-066
LOG=$I/run.log
echo "=== I-066 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
python3 $I/floors.py ta18 I-064 I-065 > $I/group_ta18.txt
python3 iter/I-043/top_schedules.py ta18 "results/I-0*/ta18_*_Certificate.csv" 1 >> $I/group_ta18.txt
python3 $I/floors.py ta26 I-045 I-058 > $I/group_ta26.txt
python3 iter/I-043/top_schedules.py ta26 "results/I-0*/ta26_*_Certificate.csv" 1 >> $I/group_ta26.txt
for inst in ta18 ta26; do
  sort -k3 -n $I/group_$inst.txt -o $I/group_$inst.txt
  K=$(wc -l < $I/group_$inst.txt)
  echo "$inst group: $K schedules, $(cut -d' ' -f3 $I/group_$inst.txt | head -1) to $(cut -d' ' -f3 $I/group_$inst.txt | tail -1)" >> $LOG
  python3 iter/I-055/consensus.py $inst $I/group_$inst.txt $K 600 >> $LOG 2>&1
  for f in iter/I-055/found_K*_${inst}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
done
echo "=== I-066 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*_(ta[0-9]+)_[0-9]+_Certificate.csv/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
