#!/bin/bash
#
# run_exp8.sh
# Experiment 8: HC with irace-tuned configs on CRISP instances.
# 5 neighbourhoods x 92 instances x 30 runs.
# Skips already-completed experiments (safe to restart).
#
# Usage (from WSL): bash run_exp8.sh
#

EXE="/home/diazhernan/FuzzyFW"
SCRIPT_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments"
INSTANCES_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardCrisp"
SETUP_DIR="${SCRIPT_DIR}/setup/exp8"
RESULTS_BASE="${SCRIPT_DIR}/results/exp8"
LOG_FILE="/home/diazhernan/run_exp8.log"

MAX_PARALLEL=24

# Experiment 8 configurations: folder_name -> setup_file
CONFIGS=("N1_hc_crisp" "N2_hc_crisp" "N3_hc_crisp" "Next_hc_crisp" "N8_hc_crisp")
SETUP_FILES_MAP=(
    "${SETUP_DIR}/setup_N1_hc_crisp.txt"
    "${SETUP_DIR}/setup_N2_hc_crisp.txt"
    "${SETUP_DIR}/setup_N3_hc_crisp.txt"
    "${SETUP_DIR}/setup_Next_hc_crisp.txt"
    "${SETUP_DIR}/setup_N8_hc_crisp.txt"
)

echo "======================================================" | tee "$LOG_FILE"
echo " IJSP Experiment 8 — irace-tuned HC on CRISP instances" | tee -a "$LOG_FILE"
echo " EXE:          $EXE" | tee -a "$LOG_FILE"
echo " Instances:    $INSTANCES_DIR (82 crisp instances, tai100 excluded)" | tee -a "$LOG_FILE"
echo " Setup dir:    $SETUP_DIR" | tee -a "$LOG_FILE"
echo " Results base: $RESULTS_BASE" | tee -a "$LOG_FILE"
echo " Max parallel: $MAX_PARALLEL" | tee -a "$LOG_FILE"
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

# Create output directories
for cfg in "${CONFIGS[@]}"; do
    mkdir -p "${RESULTS_BASE}/${cfg}"
done
mkdir -p "${RESULTS_BASE}/exp8_logs"

# Read crisp instance paths (82 instances, excluding tai100 — same as exp7)
mapfile -t INSTANCE_PATHS < <(ls "${INSTANCES_DIR}"/*.txt 2>/dev/null | grep -v tai100 | sort)
INSTANCE_NAMES=()
for _p in "${INSTANCE_PATHS[@]}"; do
    _b="${_p##*/}"
    INSTANCE_NAMES+=("${_b%.txt}")
done

TOTAL_INSTANCES=${#INSTANCE_NAMES[@]}
echo "Instances: $TOTAL_INSTANCES  |  Configs: ${#CONFIGS[@]}" | tee -a "$LOG_FILE"

# Build pending list (interleaved: iterate instances outer, configs inner)
PENDING_SETUP=()
PENDING_INST=()
PENDING_OUTDIR=()
SKIPPED=0

# Pre-compute completed sets for each config
declare -A COMPLETED_SET

for i in "${!CONFIGS[@]}"; do
    folder="${CONFIGS[$i]}"
    RESULTS_DIR="${RESULTS_BASE}/${folder}"
    mapfile -t STEMS < <(
        ls "${RESULTS_DIR}"/*.csv 2>/dev/null \
        | grep -oP '[^/]+(?=_[0-9]{14}(_Sols|_Robustness)?\.csv$)' \
        | sort -u
    )
    for s in "${STEMS[@]}"; do
        COMPLETED_SET["${folder}:${s}"]=1
    done
done

# Build interleaved queue
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
echo "Pending: $TOTAL  |  Already done: $SKIPPED" | tee -a "$LOG_FILE"
echo "" | tee -a "$LOG_FILE"

if [ "$TOTAL" -eq 0 ]; then
    echo "Nothing to run. All experiments already completed." | tee -a "$LOG_FILE"
    exit 0
fi

# Run with bounded parallelism
RUNNING=0
COMPLETED=0
START_TIME=$(date +%s)

for i in "${!PENDING_SETUP[@]}"; do
    setup="${PENDING_SETUP[$i]}"
    inst="${PENDING_INST[$i]}"
    outdir="${PENDING_OUTDIR[$i]}"

    inst_name="${inst##*/}"; inst_name="${inst_name%.txt}"
    cfg_name="${setup##*/setup_}"; cfg_name="${cfg_name%_hc_crisp.txt}"
    per_log="${RESULTS_BASE}/exp8_logs/${cfg_name}__${inst_name}.log"
    timeout 27000 "$EXE" "$setup" "$inst" "$outdir" > "$per_log" 2>&1 &

    RUNNING=$((RUNNING + 1))
    if [ "$RUNNING" -ge "$MAX_PARALLEL" ]; then
        wait -n 2>/dev/null || wait
        RUNNING=$((RUNNING - 1))
        COMPLETED=$((COMPLETED + 1))
        if (( (COMPLETED % (MAX_PARALLEL * 2)) == 0 )); then
            ELAPSED=$(( $(date +%s) - START_TIME ))
            echo "  [$(date '+%H:%M:%S')] Completed ~$COMPLETED / $TOTAL  (${ELAPSED}s elapsed)" | tee -a "$LOG_FILE"
        fi
    fi
done

wait
ELAPSED=$(( $(date +%s) - START_TIME ))
echo "" | tee -a "$LOG_FILE"
echo "=== Experiment 8 finished ===" | tee -a "$LOG_FILE"
echo "Total time: ${ELAPSED}s  (~$(( ELAPSED/3600 ))h $(( (ELAPSED%3600)/60 ))m)" | tee -a "$LOG_FILE"
echo "Finished: $(date)" | tee -a "$LOG_FILE"
