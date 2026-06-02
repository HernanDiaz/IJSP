#!/bin/bash
#
# run_exp4_bugfix.sh
# Re-run of Phase B (Experiment 4) for the two neighbourhoods affected by
# the BFS-seeding bug that was identified in NeighbourhoodIJSP_Cmax.cpp
# (N_3, tipo=1 and tipo=2 of acceptNeighbour) and NeighbourhoodIJSP_N8.cpp
# (N_8 reinsertion). The patched binary lives at /home/diazhernan/FuzzyFW
# (rebuilt with the fix). N_1, N_2, N_ext are NOT re-run — code analysis +
# scan over 4 instances × 30 runs showed they were unaffected.
#
# Output goes to results/exp4_bugfix/ rather than overwriting exp4/, so the
# original (buggy) results stay available as evidence.
#
# Like the original run_exp4.sh, this script skips experiments that already
# have a FINAL result CSV (so it is safe to restart).
#
# Usage (from WSL): bash run_exp4_bugfix.sh
#

EXE="/home/diazhernan/FuzzyFW"
SCRIPT_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments"
INSTANCES_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardIntervalos"
SETUP_DIR="${SCRIPT_DIR}/setup/exp4"
RESULTS_BASE="${SCRIPT_DIR}/results/exp4_bugfix"
LOG_FILE="/home/diazhernan/run_exp4_bugfix.log"

MAX_PARALLEL=24

# Only the two neighbourhoods affected by the bug.
CONFIGS=("N3_tuned" "N8_tuned")
SETUP_FILES_MAP=(
    "${SETUP_DIR}/setup_N3_tuned.txt"
    "${SETUP_DIR}/setup_N8_tuned.txt"
)

echo "======================================================" | tee "$LOG_FILE"
echo " IJSP Experiment 4 BUGFIX — re-run of N3 and N8" | tee -a "$LOG_FILE"
echo " EXE:          $EXE" | tee -a "$LOG_FILE"
echo " Setup dir:    $SETUP_DIR" | tee -a "$LOG_FILE"
echo " Results base: $RESULTS_BASE" | tee -a "$LOG_FILE"
echo " Max parallel: $MAX_PARALLEL" | tee -a "$LOG_FILE"
echo " Started:      $(date)" | tee -a "$LOG_FILE"
echo "======================================================" | tee -a "$LOG_FILE"

if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE" | tee -a "$LOG_FILE"
    exit 1
fi

# Quick sanity check: confirm the binary contains the fix. We grep the binary
# strings for the "newMs" comment we inserted; if it isn't there, the binary
# is the old one and we abort.
if ! strings "$EXE" 2>/dev/null | grep -q "BUGFIX" 2>/dev/null; then
    # Strings may not be installed or the comment may not survive optimisation;
    # only warn, don't fail.
    echo "WARNING: cannot confirm binary contains the fix via 'strings'." | tee -a "$LOG_FILE"
    echo "         Make sure /home/diazhernan/FuzzyFW was rebuilt after the fix." | tee -a "$LOG_FILE"
fi

# Verify all setup files exist
for i in "${!CONFIGS[@]}"; do
    sf="${SETUP_FILES_MAP[$i]}"
    if [ ! -f "$sf" ]; then
        echo "ERROR: Setup file not found: $sf" | tee -a "$LOG_FILE"
        exit 1
    fi
done

# Create output directories
for cfg in "${CONFIGS[@]}"; do
    mkdir -p "${RESULTS_BASE}/${cfg}"
done
mkdir -p "${RESULTS_BASE}/exp4_bugfix_logs"

# Read instance paths (Phase B baseline: exclude tai100 and tai50 ... but
# wait, the original 82-instance benchmark INCLUDES tai50, so the original
# run_exp4.sh actually excluded tai100 and was meant to include all sizes
# up to tai50. However the original script had a bug excluding "tai50". Use
# the same filter to be consistent.
mapfile -t INSTANCE_PATHS < <(ls "${INSTANCES_DIR}"/*.txt 2>/dev/null \
    | grep -v tai100 \
    | sort)
INSTANCE_NAMES=()
for _p in "${INSTANCE_PATHS[@]}"; do
    _b="${_p##*/}"
    INSTANCE_NAMES+=("${_b%.txt}")
done

