#!/bin/bash
#
# cleanup_incomplete.sh
# Re-runs instances that have Robustness/Sols files but no main CSV.
# These were killed mid-computation. Safe to re-run (will overwrite partial files).
#
# Usage: bash cleanup_incomplete.sh
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
IJSP_DIR="$(dirname "$SCRIPT_DIR")"
EXE="/c/Users/diazhernan/CLionProjects/FuzzyFW.exe"
SETUP_DIR="${SCRIPT_DIR}/setup"
INSTANCES_DIR="${IJSP_DIR}/SelectosYTaillardIntervalos"
RESULTS_BASE="${SCRIPT_DIR}/results"

NEIGHBOURHOODS=("n1" "n2" "n3" "nh")
COMPARATORS=("EV" "LEX1" "LEX2" "YX")

MAX_PARALLEL=28

echo "======================================================"
echo " IJSP Cleanup Runner (incomplete instances)"
echo "======================================================"

# Build list of incomplete experiments
SETUP_FILES=()
INSTANCE_FILES=()
RESULT_DIRS=()

for nb in "${NEIGHBOURHOODS[@]}"; do
    for comp in "${COMPARATORS[@]}"; do
        CONFIG="${nb}_${comp}"
        SETUP_FILE="${SETUP_DIR}/setup_${nb}_${comp}.txt"
        RESULTS_DIR="${RESULTS_BASE}/${CONFIG}"

        [ ! -f "$SETUP_FILE" ] && continue
        [ ! -d "$RESULTS_DIR" ] && continue

        for instance in "${INSTANCES_DIR}"/*.txt; do
            instance_name="$(basename "${instance%.txt}")"

            # Skip tai100 (handled separately)
            [[ "$instance_name" == tai100* ]] && continue

            # Check: has Robustness but no main CSV → incomplete
            has_main=$(ls "${RESULTS_DIR}/${instance_name}_"[0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9].csv 2>/dev/null | head -1)
            has_rob=$(ls "${RESULTS_DIR}/${instance_name}_"*Robustness.csv 2>/dev/null | head -1)

            if [ -z "$has_main" ] && [ -n "$has_rob" ]; then
                SETUP_FILES+=("$SETUP_FILE")
                INSTANCE_FILES+=("$instance")
                RESULT_DIRS+=("$RESULTS_DIR")
                echo "  INCOMPLETE: $CONFIG / $instance_name"
            fi
        done
    done
done

TOTAL=${#SETUP_FILES[@]}
echo ""
echo "Incomplete experiments found: $TOTAL"

if [ $TOTAL -eq 0 ]; then
    echo "Nothing to do. All experiments complete!"
    exit 0
fi

echo ""
echo "Starting cleanup with up to $MAX_PARALLEL parallel processes..."
echo ""

PIDS=()
COMPLETED=0
FAILED=0

for i in "${!SETUP_FILES[@]}"; do
    # Maintain pool of MAX_PARALLEL
    while [ ${#PIDS[@]} -ge $MAX_PARALLEL ]; do
        wait -n 2>/dev/null
        ALIVE=()
        for p in "${PIDS[@]}"; do
            if kill -0 "$p" 2>/dev/null; then
                ALIVE+=("$p")
            else
                wait "$p" 2>/dev/null
                exit_code=$?
                [ $exit_code -ne 0 ] && FAILED=$((FAILED + 1)) || true
                COMPLETED=$((COMPLETED + 1))
                echo "Progress: $COMPLETED / $TOTAL completed | running: ${#ALIVE[@]} | failed: $FAILED" \
                    >> "${SCRIPT_DIR}/cleanup_incomplete.log"
            fi
        done
        PIDS=("${ALIVE[@]}")
    done

    "$EXE" "${SETUP_FILES[$i]}" "${INSTANCE_FILES[$i]}" "${RESULT_DIRS[$i]}" \
        >> "${SCRIPT_DIR}/cleanup_incomplete.log" 2>&1 &
    PIDS+=($!)
done

# Wait for remaining
while [ ${#PIDS[@]} -gt 0 ]; do
    wait -n 2>/dev/null
    ALIVE=()
    for p in "${PIDS[@]}"; do
        if kill -0 "$p" 2>/dev/null; then
            ALIVE+=("$p")
        else
            wait "$p" 2>/dev/null
            exit_code=$?
            [ $exit_code -ne 0 ] && FAILED=$((FAILED + 1)) || true
            COMPLETED=$((COMPLETED + 1))
        fi
    done
    PIDS=("${ALIVE[@]}")
done

echo ""
echo "======================================================"
echo " Cleanup complete: $COMPLETED done, $FAILED failed"
echo "======================================================"
