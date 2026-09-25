#!/bin/bash
# I-055: consensus recombination over nested groups of CP-SAT floors.
# Fixed before running: the per-hint floors of I-054 (floors.py), lowest
# first; ta25 with K = 4 8 16 32 64 107, ta27 with K = 4 8 16 37; only the
# machine pairs every member orders alike are fixed; 120 s of CP-SAT each
# (14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-055
LOG=$I/run.log
echo "=== I-055 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
for inst in ta25 ta27; do python3 iter/I-054/floors.py $inst > $I/floors_$inst.txt; done
for K in 4 8 16 32 64 107; do python3 $I/consensus.py ta25 $I/floors_ta25.txt $K 120 >> $LOG 2>&1; done
for K in 4 8 16 37; do python3 $I/consensus.py ta27 $I/floors_ta27.txt $K 120 >> $LOG 2>&1; done
echo "=== I-055 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*_(ta[0-9]+)_[0-9]+_Certificate.csv/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
