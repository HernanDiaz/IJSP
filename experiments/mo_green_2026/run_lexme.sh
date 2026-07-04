#!/bin/bash
#
# run_lexme.sh — Phase 2a of the MO green line: lexicographic Cmax->NPE
# (objective ijsp.makespan-energy, N2Plus LS, irace-tuned N2 params) on the
# energy-extended instances. 30 runs / 900 s, same protocol as the paper.
#
# The makespan-only baseline is NOT re-run: it is recovered post-hoc from the
# n2_worstcase_2026 N2Plus results (same tuned config and LS), recomputing NPE
# in Python from their _Sols orders (see analyze_lexme.py).
#
# Usage (from WSL):
#   bash run_lexme.sh <tier> [max_parallel]
#     tier: smoke | classical | tai_small | full   (same tiers as n2_worstcase)
#     max_parallel: default 3; up to 14 is safe on this machine for these sizes.
#
# Long runs: launch detached so WSL teardown does not kill the batch:
#   wsl bash -c "setsid nohup bash experiments/mo_green_2026/run_lexme.sh full 14 \
#     > experiments/mo_green_2026/run_full.log 2>&1 < /dev/null &"
#
# Safe to re-run: (instance) pairs with a FINAL result CSV are skipped.
#
set -u

REPO="/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP"
EXE="/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW"
INSTANCES_DIR="${REPO}/SelectosYTaillardIntervalosEnergia"
EXP_DIR="${REPO}/experiments/mo_green_2026"
SETUP="${EXP_DIR}/setup/setup_LexME_tuned.txt"
RESULTS_DIR="${EXP_DIR}/results/LexME"
BASELINE_CSV="${REPO}/experiments/cor_tabu_2026/statistical_results/runs_data.csv"
LOG_FILE="${EXP_DIR}/run_lexme.log"

TIER="${1:-smoke}"
MAX_PARALLEL="${2:-3}"
TIMEOUT=64800

if [ ! -f "$EXE" ]; then
    echo "ERROR: Executable not found: $EXE  (build with: cd $REPO && make -j4)"; exit 1
fi
[ -f "$SETUP" ] || { echo "ERROR: Setup file not found: $SETUP"; exit 1; }
if [ ! -d "$INSTANCES_DIR" ]; then
    echo "ERROR: $INSTANCES_DIR missing. Generate it with:"
    echo "  python3 ${EXP_DIR}/extend_instances.py"; exit 1
fi

# Canonical instance set = the 82 instances of the published N2 baseline.
mapfile -t BASELINE_INSTANCES < <(awk -F, '$2=="n2"{print $4}' "$BASELINE_CSV" | sort -u)

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
echo " LexME (Cmax->NPE lexicographic)  |  tier=$TIER  parallel=$MAX_PARALLEL" | tee -a "$LOG_FILE"
echo " Instances: ${#INSTANCES[@]}   Started: $(date)" | tee -a "$LOG_FILE"
echo "======================================================" | tee -a "$LOG_FILE"
[ "${#INSTANCES[@]}" -eq 0 ] && { echo "No instances for tier '$TIER'."; exit 1; }

mkdir -p "$RESULTS_DIR" "${EXP_DIR}/results/logs"

declare -A DONE
while IFS= read -r s; do DONE["$s"]=1; done < <(
    ls "${RESULTS_DIR}"/*.csv 2>/dev/null | grep -vE '(_Sols|_Robustness|_Scenarios)\.csv$' \
    | grep -oP '[^/]+(?=_[0-9]{14}\.csv$)' | sort -u )

P_INST=(); SKIPPED=0
for inst in "${INSTANCES[@]}"; do
    ipath="${INSTANCES_DIR}/${inst}.txt"
    [ -f "$ipath" ] || { echo "WARN: missing instance file $ipath" | tee -a "$LOG_FILE"; continue; }
    if [[ -z "${DONE[$inst]+_}" ]]; then P_INST+=("$ipath"); else SKIPPED=$((SKIPPED+1)); fi
done

TOTAL=${#P_INST[@]}
echo "Pending: $TOTAL  |  Already done: $SKIPPED" | tee -a "$LOG_FILE"
[ "$TOTAL" -eq 0 ] && { echo "Nothing to run."; exit 0; }

START=$(date +%s)
for i in "${!P_INST[@]}"; do
    while (( $(jobs -rp | wc -l) >= MAX_PARALLEL )); do wait -n 2>/dev/null || sleep 5; done
    inst="${P_INST[$i]}"
    iname="${inst##*/}"; iname="${iname%.txt}"
    plog="${EXP_DIR}/results/logs/LexME__${iname}.log"
    echo "  [$(date '+%H:%M:%S')] launch ${iname}  ($((i+1))/$TOTAL)" | tee -a "$LOG_FILE"
    timeout "$TIMEOUT" "$EXE" "$SETUP" "$inst" "$RESULTS_DIR" > "$plog" 2>&1 &
done
wait
echo "=== finished tier=$TIER in $(( $(date +%s)-START ))s  ($(date)) ===" | tee -a "$LOG_FILE"
