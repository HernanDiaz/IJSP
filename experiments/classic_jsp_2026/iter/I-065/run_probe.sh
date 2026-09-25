#!/bin/bash
# I-065: deeper in the hint list. ta18, the distinct feasible schedules ranked
# 21 to 80 by makespan (1 to 20 were I-064's), 20 s of CP-SAT each (I-042
# model, 14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-065
LOG=$I/probe_run.log
echo "=== I-065 probe start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
python3 iter/I-043/top_schedules.py ta18 "results/I-0*/ta18_*_Certificate.csv" 80 | tail -n 60 > $I/hints_ta18.txt
echo "hints: $(wc -l < $I/hints_ta18.txt), makespans $(head -1 $I/hints_ta18.txt | cut -d' ' -f3) to $(tail -1 $I/hints_ta18.txt | cut -d' ' -f3)" >> $LOG
k=20
while read cert run ms; do
  k=$((k+1))
  echo "--- ta18 hint $k: $cert run $run makespan $ms" >> $LOG
  python3 iter/I-042/cpsat_probe.py ta18 $cert $run 20 >> $LOG 2>&1
  collect ta18 hint$k
done < $I/hints_ta18.txt
echo "=== I-065 probe exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta18 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "^hints|^ta18: status" $LOG | sed -E 's/.*best ([0-9]+) \(hint ([0-9]+).*/\2 \1/' | awk 'NR==1{print; next} {n++; if ($2<$1) i++; if (m==""||$2<m) m=$2} END {print "hints", n, "improved", i, "best", m}'
grep -ciE "INFEAS|anomal" $LOG
