#!/bin/bash
###############################################################################
# irace target runner for IPRTS (cold start, perturbation restarts)
# Called by irace as:
#   target-runner.sh <config_id> <instance_id> <seed> <instance_path> \
#                    --pool_size 10 --pool_mindist 0.05 ...
#
# Mirrors cor_tabu_2026/irace/target-runner.sh (same chain: .cmd -> .ps1 ->
# this script inside WSL; objective = best run midpoint from *_Sols.csv).
###############################################################################

EXE="/home/diazhernan/FuzzyFW_iprts"
BASE_SETUP="$(dirname "$0")/base_setup_iprts.txt"

# --- Parse irace arguments ---
CONFIG_ID="$1"
INSTANCE_ID="$2"
SEED="$3"
INSTANCE="$4"
shift 4

while [[ $# -gt 0 ]]; do
    key="${1#--}"
    val="$2"
    eval "param_${key}=\$val"
    shift 2
done

# --- Create temp workspace ---
TMPDIR=$(mktemp -d /tmp/irace_iprts_XXXXXX)
trap "rm -rf $TMPDIR" EXIT

SETUP_FILE="$TMPDIR/setup.txt"
OUT_DIR="$TMPDIR/out"
mkdir -p "$OUT_DIR"

# --- Generate setup file from base template ---
sed \
    -e "s/IRACE_SEED/${SEED}/" \
    -e "s/IRACE_POOL_SIZE/${param_pool_size}/" \
    -e "s/IRACE_POOL_MINDIST/${param_pool_mindist}/" \
    -e "s/IRACE_PATIENCE/${param_patience}/" \
    -e "s/IRACE_PERTURB/${param_perturb}/" \
    -e "s/IRACE_TABU_ITER/${param_tabu_iter}/" \
    -e "s/IRACE_TABU_MAX/${param_tabu_max}/" \
    -e "s/IRACE_MAX_ROUNDS/${param_max_rounds}/" \
    "$BASE_SETUP" > "$SETUP_FILE"

# --- Run FuzzyFW with background timeout (timelimit 300 + margin) ---
TIMEOUT=${IRACE_TIMEOUT:-360}
setsid "$EXE" "$SETUP_FILE" "$INSTANCE" "$OUT_DIR" > /dev/null 2>&1 &
EXE_PID=$!
( sleep ${TIMEOUT}; kill -9 -${EXE_PID} 2>/dev/null; kill -9 ${EXE_PID} 2>/dev/null ) &
KILLER_PID=$!
wait ${EXE_PID}
kill ${KILLER_PID} 2>/dev/null
wait ${KILLER_PID} 2>/dev/null

# --- Extract best midpoint from Sols.csv ---
SOLS=$(ls "$OUT_DIR"/*_Sols.csv 2>/dev/null | head -1)

if [[ -z "$SOLS" || $(wc -l < "$SOLS") -lt 2 ]]; then
    echo "Inf"
    exit 0
fi

best_mid=$(awk -F';' 'NR>1 {
    ov = $NF
    gsub(/[() ]/, "", ov)
    n = split(ov, a, ",")
    if (n == 2) {
        mid = (a[1] + a[2]) / 2.0
        if (NR == 2 || mid < best) best = mid
    }
} END { print best }' "$SOLS")

echo "$best_mid"
