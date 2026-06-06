#!/bin/bash
###############################################################################
# Run irace for each neighbourhood separately
# Results saved to experiments/irace/results/irace_{NB}/
###############################################################################

IRACE_DIR="$(cd "$(dirname "$0")" && pwd)"
RSCRIPT="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"

NEIGHBOURHOODS=(
    "ijsp.makespan.n1"
    "ijsp.makespan.n2"
    "ijsp.makespan.n3"
    "ijsp.makespan.next"
    "ijsp.makespan.n8"
)

NB_NAMES=( "N1" "N2" "N3" "Next" "N8" )

echo "=================================================="
echo " irace hyperparameter tuning — IJSP TabuSearch"
echo " Started: $(date)"
echo "=================================================="

for i in "${!NEIGHBOURHOODS[@]}"; do
    NB="${NEIGHBOURHOODS[$i]}"
    NAME="${NB_NAMES[$i]}"
    OUT_DIR="$IRACE_DIR/results/irace_${NAME}"

    echo ""
    echo "--- Tuning $NAME ($NB) ---"
    echo "    Output: $OUT_DIR"
    echo "    Started: $(date)"

    mkdir -p "$OUT_DIR"
    cp "$IRACE_DIR/scenario.txt"       "$OUT_DIR/"
    cp "$IRACE_DIR/parameters.txt"     "$OUT_DIR/"
    cp "$IRACE_DIR/instances.txt"      "$OUT_DIR/"
    cp "$IRACE_DIR/base_setup_ts_lex2.txt" "$OUT_DIR/"
    # Create a neighbourhood-specific target-runner.sh (NB hardcoded, no env var needed)
    sed "s|NB=\"\${IRACE_NEIGHBOURHOOD:-ijsp.makespan.n2}\"|NB=\"${NB}\"|" \
        "$IRACE_DIR/target-runner.sh" > "$OUT_DIR/target-runner.sh"
    cp "$IRACE_DIR/target-runner.cmd"  "$OUT_DIR/"
    cp "$IRACE_DIR/target-runner.ps1"  "$OUT_DIR/"
    chmod +x "$OUT_DIR/target-runner.sh"
    sed -i 's/\r$//' "$OUT_DIR/target-runner.sh"

    # Update logFile path in scenario
    sed -i "s|logFile.*|logFile = \"irace_${NAME}.log\"|" "$OUT_DIR/scenario.txt"

    # Run irace from the output directory
    cd "$OUT_DIR"
    "$RSCRIPT" -e "library(irace); irace_main(scenario=readScenario('scenario.txt'))" \
        > "irace_${NAME}_stdout.log" 2>&1

    EXIT_CODE=$?
    if [ $EXIT_CODE -eq 0 ]; then
        echo "    [OK] $NAME tuning complete at $(date)"
        # Extract best configuration
        "$RSCRIPT" -e "
            library(irace)
            load('irace_${NAME}.log')
            cat('Best configuration for ${NAME}:\n')
            print(getFinalElites(iraceResults, n=1))
        " >> "irace_${NAME}_stdout.log" 2>&1
    else
        echo "    [WARN] $NAME irace exited with code $EXIT_CODE"
    fi

    cd "$IRACE_DIR"
done

echo ""
echo "=================================================="
echo " All tuning runs complete: $(date)"
echo " Results in: $IRACE_DIR/results/"
echo "=================================================="
