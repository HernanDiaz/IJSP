#!/bin/bash
# Phase 2: full IPRTS run over the paper's 82 instances (12 selectas + 70
# Taillard up to tai50x20), 30 runs each (from the setup). Baseline TS-N2 is
# NOT rerun: compare against experiments/cor_tabu_2026/statistical_results.
#
# Job queue with longest-processing-time-first (LPT) ordering and a fixed pool
# of parallel slots: the 7.5 h tai50x20 instances start first and the cheap
# selectas backfill, minimising idle cores at the tail.
#
# Launch DETACHED (survives session close):
#   wsl bash -c "setsid nohup bash <this> [setup] [outdir] [slots] \
#       > <log> 2>&1 < /dev/null &"
set -u

REPO="$(cd "$(dirname "$0")/../.." && pwd)"
SETUP="${1:-$REPO/experiments/iprts_2026/setup/setup_IPRTS_cold_tuned.txt}"
OUTDIR="${2:-$REPO/experiments/iprts_2026/results/phase2_cold_tuned}"
SLOTS="${3:-26}"
BIN="$REPO/../FuzzyFW"
INST="$REPO/SelectosYTaillardIntervalos"

mkdir -p "$OUTDIR"
cd "$REPO"

# Build the instance list, largest class first (LPT). Within Taillard a class
# is globbed; selectas are listed last (cheapest).
classes="tai50_20 tai50_15 tai30_20 tai30_15 tai20_20 tai20_15 tai15_15"
jobs=()
for c in $classes; do
    for f in "$INST/${c}"_*.F.15_01.txt; do
        [ -e "$f" ] && jobs+=("$f")
    done
done
for f in "$INST"/F0.15.0.*.txt; do
    [ -e "$f" ] && jobs+=("$f")
done

echo "=== Phase 2 start $(date) — ${#jobs[@]} instances, $SLOTS slots"
echo "=== setup: $SETUP"

launch() {
    local f="$1"
    local stem="$(basename "${f%.txt}")"
    if grep -q 'The process has finished' "$OUTDIR/$stem.out" 2>/dev/null; then
        echo "skip $stem (already finished)"; return
    fi
    rm -f "$OUTDIR/$stem"*
    "$BIN" "$SETUP" "$f" "$OUTDIR/" > "$OUTDIR/$stem.out" 2>&1
    echo "done $stem $(date +%T)"
}

running=0
for f in "${jobs[@]}"; do
    launch "$f" &
    running=$((running + 1))
    if [ "$running" -ge "$SLOTS" ]; then
        wait -n
        running=$((running - 1))
    fi
done
wait
echo "=== PHASE 2 COMPLETE $(date)"
