#!/bin/bash
# I-089: all remaining ta18 machine triples (455 minus the 30 of I-078) from
# the 1401 group (iter/I-074/group_ta18.txt), 50 s each. Usage: batch.sh <first> <last>
set -u
F=$1; L=$2
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-089
G=iter/I-074/group_ta18.txt
K=$(wc -l < $G)
LOG=$I/run_${F}_${L}.log
echo "=== I-089 triples $F-$L start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
sed -n "${F},${L}p" $I/sets.txt | while read set; do
  python3 iter/I-068/break_machines.py ta18 $G $K 50 $set >> $LOG 2>&1
done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-089 triples $F-$L exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of $((L - F + 1))"
grep -v "OPTIMAL, bound 1401" $LOG | grep -v "^==="
