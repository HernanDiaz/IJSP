#!/bin/bash
# I-079: the core-breaking method on ta27 (own best 1685, best known 1680). The
# 37-floor consensus of I-055 (1685 proven optimal inside it); each of the 20
# machines freed on its own, 60 s each.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-079
G=$I/group_ta27.txt
K=$(wc -l < $G)
LOG=$I/run.log
echo "=== I-079 start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
for m in $(seq 0 19); do python3 iter/I-068/break_machines.py ta27 $G $K 60 $m >> $LOG 2>&1; done
for f in iter/I-068/found_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/; done
echo "=== I-079 exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta27 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
echo "closed: $(grep -c 'status OPTIMAL' $LOG) of 20"
grep -v "OPTIMAL, bound 1685" $LOG | grep -v "^==="
