#!/bin/bash
# M1 (review): re-run the makespan-only TS-N2 baseline IN THIS CAMPAIGN
# (same machine/load as the Exp.1/Exp.4 arms), so the "zero makespan cost"
# claim no longer rests on cross-campaign data. Same published tuned setup
# (cor_tabu_2026/setup_N2_tuned.txt, 30 runs x 900 s), run on the energy
# instances (power section is ignored by the makespan-only objective).
# Writes to results/BASELINE_RERUN — nothing existing is touched.
#
# Launch from PowerShell (keeps the WSL VM alive; ~44 h at par=14):
#   Start-Process wsl -ArgumentList 'bash',
#     '/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/RUN_baseline_rerun_fg.sh',
#     '14' -WindowStyle Hidden
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
PAR="${1:-14}"; TIMEOUT=64800
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
OUT="$PWD/$D/results/BASELINE_RERUN"
LOG="$PWD/$D/baseline_rerun.log"
mkdir -p "$OUT/logs"
SETUP="$PWD/experiments/cor_tabu_2026/setup/setup_N2_tuned.txt"
tr -d '\r' < "$SETUP" > "$OUT/s_n2.txt"

# Disk janitor: delete FuzzyFW's unused ~20-30 MB _Scenarios.csv dumps every
# 90 s so they never fill the disk (unstoppable without recompiling). Dies
# with this script. Critical here: 82 instances x 30 runs would otherwise be
# hundreds of GB of Scenarios.
( while :; do find "$OUT" -name '*_Scenarios.csv' -delete 2>/dev/null; sleep 90; done ) &
JANITOR=$!
trap 'kill $JANITOR 2>/dev/null; find "$OUT" -name "*_Scenarios.csv" -delete 2>/dev/null' EXIT

mapfile -t INSTS < <(ls SelectosYTaillardIntervalosEnergia/*.txt \
  | sed 's|.*/||; s|\.txt$||')
echo "BASELINE re-run insts=${#INSTS[@]} par=$PAR $(date)" | tee "$LOG"
for inst in "${INSTS[@]}"; do
  done_csv=$(ls "$OUT/${inst}"_*.csv 2>/dev/null \
    | grep -vE "_(Sols|Robustness|Scenarios|Front)\.csv$" | head -1)
  [ -n "${done_csv:-}" ] && continue
  echo "  [$(date '+%H:%M:%S')] TS-N2 / $inst" | tee -a "$LOG"
  timeout $TIMEOUT "$EXE" "$OUT/s_n2.txt" \
    "SelectosYTaillardIntervalosEnergia/$inst.txt" "$OUT/" \
    > "$OUT/logs/${inst}.log" 2>&1 &
  while [ "$(jobs -rp | wc -l)" -ge "$PAR" ]; do wait -n; done
done
wait
echo "BASELINE re-run DONE $(date)" | tee -a "$LOG"
