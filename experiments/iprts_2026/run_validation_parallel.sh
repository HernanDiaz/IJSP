#!/bin/bash
# IPRTS quick validation, parallel version: launches every instance that does
# not yet have a finished marker, one process per instance (machine has many
# more cores than instances).
# Usage (from anywhere, inside WSL):
#   bash run_validation_parallel.sh [setup_file] [results_dir]
set -u

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SETUP="${1:-$REPO/experiments/iprts_2026/setup/setup_IPRTS_val_quick.txt}"
OUTDIR="${2:-$REPO/experiments/iprts_2026/results/validation_v1}"
BIN="$REPO/../FuzzyFW"

INSTANCES="
F0.15.0.abz7_06
F0.15.0.abz8_05
F0.15.0.abz9_10
F0.15.0.ft10_10
F0.15.0.ft20_05
F0.15.0.la21_04
F0.15.0.la24_03
F0.15.0.la25_04
F0.15.0.la27_09
F0.15.0.la29_03
F0.15.0.la38_06
F0.15.0.la40_05
"

mkdir -p "$OUTDIR"
cd "$REPO"

launched=0
for inst in $INSTANCES; do
    if grep -q 'The process has finished' "$OUTDIR/$inst.out" 2>/dev/null; then
        echo "skip $inst (already finished)"
        continue
    fi
    rm -f "$OUTDIR/$inst"*
    "$BIN" "$SETUP" "SelectosYTaillardIntervalos/$inst.txt" "$OUTDIR/" \
        > "$OUTDIR/$inst.out" 2>&1 &
    launched=$((launched + 1))
    echo "launched $inst (pid $!)"
done

echo "waiting for $launched processes..."
wait
echo "ALL_PARALLEL_DONE $(date +%H:%M:%S)"
