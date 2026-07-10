#!/bin/bash
# Detached launcher for the TUNED LADDER re-run (irace config #70).
# Writes to results/EXP4_TUNED/LADDER — the inherited EXP4/LADDER data is
# left untouched. Usage: LAUNCH_exp4tuned_ladder.sh <tier> [par]
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
TIER="${1:-classical}"; PAR="${2:-14}"
tr -d '\r' < "$D/run_exp4_ladder.sh" > /tmp/exp4l_tuned.sh
OUT_DIR="$PWD/$D/results/EXP4_TUNED/LADDER" \
BASE_SETUP="$PWD/$D/setup/setup_LADDER_irace.txt" \
LOG_FILE="$PWD/$D/exp4tuned_ladder_${TIER}.out" \
  setsid nohup bash /tmp/exp4l_tuned.sh "$TIER" "$PAR" \
  > "$D/exp4tuned_ladder_${TIER}.out" 2>&1 < /dev/null &
echo "launched pid $! (tier=$TIER) -> $D/exp4tuned_ladder_${TIER}.out"
