#!/bin/bash
# M9 (review): strengthened-anchor ladder on the two large x20 groups, the
# regime where Exp.4 showed the anchor capping the front above the
# low-makespan corner. Same pipeline and tuned config (#70) as EXP4_TUNED,
# but the per-run budget is re-split in favour of the anchor:
#   tai30_20: anchor 650 s + levels {10,20,50,100} x 60 s   (~890 s total)
#   tai50_20: anchor 700 s + levels {10,50,100}    x 65 s   (~895 s total)
# Writes to results/EXP4_TUNED_STRONGANCHOR/LADDER — nothing touched.
#
# Launch from PowerShell (~11 h at par=14):
#   Start-Process wsl -ArgumentList 'bash',
#     '/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/RUN_ladder_stronganchor_fg.sh',
#     '14' -WindowStyle Hidden
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
PAR="${1:-14}"
# Derive the runner: patch the budget split and add a large20 tier.
tr -d '\r' < "$D/run_exp4_ladder.sh" \
  | sed -e 's|^    tai30\*) .*|    tai30*) echo "650 60 10 20 50 100";;|' \
        -e 's|^    tai50\*) .*|    tai50*) echo "700 65 10 50 100";;|' \
        -e 's#^  full) echo "\$i";;#  large20) [[ "$i" == tai30_20* || "$i" == tai50_20* ]] \&\& echo "$i";;\n  full) echo "$i";;#' \
  > /tmp/exp4l_strong.sh
# guard: derivation must produce a runner that contains the new tier
grep -q 'large20)' /tmp/exp4l_strong.sh || { echo "sed derivation failed"; exit 1; }
bash -n /tmp/exp4l_strong.sh || { echo "patched runner fails syntax"; exit 1; }
# Disk janitor: FuzzyFW always dumps a ~20-30 MB _Scenarios.csv per run/level
# (post-execution robustness, unused here and unstoppable without recompiling).
# Delete them every 90 s so they never accumulate. Dies with this script.
JDIR="$PWD/$D/results/EXP4_TUNED_STRONGANCHOR"
( while :; do find "$JDIR" -name '*_Scenarios.csv' -delete 2>/dev/null; sleep 90; done ) &
JANITOR=$!
trap 'kill $JANITOR 2>/dev/null' EXIT
OUT_DIR="$PWD/$D/results/EXP4_TUNED_STRONGANCHOR/LADDER" \
BASE_SETUP="$PWD/$D/setup/setup_LADDER_irace.txt" \
LOG_FILE="$PWD/$D/ladder_stronganchor.log" \
  bash /tmp/exp4l_strong.sh large20 "$PAR" \
  > "$D/ladder_stronganchor.out" 2>&1
find "$JDIR" -name '*_Scenarios.csv' -delete 2>/dev/null
echo "FINISHED rc=$?" >> "$D/ladder_stronganchor.out"
