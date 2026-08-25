#!/usr/bin/env bash
cd "$(dirname "$0")/.."
n=$(pgrep -c FuzzyFW 2>/dev/null || echo 0)
echo "FuzzyFW vivos antes: $n"
if [ "$n" -gt 0 ]; then echo "ABORTA: hay FuzzyFW vivos"; exit 1; fi
nohup bash final/run_phase0.sh > final/phase0_nohup.log 2>&1 &
echo "PHASE0 LAUNCHED pid=$!"
sleep 10
tail -2 final/phase0/run.log 2>/dev/null
echo "FuzzyFW vivos ahora: $(pgrep -c FuzzyFW 2>/dev/null || echo 0)"
