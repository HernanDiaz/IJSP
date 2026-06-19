#!/bin/bash
###############################################################################
# irace target runner — IJSP pure Genetic Algorithm (makespan, LEX2 ranking).
# Called by irace as:
#   target-runner.sh <config_id> <instance_id> <seed> <instance_path> \
#       --pop 200 --cxpb 0.9 --mutpb 0.1 --tourn 3 --crossover ijsp.jox [--n8moves 3]
# The MUTATION operator is hardcoded per run by run_irace_ga.sh (MUT_OPERATOR).
# Fixed in base_setup: noImprovement=25, timelimit=120.
###############################################################################

EXE="/home/diazhernan/FuzzyFW_ga"
BASE_SETUP="$(dirname "$0")/base_setup_ga.txt"

CONFIG_ID="$1"
INSTANCE_ID="$2"
SEED="$3"
INSTANCE="$4"
shift 4

# Parse named parameters (WSL1: declare -A broken, so eval into param_<name>)
while [[ $# -gt 0 ]]; do
    key="${1#--}"
    val="$2"
    eval "param_${key}=\$val"
    shift 2
done

# Mutation operator hardcoded by run_irace_ga.sh:
MUT="MUT_OPERATOR"
# n8.moves only applies to the N8 mutation; default 1 (ignored by Swap/Insertion)
N8MOVES="${param_n8moves:-1}"

TMPDIR=$(mktemp -d /tmp/iraceGA_XXXXXX)
trap "rm -rf $TMPDIR" EXIT
SETUP_FILE="$TMPDIR/setup.txt"
OUT_DIR="$TMPDIR/out"
mkdir -p "$OUT_DIR"

# Generate setup from the template
sed \
    -e "s/IRACE_SEED/${SEED}/" \
    -e "s|IRACE_MUTATION|${MUT}|" \
    -e "s|IRACE_CROSSOVER|${param_crossover}|" \
    -e "s/IRACE_N8MOVES/${N8MOVES}/" \
    -e "s/IRACE_POP/${param_pop}/" \
    -e "s/IRACE_CXPB/${param_cxpb}/" \
    -e "s/IRACE_MUTPB/${param_mutpb}/" \
    -e "s/IRACE_TOURN/${param_tourn}/" \
    "$BASE_SETUP" > "$SETUP_FILE"

# Run FuzzyFW with a WSL1-compatible background timeout (GA stops itself at
# timelimit=120 / noImprovement=25; this killer is just a safety net).
TIMEOUT=${IRACE_TIMEOUT:-200}
setsid "$EXE" "$SETUP_FILE" "$INSTANCE" "$OUT_DIR" > /dev/null 2>&1 &
EXE_PID=$!
( sleep ${TIMEOUT}; kill -9 -${EXE_PID} 2>/dev/null; kill -9 ${EXE_PID} 2>/dev/null ) &
KILLER_PID=$!
wait ${EXE_PID}
kill ${KILLER_PID} 2>/dev/null
wait ${KILLER_PID} 2>/dev/null

# Objective for irace: minimum best-of-run makespan midpoint from *_Sols.csv
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
