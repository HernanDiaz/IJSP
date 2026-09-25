#!/bin/bash
# I-050: long CP-SAT continuations from the best 30x15 schedules of I-048.
# Fixed before running: ta33 from 1802 and ta34 from 1834 for 600 s each,
# ta32 from 1811 and ta40 from 1688 for 300 s each (I-042 model, 14 workers,
# seed 1), on an empty machine.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
I=iter/I-050
LOG=$I/probe_run.log
echo "=== I-050 probe start at $(date) ===" > $LOG
if pgrep FuzzyFW >/dev/null; then echo "solvers alive, abort" >> $LOG; exit 1; fi
collect() { for f in iter/I-042/found_${1}_*_Certificate.csv; do [ -e "$f" ] && mv "$f" $I/found_${2}_$(basename $f); done; }
while read inst secs; do
  hint=$(ls iter/I-048/found_*_found_${inst}_*_Certificate.csv | sed -E 's/.*_([0-9]+)_Certificate.csv/\1 &/' | sort -n | head -1 | cut -d' ' -f2)
  echo "--- $inst continuation from $hint for $secs s" >> $LOG
  python3 iter/I-042/cpsat_probe.py $inst $hint 1 $secs >> $LOG 2>&1
  collect $inst cont
done <<LIST
ta33 600
ta34 600
ta32 300
ta40 300
LIST
echo "=== I-050 probe exit at $(date) ===" >> $LOG
for f in $I/found_*_Certificate.csv; do
  [ -e "$f" ] || continue
  inst=$(basename $f | sed -E 's/.*found_(ta[0-9]+)_.*/\1/')
  python3 scripts/verify_certificate.py --orlib reference/taillard_orlib.txt --instance $inst --certificate $f --bounds taillard_bounds.csv --quiet >> $LOG 2>&1
done
grep -E "^--- |status|INFEAS|anomal" $LOG
