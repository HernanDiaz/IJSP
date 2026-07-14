#!/bin/bash
# M6 (review): neighbourhood ablation inside one Exp.4 arm on the large x20
# groups. Re-runs ABC-P with the PLAIN makespan neighbourhood N2 (instead of
# N2ME) on tai30_20 + tai50_20, everything else identical to the tuned arm
# (setup_ABCP_irace + algorithm/replacement overrides). Answers whether the
# large-x20 Exp.4 outcomes are mediated by N2ME's known large-instance
# pathology. Writes to results/EXP4_TUNED_ABL_N2/ABC-P — nothing touched.
#
# Launch from PowerShell (~11 h at par=14):
#   Start-Process wsl -ArgumentList 'bash',
#     '/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/RUN_ablation_abcp_n2_fg.sh',
#     '14' -WindowStyle Hidden
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
PAR="${1:-14}"; TIMEOUT=64800
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
OUT="$PWD/$D/results/EXP4_TUNED_ABL_N2"
LOG="$PWD/$D/ablation_abcp_n2.log"
mkdir -p "$OUT/ABC-P" "$OUT/logs"
tr -d '\r' < "$D/setup/setup_ABCP_irace.txt" \
  | sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
        -e 's/^replacement = .*/replacement = nsga2/' \
        -e 's/^localsearch.neighbourhood = .*/localsearch.neighbourhood = ijsp.makespan.n2/' \
  > "$OUT/s_abcp_n2.txt"

mapfile -t INSTS < <(ls SelectosYTaillardIntervalosEnergia/tai30_20_*.txt \
  SelectosYTaillardIntervalosEnergia/tai50_20_*.txt | sed 's|.*/||; s|\.txt$||')
echo "ABLATION ABC-P/N2 insts=${#INSTS[@]} par=$PAR $(date)" | tee "$LOG"
for inst in "${INSTS[@]}"; do
  done_csv=$(ls "$OUT/ABC-P/${inst}"_*.csv 2>/dev/null \
    | grep -vE "_(Sols|Robustness|Scenarios|Front)\.csv$" | head -1)
  [ -n "${done_csv:-}" ] && continue
  echo "  [$(date '+%H:%M:%S')] ABC-P(N2) / $inst" | tee -a "$LOG"
  timeout $TIMEOUT "$EXE" "$OUT/s_abcp_n2.txt" \
    "SelectosYTaillardIntervalosEnergia/$inst.txt" "$OUT/ABC-P/" \
    > "$OUT/logs/${inst}.log" 2>&1 &
  while [ "$(jobs -rp | wc -l)" -ge "$PAR" ]; do wait -n; done
done
wait
echo "ABLATION DONE $(date)" | tee -a "$LOG"
