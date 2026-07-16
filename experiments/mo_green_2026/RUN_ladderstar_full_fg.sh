#!/bin/bash
# LADDER* definitive run: the proposed method (TSN2-anchor + tuned levels,
# original budgets) on the FULL 82-instance benchmark. Writes into the same
# tree as the large-x20 run (results/EXP4_TUNED_TSN2ANCHOR/LADDER), whose 20
# instances are already complete -- the DONE markers skip them, so this run
# covers exactly the remaining 62 (~33 h at par=14). Resumable.
#
# Launch from PowerShell:
#   Start-Process wsl -ArgumentList 'bash',
#     '/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/RUN_ladderstar_full_fg.sh',
#     '14' -WindowStyle Hidden
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
PAR="${1:-14}"
tr -d '\r' < "$D/run_exp4_ladder.sh" > /tmp/exp4l_star.sh
bash -n /tmp/exp4l_star.sh || { echo "runner fails syntax"; exit 1; }
# Disk janitor: FuzzyFW dumps an unused ~20-30 MB _Scenarios.csv per run and
# per level; over 62x30 pipelines that is hundreds of GB. Purge every 90 s
# under this run's tree; dies with this script and sweeps once more on exit.
JDIR="$PWD/$D/results/EXP4_TUNED_TSN2ANCHOR"
( while :; do find "$JDIR" -name '*_Scenarios.csv' -delete 2>/dev/null; sleep 90; done ) &
JANITOR=$!
trap 'kill $JANITOR 2>/dev/null; find "$JDIR" -name "*_Scenarios.csv" -delete 2>/dev/null' EXIT
OUT_DIR="$PWD/$D/results/EXP4_TUNED_TSN2ANCHOR/LADDER" \
BASE_SETUP="$PWD/$D/setup/setup_LADDER_irace.txt" \
ANCHOR_SETUP="$PWD/$D/setup/setup_ANCHOR_tsn2.txt" \
LOG_FILE="$PWD/$D/ladderstar_full.log" \
  bash /tmp/exp4l_star.sh full "$PAR" \
  > "$D/ladderstar_full.out" 2>&1
find "$JDIR" -name '*_Scenarios.csv' -delete 2>/dev/null
echo "FINISHED rc=$?" >> "$D/ladderstar_full.out"
