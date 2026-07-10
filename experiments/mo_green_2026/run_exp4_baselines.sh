#!/bin/bash
# Exp. 4 — free Pareto baselines: ABC-P (ABCPSO-Pareto) and MA-P
# (MEMETIC-PARETO), random init, 30 runs x 900 s, N2ME LS. Tiered and
# resumable like run_lexme.sh. Usage: run_exp4_baselines.sh <tier> [par]
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
EXP="$REPO/experiments/mo_green_2026"
OUT="${OUT_DIR:-$EXP/results/EXP4}"
# Per-arm base setups (env-overridable for the tuned re-run; default =
# inherited shared setup, leaving the original EXP4 data untouched).
BASE_ABCP="${ABCP_SETUP:-$EXP/setup/setup_LexME_N2ME_tuned.txt}"
BASE_MAP="${MAP_SETUP:-$EXP/setup/setup_LexME_N2ME_tuned.txt}"
BASELINE_CSV="$REPO/experiments/cor_tabu_2026/statistical_results/runs_data.csv"
TIER="${1:-smoke}"; MAX="${2:-14}"; TIMEOUT=64800
LOG="${LOG_FILE:-$EXP/run_exp4_baselines.log}"

mkdir -p "$OUT/ABC-P" "$OUT/MA-P" "$OUT/logs"
sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
    -e 's/^replacement = .*/replacement = nsga2/' \
  "$BASE_ABCP" > "$OUT/s_abcp.txt"
sed -e 's/^algorithm = .*/algorithm = MEMETIC-PARETO/' \
    -e 's/^replacement = .*/replacement = nsga2/' \
  "$BASE_MAP" > "$OUT/s_map.txt"

mapfile -t ALL < <(awk -F, '$2=="n2"{print $4}' "$BASELINE_CSV" | sort -u)
sel() { for i in "${ALL[@]}"; do case "$1" in
  smoke) [[ "$i" == *ft10* ]] && echo "$i";;
  classical) [[ "$i" == *abz* || "$i" == *ft* || "$i" == *la* ]] && echo "$i";;
  tai_small) [[ "$i" == tai15* || "$i" == tai20* ]] && echo "$i";;
  full) echo "$i";; esac; done; }
mapfile -t INSTS < <(sel "$TIER")

echo "EXP4 baselines tier=$TIER par=$MAX insts=${#INSTS[@]} $(date)" | tee "$LOG"
for inst in "${INSTS[@]}"; do
  for arm in ABC-P MA-P; do
    s="$OUT/s_abcp.txt"; [ "$arm" = MA-P ] && s="$OUT/s_map.txt"
    done_csv=$(ls "$OUT/$arm/${inst}"_*.csv 2>/dev/null | grep -vE "_(Sols|Robustness|Scenarios|Front)\.csv$" | head -1)
    [ -n "${done_csv:-}" ] && continue
    while (( $(jobs -rp | wc -l) >= MAX )); do wait -n 2>/dev/null || sleep 5; done
    echo "  [$(date '+%H:%M:%S')] $arm / $inst" | tee -a "$LOG"
    timeout "$TIMEOUT" "$EXE" "$s" "SelectosYTaillardIntervalosEnergia/$inst.txt" \
      "$OUT/$arm/" > "$OUT/logs/${arm}__${inst}.log" 2>&1 &
  done
done
wait
echo "EXP4 baselines tier=$TIER DONE $(date)" | tee -a "$LOG"
