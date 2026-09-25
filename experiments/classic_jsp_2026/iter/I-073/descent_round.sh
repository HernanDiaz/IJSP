#!/bin/bash
# Core-breaking descent, one batch of a round (from I-073 on).
# Usage: descent_round.sh <out dir> <group file> <batch: m13 | 1 | 2 | 3> <seconds>
#   m13  : machine 13 alone
#   1..3 : machine pairs 1-35, 36-70, 71-105 in lexicographic order
# Uses iter/I-068/break_machines.py; found schedules go to <out dir>.
set -u
OUT=$1; GROUP=$2; B=$3; SECS=$4
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
LOG=$OUT/run_$B.log
echo "=== $OUT batch $B start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
K=$(wc -l < $GROUP)
if [ "$B" = "m13" ]; then
  echo 13 > $OUT/sets_$B.txt
else
  python3 -c "import itertools; [print('%d,%d' % p) for p in itertools.combinations(range(15), 2)]" | sed -n "$(( (B - 1) * 35 + 1 )),$(( B * 35 ))p" > $OUT/sets_$B.txt
fi
while read set; do python3 iter/I-068/break_machines.py ta18 $GROUP $K $SECS $set >> $LOG 2>&1; done < $OUT/sets_$B.txt
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $OUT/; done
echo "=== $OUT batch $B exit at $(date) ===" >> $LOG
for f in $OUT/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of $(wc -l < $OUT/sets_$B.txt)"
grep -v "OPTIMAL, bound $(head -1 $GROUP | cut -d' ' -f3)" $LOG | grep -v "^==="
