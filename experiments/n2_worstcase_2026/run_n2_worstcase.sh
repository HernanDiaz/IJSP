#!/bin/bash
#
# run_n2_worstcase.sh
# Runs the N2Plus (worst-case only, G+) and N2Minus (best-case only, G-)
# neighbourhoods with the irace-tuned N2 config, on the SAME 82 instances /
# 30 runs / seed as the published N2 baseline (cor_tabu_2026), so the results
# are directly comparable to the paper's N2 rows in
#   experiments/cor_tabu_2026/statistical_results/runs_data.csv
#
# Only the neighbourhood differs from the paper's N2 config; hyperparameters
# are NOT re-tuned. This isolates the effect of restricting arc reversals to a
# single interval bound's critical path.
#
# Usage (from WSL):
#   bash run_n2_worstcase.sh <tier> [max_parallel]
#     tier        : smoke | classical | tai_small | full
#     max_parallel: concurrent jobs (default 3 — keep low to avoid host freeze)
#
#   tiers:
#     smoke     -> ft10 only            (2 jobs;  quick wiring validation)
#     classical -> abz/ft/la            (24 jobs; small, minutes each)
#     tai_small -> tai15/tai20          (60 jobs; small Taillard)
#     full      -> all 82 baseline      (164 jobs; includes tai50/tai100, hours each)
#
# Safe to re-run: a (config,instance) pair with a FINAL result CSV is skipped.
#
set -u

REPO="/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP"
EXE="/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW"
INSTANCES_DIR="${REPO}/SelectosYTaillardIntervalos"
EXP_DIR="${REPO}/experiments/n2_worstcase_2026"
SETUP_DIR="${EXP_DIR}/setup"
RESULTS_BASE="${EXP_DIR}/results"
BASELINE_CSV="${REPO}/experiments/cor_tabu_2026/statistical_results/runs_data.csv"
LOG_FILE="${EXP_DIR}/run.log"

TIER="${1:-smoke}"
MAX_PARALLEL="${2:-3}"
# tai100 has 2000 ops; 30 trials * 900 s + overhead can exceed 7.5 h. 18 h cap.
TIMEOUT=64800

CONFIGS=("N2Plus" "N2Minus")
SETUP_FILES_MAP=(
    "${SETUP_DIR}/setup_N2Plus_tuned.txt"
    "${SETUP_DIR}/setup_N2Minus_tuned.txt"
)

if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE  (build with: cd $REPO && make -j4)"; exit 1
fi
for sf in "${SETUP_FILES_MAP[@]}"; do
    [ -f "$sf" ] || { echo "ERROR: Setup file not found: $sf"; exit 1; }
done

# The canonical instance set = exactly the 82 instances the published N2 covers.
mapfile -t BASELINE_INSTANCES < <(awk -F, '$2=="n2"{print $4}' "$BASELINE_CSV" | sort -u)

# Filter by tier.
select_instances() {
    local tier="$1"; local inst
    for inst in "${BASELINE_INSTANCES[@]}"; do
        case "$tier" in
            smoke)     [[ "$inst" == *ft10* ]] && echo "$inst" ;;
            classical) [[ "$inst" == *abz* || "$inst" == *ft* || "$inst" == *la* ]] && echo "$inst" ;;
            tai_small) [[ "$inst" == tai15* || "$inst" == tai20* ]] && echo "$inst" ;;
            full)      echo "$inst" ;;
            *) echo "ERROR: unknown tier '$tier'" >&2; exit 1 ;;
        esac
    done
}
mapfile -t INSTANCES < <(select_instances "$TIER")

echo "======================================================" | tee "$LOG_FILE"
echo " N2 worst-case experiment  |  tier=$TIER  parallel=$MAX_PARALLEL" | tee -a "$LOG_FILE"
echo " Configs: ${CONFIGS[*]}   Instances: ${#INSTANCES[@]}   Started: $(date)" | tee -a "$LOG_FILE"
echo "======================================================" | tee -a "$LOG_FILE"
if [ "${#INSTANCES[@]}" -eq 0 ]; then echo "No instances selected for tier '$TIER'."; exit 1; fi

for cfg in "${CONFIGS[@]}"; do mkdir -p "${RESULTS_BASE}/${cfg}"; done
mkdir -p "${RESULTS_BASE}/logs"

# A pair is COMPLETE only if a FINAL CSV exists (STEM_<14digits>.csv, no
# _Sols/_Robustness/_Scenarios suffix). Build the pending queue.
declare -A DONE
for i in "${!CONFIGS[@]}"; do
    d="${RESULTS_BASE}/${CONFIGS[$i]}"
    while IFS= read -r s; do DONE["${CONFIGS[$i]}:${s}"]=1; done < <(
        ls "${d}"/*.csv 2>/dev/null | grep -vE '(_Sols|_Robustness|_Scenarios)\.csv$' \
        | grep -oP '[^/]+(?=_[0-9]{14}\.csv$)' | sort -u )
done

P_SETUP=(); P_INST=(); P_OUT=(); SKIPPED=0
for inst in "${INSTANCES[@]}"; do
    ipath="${INSTANCES_DIR}/${inst}.txt"
    [ -f "$ipath" ] || { echo "WARN: missing instance file $ipath" | tee -a "$LOG_FILE"; continue; }
    for i in "${!CONFIGS[@]}"; do
        if [[ -z "${DONE[${CONFIGS[$i]}:${inst}]+_}" ]]; then
            P_SETUP+=("${SETUP_FILES_MAP[$i]}"); P_INST+=("$ipath"); P_OUT+=("${RESULTS_BASE}/${CONFIGS[$i]}")
        else SKIPPED=$((SKIPPED+1)); fi
    done
done

TOTAL=${#P_SETUP[@]}
echo "Pending: $TOTAL  |  Already done: $SKIPPED" | tee -a "$LOG_FILE"
[ "$TOTAL" -eq 0 ] && { echo "Nothing to run."; exit 0; }

START=$(date +%s)
for i in "${!P_SETUP[@]}"; do
    while (( $(jobs -rp | wc -l) >= MAX_PARALLEL )); do wait -n 2>/dev/null || sleep 5; done
    setup="${P_SETUP[$i]}"; inst="${P_INST[$i]}"; outdir="${P_OUT[$i]}"
    iname="${inst##*/}"; iname="${iname%.txt}"
    cfg="${outdir##*/}"
    plog="${RESULTS_BASE}/logs/${cfg}__${iname}.log"
    echo "  [$(date '+%H:%M:%S')] launch ${cfg} / ${iname}  ($((i+1))/$TOTAL)" | tee -a "$LOG_FILE"
    timeout "$TIMEOUT" "$EXE" "$setup" "$inst" "$outdir" > "$plog" 2>&1 &
done
wait
echo "=== finished tier=$TIER in $(( $(date +%s)-START ))s  ($(date)) ===" | tee -a "$LOG_FILE"
