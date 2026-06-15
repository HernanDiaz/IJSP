#!/bin/bash
# Via 1 test: run the 20 x20-machine Taillard instances (tai30_20, tai50_20)
# -- the classes where cold IPRTS loses to TS-N2 -- with a given setup, to
# test whether capping the path-relinking walk recovers them.
#
# Launch DETACHED from PowerShell:
#   wsl bash -c "setsid nohup bash <this> [setup] [outdir] > <log> 2>&1 < /dev/null &"
set -u

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SETUP="${1:-$REPO/experiments/iprts_2026/setup/setup_IPRTS_via1_cap40.txt}"
OUTDIR="${2:-$REPO/experiments/iprts_2026/results/via1_cap40}"
BIN="$REPO/../FuzzyFW"
INST="$REPO/SelectosYTaillardIntervalos"

mkdir -p "$OUTDIR"
cd "$REPO"

jobs=()
for c in tai30_20 tai50_20; do
    for k in 01 02 03 04 05 06 07 08 09 10; do
        f="$INST/${c}_${k}.F.15_01.txt"
        [ -e "$f" ] && jobs+=("$f")
    done
done

echo "=== Via 1 test start $(date) -- ${#jobs[@]} instances"
echo "=== setup: $SETUP"

for f in "${jobs[@]}"; do
    stem="$(basename "${f%.txt}")"
    if grep -q 'The process has finished' "$OUTDIR/$stem.out" 2>/dev/null; then
        echo "skip $stem (already finished)"; continue
    fi
    rm -f "$OUTDIR/$stem"*
    ( "$BIN" "$SETUP" "$f" "$OUTDIR/" > "$OUTDIR/$stem.out" 2>&1; echo "done $stem $(date +%T)" ) &
done
wait
echo "=== VIA 1 TEST COMPLETE $(date)"
