#!/bin/bash
# I-084: ta27 round 2 from 1684. sets.txt holds machines 15 and 17 (run at 180 s)
# then the 37 pairs containing 15 or 17 (90 s each); lines <first>..<last>.
# Usage: batch.sh <first> <last>
set -u
F=$1; L=$2
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-084
G=iter/I-083/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run_${F}_${L}.log
echo "=== I-084 sets $F-$L start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
sed -n "${F},${L}p" $I/sets.txt | while read set; do
  case $set in *,*) secs=90 ;; *) secs=180 ;; esac
  python3 iter/I-068/break_machines.py ta27 $G $K $secs $set >> $LOG 2>&1
done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-084 sets $F-$L exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of $((L - F + 1))"
grep -v "OPTIMAL, bound 1684" $LOG | grep -v "^==="
