#!/bin/bash
#
# verify_refactor.sh
# Quick sanity check after refactoring: runs 4 small instances × 5 configs
# and compares results against the baseline in statistical_results_exp7/runs_data.csv.
#
# - Results go to /tmp/ijsp_verify (NEVER touches experiments/results/exp7/)
# - Exits 0 if all checks pass, 1 if any mismatch is detected.
#
# Usage (from WSL): bash experiments/verify_refactor.sh

set -euo pipefail

EXE="/home/diazhernan/FuzzyFW"
SCRIPT_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments"
INSTANCES_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardIntervalos"
SETUP_DIR="${SCRIPT_DIR}/setup/exp7"
BASELINE="${SCRIPT_DIR}/statistical_results_exp7/runs_data.csv"
VERIFY_DIR="/tmp/ijsp_verify"

# Verification subset: 4 small + 2 medium instances, all present in the baseline.
# Small (40-100 ops): quick sanity; medium (225-300 ops): catch regressions on
# harder instances without running the full 82-instance suite.
VERIFY_INSTANCES=(
    "F0.15.0.ft10_10"
    "F0.15.0.ft20_05"
    "F0.15.0.abz7_06"
    "F0.15.0.abz8_05"
    "tai15_15_01.F.15_01"
    "tai20_15_01.F.15_01"
)

CONFIGS=("N1_hc_tuned" "N2_hc_tuned" "N3_hc_tuned" "Next_hc_tuned" "N8_hc_tuned")
SETUP_FILES=(
    "${SETUP_DIR}/setup_N1_hc_tuned.txt"
    "${SETUP_DIR}/setup_N2_hc_tuned.txt"
    "${SETUP_DIR}/setup_N3_hc_tuned.txt"
    "${SETUP_DIR}/setup_Next_hc_tuned.txt"
    "${SETUP_DIR}/setup_N8_hc_tuned.txt"
)

echo "======================================================="
echo " IJSP Refactor Verification"
echo " Instances : ${#VERIFY_INSTANCES[@]} (small subset)"
echo " Configs   : ${#CONFIGS[@]}"
echo " Results   : ${VERIFY_DIR}"
echo " Baseline  : ${BASELINE}"
echo " Started   : $(date)"
echo "======================================================="

# Sanity checks
if [ ! -f "$EXE" ]; then
    echo "ERROR: executable not found: $EXE"
    exit 1
fi
if [ ! -f "$BASELINE" ]; then
    echo "ERROR: baseline not found: $BASELINE"
    exit 1
fi
for sf in "${SETUP_FILES[@]}"; do
    if [ ! -f "$sf" ]; then
        echo "ERROR: setup file not found: $sf"
        exit 1
    fi
done
for inst in "${VERIFY_INSTANCES[@]}"; do
    if [ ! -f "${INSTANCES_DIR}/${inst}.txt" ]; then
        echo "ERROR: instance not found: ${INSTANCES_DIR}/${inst}.txt"
        exit 1
    fi
done

# Create output directories
for cfg in "${CONFIGS[@]}"; do
    mkdir -p "${VERIFY_DIR}/${cfg}"
done

# Run all combinations in parallel (same pattern as run_exp7.sh)
PIDS=()
for i in "${!CONFIGS[@]}"; do
    cfg="${CONFIGS[$i]}"
    sf="${SETUP_FILES[$i]}"
    outdir="${VERIFY_DIR}/${cfg}"
    for inst in "${VERIFY_INSTANCES[@]}"; do
        inst_file="${INSTANCES_DIR}/${inst}.txt"
        log="${VERIFY_DIR}/${cfg}__${inst}.log"
        echo "  Launching: ${cfg} × ${inst}"
        timeout 300 "$EXE" "$sf" "$inst_file" "$outdir" > "$log" 2>&1 &
        PIDS+=($!)
    done
done

echo ""
echo "Waiting for ${#PIDS[@]} runs to finish..."
for pid in "${PIDS[@]}"; do
    wait "$pid" || true
done
echo "All runs done. $(date)"
echo ""

# Compare against baseline
python3 "${SCRIPT_DIR}/compare_verify.py" \
    --results-dir  "$VERIFY_DIR" \
    --baseline     "$BASELINE" \
    --configs      "${CONFIGS[@]}"

exit $?
