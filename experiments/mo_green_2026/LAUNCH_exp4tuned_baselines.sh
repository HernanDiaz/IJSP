#!/bin/bash
# Detached launcher for the TUNED baselines re-run (ABC-P #118, MA-P #125).
# Writes to results/EXP4_TUNED — inherited data untouched.
# Usage: LAUNCH_exp4tuned_baselines.sh <tier> [par]
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
TIER="${1:-classical}"; PAR="${2:-14}"
tr -d '\r' < "$D/run_exp4_baselines.sh" > /tmp/exp4b_tuned.sh
# LOG_FILE (runner's internal tee) MUST differ from the stdout redirect
# below, or the two writers race and truncate each other.
OUT_DIR="$PWD/$D/results/EXP4_TUNED" \
ABCP_SETUP="$PWD/$D/setup/setup_ABCP_irace.txt" \
MAP_SETUP="$PWD/$D/setup/setup_MAP_irace.txt" \
LOG_FILE="$PWD/$D/exp4tuned_baselines_${TIER}.log" \
  setsid nohup bash /tmp/exp4b_tuned.sh "$TIER" "$PAR" \
  > "$D/exp4tuned_baselines_${TIER}.out" 2>&1 < /dev/null &
echo "launched pid $! (tier=$TIER) -> $D/exp4tuned_baselines_${TIER}.out"
