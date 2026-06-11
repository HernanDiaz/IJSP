#!/bin/bash
###############################################################################
# Run irace for IPRTS (single configuration space, no neighbourhood loop).
# Results in experiments/iprts_2026/irace/results/irace_IPRTS/.
# LAUNCH DETACHED on this machine (background tasks die at ~60 min):
#   wsl bash -c "setsid nohup bash <this script> > <log> 2>&1 < /dev/null &"
###############################################################################

IRACE_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$IRACE_DIR/../../.." && pwd)"
RSCRIPT="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"
OUT_DIR="$IRACE_DIR/results/irace_IPRTS"

echo "=== irace IPRTS tuning — started $(date)"

# Fresh native-filesystem copy of the current binary (faster startup than /mnt/c)
cp "$REPO/../FuzzyFW" /home/diazhernan/FuzzyFW_iprts
chmod +x /home/diazhernan/FuzzyFW_iprts

mkdir -p "$OUT_DIR"
cp "$IRACE_DIR/scenario.txt" "$IRACE_DIR/parameters.txt" "$IRACE_DIR/instances.txt" \
   "$IRACE_DIR/base_setup_iprts.txt" "$IRACE_DIR/target-runner.sh" \
   "$IRACE_DIR/target-runner.cmd" "$IRACE_DIR/target-runner.ps1" "$OUT_DIR/"
chmod +x "$OUT_DIR/target-runner.sh"
sed -i 's/\r$//' "$OUT_DIR/target-runner.sh"

# The Windows user library (where irace lives) is not on .libPaths() when R
# is launched from a non-login WSL shell: pin it explicitly.
RLIB="C:/Users/diazhernan/AppData/Local/R/win-library/4.5"

cd "$OUT_DIR"
"$RSCRIPT" -e ".libPaths(c('$RLIB', .libPaths())); library(irace); irace_main(scenario=readScenario('scenario.txt'))" \
    > irace_IPRTS_stdout.log 2>&1
EXIT_CODE=$?

if [ $EXIT_CODE -eq 0 ]; then
    "$RSCRIPT" -e "
        .libPaths(c('$RLIB', .libPaths()))
        library(irace)
        load('irace_IPRTS.log')
        cat('Best configurations for IPRTS:\n')
        print(getFinalElites(iraceResults, n=3))
    " >> irace_IPRTS_stdout.log 2>&1
    echo "=== irace IPRTS tuning COMPLETE $(date)"
else
    echo "=== irace exited with code $EXIT_CODE at $(date)"
fi