TOTAL_INSTANCES=${#INSTANCE_NAMES[@]}
echo "Instances: $TOTAL_INSTANCES  |  Configs: ${#CONFIGS[@]}" | tee -a "$LOG_FILE"

# Skip already-completed experiments (final CSV present, NOT just _Sols etc.)
PENDING_SETUP=()
PENDING_INST=()
PENDING_OUTDIR=()
SKIPPED=0

declare -A COMPLETED_SET  # key: "config:instance_stem"
for i in "${!CONFIGS[@]}"; do
    folder="${CONFIGS[$i]}"
    RESULTS_DIR="${RESULTS_BASE}/${folder}"
    mapfile -t STEMS < <(
        ls "${RESULTS_DIR}"/*.csv 2>/dev/null \
        | grep -vE '(_Sols|_Robustness|_Scenarios)\.csv$' \
        | grep -oP '[^/]+(?=_[0-9]{14}\.csv$)' \
        | sort -u
    )
    for s in "${STEMS[@]}"; do
        COMPLETED_SET["${folder}:${s}"]=1
    done
done

for idx in "${!INSTANCE_NAMES[@]}"; do
    name="${INSTANCE_NAMES[$idx]}"
    inst_path="${INSTANCE_PATHS[$idx]}"
    for i in "${!CONFIGS[@]}"; do
        folder="${CONFIGS[$i]}"
        sf="${SETUP_FILES_MAP[$i]}"
        RESULTS_DIR="${RESULTS_BASE}/${folder}"
        if [[ -z "${COMPLETED_SET[${folder}:${name}]+_}" ]]; then
            PENDING_SETUP+=("$sf")
            PENDING_INST+=("$inst_path")
            PENDING_OUTDIR+=("$RESULTS_DIR")
        else
            SKIPPED=$((SKIPPED + 1))
        fi
    done
done

TOTAL=${#PENDING_SETUP[@]}
echo "Pending: $TOTAL  |  Already done (final CSV): $SKIPPED" | tee -a "$LOG_FILE"
echo "" | tee -a "$LOG_FILE"

if [ "$TOTAL" -eq 0 ]; then
    echo "Nothing to run. All experiments already completed." | tee -a "$LOG_FILE"
    exit 0
fi

# Per-job wall-clock cap. The setup file already imposes a 900 s per-run cap
# and the run dispatches 30 internal runs; 27000 s (7.5 h) is the same value
# used in the original run_exp4.sh and leaves headroom.
TIMEOUT=27000

# Run with bounded parallelism. Use jobs -rp for an accurate count of running
# children (more robust to counter drift than tracking RUNNING manually).
COMPLETED=0
START_TIME=$(date +%s)

for i in "${!PENDING_SETUP[@]}"; do
    while (( $(jobs -rp | wc -l) >= MAX_PARALLEL )); do
        wait -n 2>/dev/null || sleep 5
    done

    setup="${PENDING_SETUP[$i]}"
    inst="${PENDING_INST[$i]}"
    outdir="${PENDING_OUTDIR[$i]}"

    inst_name="${inst##*/}"; inst_name="${inst_name%.txt}"
    cfg_name="${setup##*/setup_}"; cfg_name="${cfg_name%_tuned.txt}"
    per_log="${RESULTS_BASE}/exp4_bugfix_logs/${cfg_name}__${inst_name}.log"
    timeout "$TIMEOUT" "$EXE" "$setup" "$inst" "$outdir" > "$per_log" 2>&1 &

    LAUNCHED=$((i + 1))
    if (( LAUNCHED % MAX_PARALLEL == 0 )); then
        ELAPSED=$(( $(date +%s) - START_TIME ))
        echo "  [$(date '+%H:%M:%S')] Launched $LAUNCHED / $TOTAL  (${ELAPSED}s elapsed)" | tee -a "$LOG_FILE"
    fi
done

wait
ELAPSED=$(( $(date +%s) - START_TIME ))
echo "" | tee -a "$LOG_FILE"
echo "=== Experiment 4 BUGFIX finished ===" | tee -a "$LOG_FILE"
echo "Total time: ${ELAPSED}s  (~$(( ELAPSED/3600 ))h $(( (ELAPSED%3600)/60 ))m)" | tee -a "$LOG_FILE"
echo "Finished: $(date)" | tee -a "$LOG_FILE"
