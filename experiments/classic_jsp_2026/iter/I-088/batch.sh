#!/bin/bash
# I-088: the core-breaking descent on ta25 (own best 1603, best known 1595),
# group = the 8 lowest per-hint floors (I-055: 1603 optimal in their consensus,
# 687 free pairs, 5 s). sets.txt: the 20 machines (30 s each) then the 190
# pairs (45 s each). Usage: batch.sh <first> <last>
set -u
F=$1; L=$2
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-088
G=$I/group_ta25.txt
K=$(wc -l < $G)
LOG=$I/run_${F}_${L}.log
echo "=== I-088 sets $F-$L start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
sed -n "${F},${L}p" $I/sets.txt | while read set; do
  case $set in *,*) secs=45 ;; *) secs=30 ;; esac
  python3 iter/I-068/break_machines.py ta25 $G $K $secs $set >> $LOG 2>&1
done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-088 sets $F-$L exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta25 --certificate $f --bounds taillard_bounds.csv >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of $((L - F + 1))"
grep -v "OPTIMAL, bound 1603" $LOG | grep -v "^==="
