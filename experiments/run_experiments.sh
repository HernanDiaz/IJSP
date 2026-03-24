#!/bin/bash
#
# run_experiments.sh
# Runs all 16 x 92 = 1472 IJSP experiments with controlled parallelism (max 20 simultaneous)
#
# Usage: bash run_experiments.sh
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IJSP_DIR="$(dirname "$SCRIPT_DIR")"
EXE="/c/Users/diazhernan/CLionProjects/FuzzyFW.exe"
SETUP_DIR="${SCRIPT_DIR}/setup"
INSTANCES_DIR="${IJSP_DIR}/SelectosYTaillardIntervalos"
RESULTS_BASE="${SCRIPT_DIR}/results"

NEIGHBOURHOODS=("n1" "n2" "n3" "nh")
COMPARATORS=("EV" "LEX1" "LEX2" "YX")

MAX_PARALLEL=20

echo "======================================================"
echo " IJSP Makespan Experiment Runner"
echo " EXE:       $EXE"
echo " Instances: $INSTANCES_DIR"
echo " Results:   $RESULTS_BASE"
echo " Max parallel processes: $MAX_PARALLEL"
echo "======================================================"

# Verify executable exists
if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE"
    exit 1
fi

# Count total experiments
TOTAL=0
for nb in "${NEIGHBOURHOODS[@]}"; do
    for comp in "${COMPARATORS[@]}"; do
        for instance in "${INSTANCES_DIR}"/*.txt; do
            TOTAL=$((TOTAL + 1))
        done
    done
done
echo "Total experiments to run: $TOTAL"
echo ""

PIDS=()
COMPLETED=0
FAILED=0

for nb in "${NEIGHBOURHOODS[@]}"; do
    for comp in "${COMPARATORS[@]}"; do
        SETUP_FILE="${SETUP_DIR}/setup_${nb}_${comp}.txt"
        RESULTS_DIR="${RESULTS_BASE}/${nb}_${comp}"

        # Create results directory if needed
        mkdir -p "$RESULTS_DIR"

        echo "--- Running setup: ${nb}_${comp} ---"

        for instance in "${INSTANCES_DIR}"/*.txt; do
            instance_name="$(basename "$instance")"

            # Launch experiment in background
            "$EXE" "$SETUP_FILE" "$instance" "$RESULTS_DIR" &
            PIDS+=($!)

            # Throttle parallelism to MAX_PARALLEL
            if [ ${#PIDS[@]} -ge $MAX_PARALLEL ]; then
                wait ${PIDS[0]}
                exit_code=$?
                if [ $exit_code -ne 0 ]; then
                    FAILED=$((FAILED + 1))
                fi
                PIDS=("${PIDS[@]:1}")
                COMPLETED=$((COMPLETED + 1))
                if [ $((COMPLETED % 20)) -eq 0 ]; then
                    echo "Progress: $COMPLETED / $TOTAL completed ($FAILED failed)"
                fi
            fi
        done
    done
done

# Wait for all remaining background processes
echo "Waiting for remaining processes to finish..."
for pid in "${PIDS[@]}"; do
    wait "$pid"
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        FAILED=$((FAILED + 1))
    fi
    COMPLETED=$((COMPLETED + 1))
done

echo ""
echo "======================================================"
echo " Experiments finished!"
echo " Total:     $TOTAL"
echo " Completed: $COMPLETED"
echo " Failed:    $FAILED"
echo "======================================================"
