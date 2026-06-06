#!/bin/bash
###############################################################################
# irace target runner for IJSP ABCPSO + TabuSearch
# Called by irace as:
#   target-runner.sh <config_id> <instance_id> <seed> <instance_path> \
#                    --elite_size 40 --elite_sel 3 ...
#
# Fixed in base_setup: pop_size=250, noimprove=20, cross_prob=0.9,
#                      mut_prob=0.10, tabu_min=3
###############################################################################

EXE="/home/diazhernan/FuzzyFW"
BASE_SETUP="$(dirname "$0")/base_setup_ts_lex2.txt"

# --- Parse irace arguments ---
CONFIG_ID="$1"
INSTANCE_ID="$2"
SEED="$3"
INSTANCE="$4"
shift 4

# Parse named parameters using eval (WSL1: declare -A broken)
while [[ $# -gt 0 ]]; do
    key="${1#--}"
    val="$2"
    eval "param_${key}=\$val"
    shift 2
done

# --- Neighbourhood hardcoded by run_irace.sh per neighbourhood ---
NB="ijsp.makespan.n2"

# --- Create temp workspace ---
TMPDIR=$(mktemp -d /tmp/irace_XXXXXX)
trap "rm -rf $TMPDIR" EXIT

SETUP_FILE="$TMPDIR/setup.txt"
OUT_DIR="$TMPDIR/out"
mkdir -p "$OUT_DIR"

# --- Generate setup file from base template ---
sed \
    -e "s/IRACE_SEED/${SEED}/" \
    -e "s/IRACE_ELITE_SIZE/${param_elite_size}/" \
    -e "s/IRACE_ELITE_SEL/${param_elite_sel}/" \
    -e "s/IRACE_MAXTRIALS/${param_maxtrials}/" \
    -e "s/IRACE_LS_TARGET/${param_ls_target}/" \
    -e "s/IRACE_TABU_ITER/${param_tabu_iter}/" \
    -e "s/IRACE_CROSSOVER/${param_crossover}/" \
    -e "s/IRACE_MUTATION/${param_mutation}/" \
    -e "s|IRACE_NEIGHBOURHOOD|${NB}|" \
    "$BASE_SETUP" > "$SETUP_FILE"

# --- Run FuzzyFW with WSL1-compatible background timeout ---
# Timeout set to 900s — covers worst case tai50 with TabuSearch (~750s observed)
setsid "$EXE" "$SETUP_FILE" "$INSTANCE" "$OUT_DIR" > /dev/null 2>&1 &
EXE_PID=$!
( sleep 900; kill -9 -${EXE_PID} 2>/dev/null; kill -9 ${EXE_PID} 2>/dev/null ) &
KILLER_PID=$!
wait ${EXE_PID}
kill ${KILLER_PID} 2>/dev/null
wait ${KILLER_PID} 2>/dev/null

# --- Extract best_mid from Sols.csv ---
SOLS=$(ls "$OUT_DIR"/*_Sols.csv 2>/dev/null | head -1)

if [[ -z "$SOLS" || $(wc -l < "$SOLS") -lt 2 ]]; then
    echo "Inf"
    exit 0
fi

# Parse (lower, upper) from each run, compute midpoint, return minimum
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
