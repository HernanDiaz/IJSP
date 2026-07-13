#!/bin/bash
# FOREGROUND runner for the TUNED baselines (ABC-P #118, MA-P #125).
# Blocks until the whole tier finishes -> keeps the WSL VM alive for the
# entire run. Launch it from PowerShell via:
#   Start-Process wsl -ArgumentList 'bash',
#     '/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/RUN_exp4tuned_baselines_fg.sh',
#     'full','14' -WindowStyle Hidden
# Start-Process detaches it from the terminal at the Windows level, so closing
# the window does NOT kill it (the wsl.exe process keeps the VM up).
# Writes to results/EXP4_TUNED -- inherited EXP4 data untouched.
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
TIER="${1:-classical}"; PAR="${2:-14}"
tr -d '\r' < "$D/run_exp4_baselines.sh" > /tmp/exp4b_tuned_fg.sh
export OUT_DIR="$PWD/$D/results/EXP4_TUNED"
export ABCP_SETUP="$PWD/$D/setup/setup_ABCP_irace.txt"
export MAP_SETUP="$PWD/$D/setup/setup_MAP_irace.txt"
export LOG_FILE="$PWD/$D/exp4tuned_baselines_${TIER}.log"
# foreground; tee stdout to .out so progress is visible even after detach
bash /tmp/exp4b_tuned_fg.sh "$TIER" "$PAR" > "$D/exp4tuned_baselines_${TIER}.out" 2>&1
echo "FINISHED tier=$TIER rc=$?" >> "$D/exp4tuned_baselines_${TIER}.out"
