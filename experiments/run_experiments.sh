#!/bin/bash
#
# run_experiments.sh
# Runs all 20 x 92 = 1840 IJSP experiments, always keeping MAX_PARALLEL processes running.
# Skips experiments whose result file already exists (safe to restart).
#
# Usage: bash run_experiments.sh
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IJSP_DIR="$(dirname "$SCRIPT_DIR")"
EXE="/c/Users/diazhernan/CLionProjects/FuzzyFW.exe"
SETUP_DIR="${SCRIPT_DIR}/setup"
INSTANCES_DIR="${IJSP_DIR}/SelectosYTaillardIntervalos"
RESULTS_BASE="${SCRIPT_DIR}/results"

NEIGHBOURHOODS=("n1" "n2" "n3" "nh" "next")
COMPARATORS=("EV" "LEX1" "LEX2" "YX")

MAX_PARALLEL=28

echo "======================================================"
echo " IJSP Makespan Experiment Runner"
echo " EXE:       $EXE"
echo " Instances: $INSTANCES_DIR"
echo " Results:   $RESULTS_BASE"
echo " Max parallel: $MAX_PARALLEL"
echo "======================================================"

if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE"
    exit 1
fi

# Build list of pending experiments (skip already completed)
SETUP_FILES=()
INSTANCE_FILES=()
RESULT_DIRS=()
SKIPPED=0


# Pre-read instance names once (avoids 1472 basename subprocesses in the loop).
# INSTANCE_NAMES[i] = bare name without path or .txt extension.
# INSTANCE_PATHS[i] = full path.
mapfile -t INSTANCE_PATHS < <(ls "${INSTANCES_DIR}"/*.txt 2>/dev/null)
INSTANCE_NAMES=()
for _p in "${INSTANCE_PATHS[@]}"; do
    _b="${_p##*/}"          # strip leading path (bash, no subprocess)
    INSTANCE_NAMES+=("${_b%.txt}")  # strip .txt
done

for nb in "${NEIGHBOURHOODS[@]}"; do
    for comp in "${COMPARATORS[@]}"; do
        SETUP_FILE="${SETUP_DIR}/setup_${nb}_${comp}.txt"
        RESULTS_DIR="${RESULTS_BASE}/${nb}_${comp}"
        mkdir -p "$RESULTS_DIR"

        # ONE ls+grep call per config to find completed main CSVs.
        # Main CSV pattern: {name}_{14-digit-timestamp}.csv
        # _Robustness.csv / _Sols.csv / _Scenarios.csv are excluded by the regex.
        declare -A done_set
        while IFS= read -r fname; do
            # Strip _NNNNNNNNNNNNNN.csv suffix using bash globbing (no subprocess)
            key="${fname%_??????????????.csv}"   # 14 ? chars
            done_set["$key"]=1
        done < <(ls "${RESULTS_DIR}/" 2>/dev/null | grep -E '_[0-9]{14}\.csv$')

        for i in "${!INSTANCE_NAMES[@]}"; do
            instance_name="${INSTANCE_NAMES[$i]}"
            # Skip tai100 instances (run separately later)
            [[ "$instance_name" == tai100* ]] && continue
            if [ "${done_set[$instance_name]+x}" ]; then
                SKIPPED=$((SKIPPED + 1))
                continue
            fi
            SETUP_FILES+=("$SETUP_FILE")
            INSTANCE_FILES+=("${INSTANCE_PATHS[$i]}")
            RESULT_DIRS+=("$RESULTS_DIR")
        done

        unset done_set
    done
done

TOTAL=${#SETUP_FILES[@]}
echo "Skipped (already done): $SKIPPED"
echo "Experiments to run:     $TOTAL"
echo ""

if [ $TOTAL -eq 0 ]; then
    echo "All experiments already completed."
    exit 0
fi

PIDS=()
NEXT=0
COMPLETED=0
FAILED=0

# Fill initial pool
while [ $NEXT -lt $TOTAL ] && [ ${#PIDS[@]} -lt $MAX_PARALLEL ]; do
    "${EXE}" "${SETUP_FILES[$NEXT]}" "${INSTANCE_FILES[$NEXT]}" "${RESULT_DIRS[$NEXT]}" &
    PIDS+=($!)
    NEXT=$((NEXT + 1))
done
echo "Launched initial batch of ${#PIDS[@]} processes."

# Main loop: as soon as any slot frees up, launch the next experiment
while [ ${#PIDS[@]} -gt 0 ]; do

    # Wait for ANY child process to finish; capture its exit code directly.
    # Do NOT call wait $pid again afterwards — bash returns 127 for already-reaped pids.
    wait -n 2>/dev/null
    finished_code=$?

    # Identify which PIDs have finished
    ALIVE=()
    NEWLY_DONE=0
    for p in "${PIDS[@]}"; do
        if kill -0 "$p" 2>/dev/null; then
            ALIVE+=("$p")
        else
            NEWLY_DONE=$((NEWLY_DONE + 1))
            COMPLETED=$((COMPLETED + 1))
        fi
    done
    # Attribute the single exit code from wait -n to however many finished this round
    # (usually 1, but guard against edge cases)
    [ $finished_code -ne 0 ] && [ $NEWLY_DONE -gt 0 ] && FAILED=$((FAILED + 1))
    PIDS=("${ALIVE[@]}")

    # Immediately refill free slots
    while [ $NEXT -lt $TOTAL ] && [ ${#PIDS[@]} -lt $MAX_PARALLEL ]; do
        "${EXE}" "${SETUP_FILES[$NEXT]}" "${INSTANCE_FILES[$NEXT]}" "${RESULT_DIRS[$NEXT]}" &
        PIDS+=($!)
        NEXT=$((NEXT + 1))
    done

    # Progress report every 20 completions
    if [ $((COMPLETED % 20)) -eq 0 ] && [ $COMPLETED -gt 0 ]; then
        echo "Progress: $COMPLETED / $TOTAL completed | $(( NEXT - ${#PIDS[@]} )) launched | running: ${#PIDS[@]} | failed: $FAILED"
    fi

done

echo ""
echo "======================================================"
echo " All experiments finished!"
echo " Skipped:   $SKIPPED"
echo " Completed: $COMPLETED"
echo " Failed:    $FAILED"
echo "======================================================"
