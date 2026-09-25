#!/bin/bash
# The ta27 machine-pair campaign (I-082 on): pairs.txt (187 pairs, the three of
# I-081 excluded), lines <first>..<last>, 90 s each, I-079's group.
# Usage: batch.sh <first> <last>
set -u
F=$1; L=$2
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-082
G=iter/I-079/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run_${F}_${L}.log
echo "=== I-082 pairs $F-$L start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
sed -n "${F},${L}p" $I/pairs.txt | while read set; do
  python3 iter/I-068/break_machines.py ta27 $G $K 90 $set >> $LOG 2>&1
done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-082 pairs $F-$L exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of $((L - F + 1))"
grep -v "OPTIMAL, bound 1685" $LOG | grep -v "^==="
