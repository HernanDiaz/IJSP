#!/bin/bash
###############################################################################
# run_irace_hc_crisp.sh
# Run irace for each neighbourhood — HillClimbing on CRISP instances
# Mirrors run_irace_hc.sh but uses SelectosYTaillardCrisp instances.
#
# Usage: bash run_irace_hc_crisp.sh
# Results saved to: experiments/irace_hc_crisp/results/irace_{NB}/
###############################################################################

IRACE_DIR="$(cd "$(dirname "$0")" && pwd)"
RSCRIPT="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"
R_USER_LIB="C:/Users/diazhernan/AppData/Local/R/win-library/4.5"

NEIGHBOURHOODS=(
    "ijsp.makespan.n1"
    "ijsp.makespan.n2"
    "ijsp.makespan.n3"
    "ijsp.makespan.next"
    "ijsp.makespan.n8"
)

NB_NAMES=( "N1" "N2" "N3" "Next" "N8" )

echo "=================================================="
echo " irace hyperparameter tuning — HC on CRISP instances"
echo " Started: $(date)"
echo "=================================================="

for i in "${!NEIGHBOURHOODS[@]}"; do
    NB="${NEIGHBOURHOODS[$i]}"
    NAME="${NB_NAMES[$i]}"
    OUT_DIR="$IRACE_DIR/results/irace_${NAME}"

    echo ""
    echo "--- Tuning $NAME ($NB) ---"
    echo "    Output: $OUT_DIR"

    # Skip if already completed successfully
    if grep -q "No test instances, skip testing" "$OUT_DIR/irace_${NAME}_stdout.log" 2>/dev/null; then
        echo "    [SKIP] $NAME already completed successfully, skipping."
        continue
    fi

    echo "    Started: $(date)"

    mkdir -p "$OUT_DIR"

    # Copy all config files
    cp "$IRACE_DIR/scenario.txt"            "$OUT_DIR/"
    cp "$IRACE_DIR/parameters.txt"          "$OUT_DIR/"
    cp "$IRACE_DIR/instances.txt"           "$OUT_DIR/"
    cp "$IRACE_DIR/base_setup_hc_lex2.txt"  "$OUT_DIR/"
    cp "$IRACE_DIR/target-runner.cmd"       "$OUT_DIR/"
    cp "$IRACE_DIR/target-runner.ps1"       "$OUT_DIR/"

    # Create neighbourhood-specific target-runner.sh (NB hardcoded)
    sed "s|NB=\"\${IRACE_NEIGHBOURHOOD:-ijsp.makespan.n2}\"|NB=\"${NB}\"|" \
        "$IRACE_DIR/target-runner.sh" > "$OUT_DIR/target-runner.sh"
    chmod +x "$OUT_DIR/target-runner.sh"
    sed -i 's/\r$//' "$OUT_DIR/target-runner.sh"

    # Update logFile in scenario
    sed -i "s|logFile.*|logFile = \"irace_${NAME}.log\"|" "$OUT_DIR/scenario.txt"

    # Run irace from the output directory
    cd "$OUT_DIR"
    echo ".libPaths(c('${R_USER_LIB}', .libPaths()))" > .Rprofile

    # Launch Rscript in background so we can watchdog it
    "$RSCRIPT" -e "library(irace); irace_main(scenario=readScenario('scenario.txt'))" \
        > "irace_${NAME}_stdout.log" 2>&1 &
    RSCRIPT_PID=$!

    # Watchdog: poll every 30s; if irace completion marker appears, kill stuck Rscript
    while kill -0 $RSCRIPT_PID 2>/dev/null; do
        if grep -q "No test instances, skip testing" "irace_${NAME}_stdout.log" 2>/dev/null; then
            echo "    [WATCHDOG] irace_${NAME} completed — terminating Rscript (PID $RSCRIPT_PID)"
            kill $RSCRIPT_PID 2>/dev/null
            sleep 2
            kill -9 $RSCRIPT_PID 2>/dev/null
            break
        fi
        sleep 30
    done
    wait $RSCRIPT_PID 2>/dev/null

    if grep -q "No test instances, skip testing" "irace_${NAME}_stdout.log" 2>/dev/null; then
        echo "    [OK] $NAME tuning complete at $(date)"
        # Extract best configuration
        "$RSCRIPT" -e "
            library(irace)
            load('irace_${NAME}.log')
            cat('Best configuration for ${NAME} (HC crisp):\n')
            print(getFinalElites(iraceResults, n=1))
        " >> "irace_${NAME}_stdout.log" 2>&1
    else
        echo "    [WARN] $NAME irace may not have completed — check irace_${NAME}_stdout.log"
    fi

    cd "$IRACE_DIR"
done

echo ""
echo "=================================================="
echo " All HC crisp tuning runs complete: $(date)"
echo " Results in: $IRACE_DIR/results/"
echo "=================================================="
