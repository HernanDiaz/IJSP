#!/bin/bash
# Review follow-up: TSN2-ANCHOR ladder on the two large x20 groups. The M9
# strong-anchor run recovered tai50_20 but NOT tai30_20, because there the
# anchor was already converged at the original budget -- the +1.8% makespan
# gap is a CONFIG limitation, not a time one. This run swaps only the anchor
# phase to the makespan-strong TSN2 config (JOX/insertion/100% LS/plain N2/
# 24 trials, via setup_ANCHOR_tsn2.txt), keeping the ORIGINAL budgets so the
# effect is attributable to the config, not extra time. Levels stay on the
# tuned #70 config. Writes to results/EXP4_TUNED_TSN2ANCHOR/LADDER.
#
# Launch from PowerShell (~11 h at par=14):
#   Start-Process wsl -ArgumentList 'bash',
#     '/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/RUN_ladder_tsn2anchor_fg.sh',
#     '14' -WindowStyle Hidden
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
PAR="${1:-14}"
# Derive the runner: add a large20 tier (original budgets, unchanged).
tr -d '\r' < "$D/run_exp4_ladder.sh" \
  | sed 's#^  full) echo "\$i";;#  large20) [[ "$i" == tai30_20* || "$i" == tai50_20* ]] \&\& echo "$i";;\n  full) echo "$i";;#' \
  > /tmp/exp4l_tsn2full.sh
grep -q 'large20)' /tmp/exp4l_tsn2full.sh || { echo "sed derivation failed"; exit 1; }
bash -n /tmp/exp4l_tsn2full.sh || { echo "patched runner fails syntax"; exit 1; }
# Disk janitor (see RUN_ladder_stronganchor_fg.sh): delete FuzzyFW's unused
# ~20-30 MB _Scenarios.csv dumps every 90 s. Dies with this script.
JDIR="$PWD/$D/results/EXP4_TUNED_TSN2ANCHOR"
( while :; do find "$JDIR" -name '*_Scenarios.csv' -delete 2>/dev/null; sleep 90; done ) &
JANITOR=$!
trap 'kill $JANITOR 2>/dev/null' EXIT
OUT_DIR="$PWD/$D/results/EXP4_TUNED_TSN2ANCHOR/LADDER" \
BASE_SETUP="$PWD/$D/setup/setup_LADDER_irace.txt" \
ANCHOR_SETUP="$PWD/$D/setup/setup_ANCHOR_tsn2.txt" \
LOG_FILE="$PWD/$D/ladder_tsn2anchor.log" \
  bash /tmp/exp4l_tsn2full.sh large20 "$PAR" \
  > "$D/ladder_tsn2anchor.out" 2>&1
find "$JDIR" -name '*_Scenarios.csv' -delete 2>/dev/null
echo "FINISHED rc=$?" >> "$D/ladder_tsn2anchor.out"
