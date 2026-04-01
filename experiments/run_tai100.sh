#!/bin/bash
#
# run_tai100.sh
# Runs all 20 x 10 = 200 IJSP experiments for tai100_20 instances.
# Skips experiments whose result file already exists (safe to restart).
#
# Usage: bash run_tai100.sh
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IJSP_DIR="$(dirname "$SCRIPT_DIR")"
EXE="/c/Users/diazhernan/CLionProjects/FuzzyFW.exe"
SETUP_DIR="${SCRIPT_DIR}/setup"
INSTANCES_DIR="${IJSP_DIR}/SelectosYTaillardIntervalos"
RESULTS_BASE="${SCRIPT_DIR}/results"

NEIGHBOURHOODS=("n1" "n2" "n3" "nh" "next")
COMPARATORS=("EV" "LEX1" "LEX2" "YX")

MAX_PARALLEL=14

echo "======================================================"
echo " IJSP Makespan Experiment Runner — tai100 instances"
echo " EXE:       $EXE"
echo " Instances: $INSTANCES_DIR"
echo " Results:   $RESULTS_BASE"
echo " Max parallel: $MAX_PARALLEL"
echo "======================================================"

if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE"
    exit 1
fi

SETUP_FILES=()
INSTANCE_FILES=()
RESULT_DIRS=()
SKIPPED=0

mapfile -t INSTANCE_PATHS < <(ls "${INSTANCES_DIR}"/tai100*.txt 2>/dev/null)
INSTANCE_NAMES=()
for _p in "${INSTANCE_PATHS[@]}"; do
    _b="${_p##*/}"
    INSTANCE_NAMES+=("${_b%.txt}")
done

echo "Found ${#INSTANCE_NAMES[@]} tai100 instances: ${INSTANCE_NAMES[*]}"
echo ""

for nb in "${NEIGHBOURHOODS[@]}"; do
    for comp in "${COMPARATORS[@]}"; do
        SETUP_FILE="${SETUP_DIR}/setup_${nb}_${comp}.txt"
        RESULTS_DIR="${RESULTS_BASE}/${nb}_${comp}"
        mkdir -p "$RESULTS_DIR"

        declare -A done_set
        while IFS= read -r fname; do
            key="${fname%_??????????????.csv}"
            done_set["$key"]=1
        done < <(ls "${RESULTS_DIR}/" 2>/dev/null | grep -E '^tai100.*_[0-9]{14}\.csv$')

        for i in "${!INSTANCE_NAMES[@]}"; do
            instance_name="${INSTANCE_NAMES[$i]}"
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
    echo "All tai100 experiments already completed."
    exit 0
fi

PIDS=()
NEXT=0
COMPLETED=0
FAILED=0

while [ $NEXT -lt $TOTAL ] && [ ${#PIDS[@]} -lt $MAX_PARALLEL ]; do
    "${EXE}" "${SETUP_FILES[$NEXT]}" "${INSTANCE_FILES[$NEXT]}" "${RESULT_DIRS[$NEXT]}" &
    PIDS+=($!)
    NEXT=$((NEXT + 1))
done
echo "Launched initial batch of ${#PIDS[@]} processes."

while [ ${#PIDS[@]} -gt 0 ]; do

    wait -n 2>/dev/null
    finished_code=$?

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
    [ $finished_code -ne 0 ] && [ $NEWLY_DONE -gt 0 ] && FAILED=$((FAILED + 1))
    PIDS=("${ALIVE[@]}")

    while [ $NEXT -lt $TOTAL ] && [ ${#PIDS[@]} -lt $MAX_PARALLEL ]; do
        "${EXE}" "${SETUP_FILES[$NEXT]}" "${INSTANCE_FILES[$NEXT]}" "${RESULT_DIRS[$NEXT]}" &
        PIDS+=($!)
        NEXT=$((NEXT + 1))
    done

    if [ $((COMPLETED % 10)) -eq 0 ] && [ $COMPLETED -gt 0 ]; then
        echo "Progress: $COMPLETED / $TOTAL completed | running: ${#PIDS[@]} | failed: $FAILED"
    fi

done

echo ""
echo "======================================================"
echo " tai100 experiments finished!"
echo " Skipped:   $SKIPPED"
echo " Completed: $COMPLETED"
echo " Failed:    $FAILED"
echo "======================================================"
