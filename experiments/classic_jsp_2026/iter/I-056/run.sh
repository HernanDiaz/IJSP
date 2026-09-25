#!/bin/bash
# I-056: the ta25 consensus subspaces I-055 could not close in 120 s, now with
# 600 s each: K = 64 and K = 107 (14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-056
LOG=$I/run.log
echo "=== I-056 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
cp iter/I-055/floors_ta25.txt $I/floors_ta25.txt
for K in 64 107; do python3 iter/I-055/consensus.py ta25 $I/floors_ta25.txt $K 600 >> $LOG 2>&1; done
for f in iter/I-055/found_K*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-056 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta25 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
cat $LOG
