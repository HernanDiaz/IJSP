#!/bin/bash
# I-052: CP-SAT's own randomness. The three hints that gave the best 20x20 own
# bests still above the best known, each with CP-SAT seeds 2 to 13, 45 s each
# (I-042 model, 14 workers). Seed 1 is what I-045/I-046 already ran.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-052
LOG=$I/probe_run.log
echo "=== I-052 probe start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
while read inst cert run; do
  for seed in $(seq 2 13); do
    echo "--- $inst seed $seed: $cert run $run" >> $LOG
    python3 iter/I-042/cpsat_probe.py $inst $cert $run 45 $seed >> $LOG 2>&1
    collect $inst s$seed
  done
done <<LIST
ta25 results/I-041_w3_p03_side/ta25_20260925081410_Certificate.csv 1
ta26 results/I-002_w1_control/ta26_20260921170818_Certificate.csv 5
ta27 results/I-030_w2_s6003_current/ta27_20260924110410_Certificate.csv 1
LIST
echo "=== I-052 probe exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*found_(ta[0-9]+)_.*/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "^--- |status|INFEAS|anomal" $LOG
