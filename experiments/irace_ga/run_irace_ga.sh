#!/bin/bash
###############################################################################
# Tune the pure GA (makespan, LEX2) SEPARATELY for each mutation operator.
# Tuned: population.size, crossover (jox/gox/ppx), crossover.probability,
#        mutation.probability, selection.tournament-size (+ n8.moves for N8).
# Fixed: noImprovement=25, timelimit=120, LEX2 ranking.
#
# Launch from WSL (irace runs on Windows R, called via the .cmd/.ps1 bridge):
#   bash experiments/irace_ga/run_irace_ga.sh
# Detached:
#   setsid nohup bash experiments/irace_ga/run_irace_ga.sh > experiments/irace_ga/launch.log 2>&1 < /dev/null &
###############################################################################
set -u
IRACE_DIR="$(cd "$(dirname "$0")" && pwd)"
RSCRIPT="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"
COR_IRACE="/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/cor_tabu_2026/irace"

# Native binary copy in the WSL home (much faster than running off /mnt/c
# for the thousands of short irace experiments).
cp "/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW" /home/diazhernan/FuzzyFW_ga
chmod +x /home/diazhernan/FuzzyFW_ga

MUTATIONS=( "ijsp.n8"           "swap"                   "insertion"              "inversion" )
NAMES=(     "N8"                "Swap"                   "Insertion"              "Inversion" )
PARAMS=(    "parameters_n8.txt" "parameters_generic.txt" "parameters_generic.txt" "parameters_generic.txt" )

echo "=== GA mutation-operator tuning start $(date) ==="

for i in "${!MUTATIONS[@]}"; do
    MUT="${MUTATIONS[$i]}"
    NAME="${NAMES[$i]}"
    PFILE="${PARAMS[$i]}"
    OUT_DIR="$IRACE_DIR/results/irace_${NAME}"

    echo ""
    echo "--- Tuning GA + ${NAME} mutation (${MUT}) --- $(date)"
    mkdir -p "$OUT_DIR"
    cp "$IRACE_DIR/scenario.txt"       "$OUT_DIR/"
    cp "$IRACE_DIR/$PFILE"             "$OUT_DIR/parameters.txt"
    cp "$IRACE_DIR/instances.txt"      "$OUT_DIR/"
    cp "$IRACE_DIR/base_setup_ga.txt"  "$OUT_DIR/"

    # target-runner.sh with the mutation operator hardcoded
    sed "s|MUT=\"MUT_OPERATOR\"|MUT=\"${MUT}\"|" \
        "$IRACE_DIR/target-runner.sh" > "$OUT_DIR/target-runner.sh"
    cp "$COR_IRACE/target-runner.cmd"  "$OUT_DIR/"
    cp "$COR_IRACE/target-runner.ps1"  "$OUT_DIR/"
    chmod +x "$OUT_DIR/target-runner.sh"
    sed -i 's/\r$//' "$OUT_DIR/target-runner.sh"

    sed -i "s|logFile.*|logFile = \"irace_${NAME}.log\"|" "$OUT_DIR/scenario.txt"

    cd "$OUT_DIR"
    "$RSCRIPT" -e "library(irace); irace_main(scenario=readScenario('scenario.txt'))" \
        > "irace_${NAME}_stdout.log" 2>&1
    echo "    [done] ${NAME} at $(date)  (best config -> tail irace_${NAME}_stdout.log)"
    cd "$IRACE_DIR"
done

echo ""
echo "=== All GA tuning runs complete: $(date) ==="
echo "Best configs: tail experiments/irace_ga/results/irace_*/irace_*_stdout.log"
