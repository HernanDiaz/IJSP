#!/bin/bash
#
# run_phaseB_tai100.sh
# Phase B (irace-tuned tabu search) on the tai100_20 instances.
# Each setup file encodes 30 runs per (instance, neighbourhood).
#
# Uses the IDENTICAL irace-tuned setup files from exp4/, so the
# hyperparameters are exactly those reported in Table 'tab:tuned' of the paper.
#
# Skips experiments that already have a FINAL result CSV (safe to restart).
# A job that only produced _Robustness/_Sols (i.e. was killed before writing
# its final CSV) is NOT considered complete and will be re-run.
#
# Usage (from WSL): bash run_phaseB_tai100.sh
#

EXE="/home/diazhernan/FuzzyFW"
SCRIPT_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments"
INSTANCES_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardIntervalos"
SETUP_DIR="${SCRIPT_DIR}/setup/exp4"
RESULTS_BASE="${SCRIPT_DIR}/results/exp4"
LOG_FILE="/home/diazhernan/run_phaseB_tai100.log"

# Keep the live job count at or below the 14 physical cores (28 logical)
# to avoid the over-subscription that slowed the original 24-way run. With
# N1-N8 already complete, only the 10 Next jobs remain, so all run at once.
MAX_PARALLEL=15

# Per-process wall-clock cap. tai100 has 2000 operations, so 30 trials of
# 900 s each plus overhead can exceed 7.5 h; 18 h leaves a safe margin while
# still catching a genuinely hung process. N_ext is the slowest neighbourhood
# (its viability check scans interior arcs of very large critical blocks),
# which is why it needs this larger cap than the original 7.5 h run allowed.
TIMEOUT=64800

# Phase B configurations (all five). The skip-by-final-CSV logic below means
# re-running this script only launches the still-incomplete jobs: N1-N8 are
# already done on tai100, so this run completes the 10 Next_tuned jobs.
CONFIGS=("N1_tuned" "N2_tuned" "N3_tuned" "Next_tuned" "N8_tuned")
SETUP_FILES_MAP=(
    "${SETUP_DIR}/setup_N1_tuned.txt"
    "${SETUP_DIR}/setup_N2_tuned.txt"
    "${SETUP_DIR}/setup_N3_tuned.txt"
    "${SETUP_DIR}/setup_Next_tuned.txt"
    "${SETUP_DIR}/setup_N8_tuned.txt"
)

echo "======================================================" | tee "$LOG_FILE"
echo " IJSP Phase B — tai100 instances (irace-tuned TS)" | tee -a "$LOG_FILE"
echo " EXE:          $EXE" | tee -a "$LOG_FILE"
echo " Setup dir:    $SETUP_DIR" | tee -a "$LOG_FILE"
echo " Results base: $RESULTS_BASE" | tee -a "$LOG_FILE"
echo " Max parallel: $MAX_PARALLEL" | tee -a "$LOG_FILE"
echo " Timeout/job:  ${TIMEOUT}s" | tee -a "$LOG_FILE"
echo " Configs:      ${CONFIGS[*]}" | tee -a "$LOG_FILE"
echo " Started:      $(date)" | tee -a "$LOG_FILE"
echo "======================================================" | tee -a "$LOG_FILE"

if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE" | tee -a "$LOG_FILE"
    exit 1
fi

# Verify all setup files exist
for i in "${!CONFIGS[@]}"; do
    sf="${SETUP_FILES_MAP[$i]}"
    if [ ! -f "$sf" ]; then
        echo "ERROR: Setup file not found: $sf" | tee -a "$LOG_FILE"
        exit 1
    fi
done

# Output directories (shared with exp4 so all Phase B results live together)
for cfg in "${CONFIGS[@]}"; do
    mkdir -p "${RESULTS_BASE}/${cfg}"
done
mkdir -p "${RESULTS_BASE}/exp4_logs"

# Read ONLY tai100 instance paths
mapfile -t INSTANCE_PATHS < <(ls "${INSTANCES_DIR}"/tai100*.txt 2>/dev/null | sort)
INSTANCE_NAMES=()
for _p in "${INSTANCE_PATHS[@]}"; do
    _b="${_p##*/}"
    INSTANCE_NAMES+=("${_b%.txt}")
done

TOTAL_INSTANCES=${#INSTANCE_NAMES[@]}
echo "Instances: $TOTAL_INSTANCES  |  Configs: ${#CONFIGS[@]}" | tee -a "$LOG_FILE"

if [ "$TOTAL_INSTANCES" -eq 0 ]; then
    echo "ERROR: No tai100 instances found in $INSTANCES_DIR" | tee -a "$LOG_FILE"
    exit 1
fi

# Build pending list. A (config,instance) pair is COMPLETE only if a FINAL
# result CSV exists: STEM_<14digits>.csv with no _Sols/_Robustness/_Scenarios
# suffix. Partial outputs (only _Robustness/_Sols) do NOT count as complete.
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

# Build interleaved queue: instances outer, configs inner
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
    echo "Nothing to run. All tai100 experiments already completed." | tee -a "$LOG_FILE"
    exit 0
fi

# Run with bounded parallelism. Throttle on the ACTUAL number of running
# background jobs (jobs -rp), which is robust against the counter drift that
# can occur when several children finish between two `wait` calls.
COMPLETED=0
START_TIME=$(date +%s)

for i in "${!PENDING_SETUP[@]}"; do
    # Wait until a slot is genuinely free
    while (( $(jobs -rp | wc -l) >= MAX_PARALLEL )); do
        wait -n 2>/dev/null || sleep 5
    done

    setup="${PENDING_SETUP[$i]}"
    inst="${PENDING_INST[$i]}"
    outdir="${PENDING_OUTDIR[$i]}"

    inst_name="${inst##*/}"; inst_name="${inst_name%.txt}"
    cfg_name="${setup##*/setup_}"; cfg_name="${cfg_name%_tuned.txt}"
    per_log="${RESULTS_BASE}/exp4_logs/${cfg_name}__${inst_name}.log"
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
echo "=== Phase B tai100 finished ===" | tee -a "$LOG_FILE"
echo "Total time: ${ELAPSED}s  (~$(( ELAPSED/3600 ))h $(( (ELAPSED%3600)/60 ))m)" | tee -a "$LOG_FILE"
echo "Finished: $(date)" | tee -a "$LOG_FILE"
