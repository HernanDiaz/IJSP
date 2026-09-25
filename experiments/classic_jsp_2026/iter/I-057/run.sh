#!/bin/bash
# I-057: break the ta25 consensus core one machine at a time. The consensus of
# all 107 floors (I-056: 1603 proven optimal inside it) with the consensus
# pairs of one machine freed, for each of the 20 machines; 60 s of CP-SAT
# each (14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-057
LOG=$I/run.log
echo "=== I-057 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
cp iter/I-055/floors_ta25.txt $I/floors_ta25.txt
for m in $(seq 0 19); do python3 $I/break_machine.py ta25 $I/floors_ta25.txt 107 60 $m >> $LOG 2>&1; done
echo "=== I-057 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta25 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
