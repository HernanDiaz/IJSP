#!/bin/bash
# I-053: deeper in the hint list. ta25, the distinct feasible schedules ranked
# 41 to 120 by makespan (1 to 40 were I-046's), 20 s of CP-SAT each (I-042
# model, 14 workers, seed 1).
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-053
LOG=$I/probe_run.log
echo "=== I-053 probe start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
python3 iter/I-043/top_schedules.py ta25 "results/I-0*/ta25_*_Certificate.csv" 120 | tail -n 80 > $I/hints_ta25.txt
echo "hints: $(wc -l < $I/hints_ta25.txt), makespans $(head -1 $I/hints_ta25.txt | cut -d' ' -f3) to $(tail -1 $I/hints_ta25.txt | cut -d' ' -f3)" >> $LOG
k=40
while read cert run ms; do
  k=$((k+1))
  echo "--- ta25 hint $k: $cert run $run makespan $ms" >> $LOG
  python3 iter/I-042/cpsat_probe.py ta25 $cert $run 20 >> $LOG 2>&1
  collect ta25 hint$k
done < $I/hints_ta25.txt
echo "=== I-053 probe exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance ta25 --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "^hints|^ta25: status" $LOG | sed -E 's/.*best ([0-9]+) \(hint ([0-9]+).*/\2 \1/' | awk 'NR==1{print; next} {n++; if ($2<$1) i++; if (m==""||$2<m) m=$2} END {print "hints", n, "improved", i, "best", m}'
grep -ciE "INFEAS|anomal" $LOG
