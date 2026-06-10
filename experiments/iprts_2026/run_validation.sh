#!/bin/bash
# IPRTS v1 quick validation: 12 selected classical instances, 5 runs x 60 s.
# Usage (from the repo root, inside WSL):
#   bash experiments/iprts_2026/run_validation.sh [setup_file] [results_dir]
set -u

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SETUP="${1:-$REPO/experiments/iprts_2026/setup/setup_IPRTS_val_quick.txt}"
OUTDIR="${2:-$REPO/experiments/iprts_2026/results/validation_v1}"
BIN="$REPO/../FuzzyFW"

INSTANCES="
F0.15.0.abz7_06.txt
F0.15.0.abz8_05.txt
F0.15.0.abz9_10.txt
F0.15.0.ft10_10.txt
F0.15.0.ft20_05.txt
F0.15.0.la21_04.txt
F0.15.0.la24_03.txt
F0.15.0.la25_04.txt
F0.15.0.la27_09.txt
F0.15.0.la29_03.txt
F0.15.0.la38_06.txt
F0.15.0.la40_05.txt
"

mkdir -p "$OUTDIR"
cd "$REPO"

for inst in $INSTANCES; do
    echo "=== $(date +%H:%M:%S) $inst ==="
    "$BIN" "$SETUP" "SelectosYTaillardIntervalos/$inst" "$OUTDIR/" \
        > "$OUTDIR/${inst%.txt}.out" 2>&1
    echo "    exit=$?"
done

echo "ALL DONE $(date +%H:%M:%S)"
