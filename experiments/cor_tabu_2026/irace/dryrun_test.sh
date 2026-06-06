#!/bin/bash
# Quick dry-run: tune N2 with only 80 experiments to validate the pipeline end-to-end
IRACE_DIR="/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/irace"
RSCRIPT="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"

TMPDIR=$(mktemp -d /mnt/c/Users/diazhernan/AppData/Local/Temp/irace_dryrun_XXXXXX)
echo "Working dir: $TMPDIR"

cp "$IRACE_DIR/scenario.txt"           "$TMPDIR/"
cp "$IRACE_DIR/parameters.txt"         "$TMPDIR/"
cp "$IRACE_DIR/instances.txt"          "$TMPDIR/"
cp "$IRACE_DIR/base_setup_ts_lex2.txt" "$TMPDIR/"
sed "s|NB=\"\${IRACE_NEIGHBOURHOOD:-ijsp.makespan.n2}\"|NB=\"ijsp.makespan.n2\"|" \
    "$IRACE_DIR/target-runner.sh" > "$TMPDIR/target-runner.sh"
cp "$IRACE_DIR/target-runner.cmd"      "$TMPDIR/"
chmod +x "$TMPDIR/target-runner.sh"

# Override budget and parallelism for quick test
sed -i 's/maxExperiments.*= 1500/maxExperiments = 250/' "$TMPDIR/scenario.txt"
sed -i 's/parallel.*= 24/parallel = 8/'                "$TMPDIR/scenario.txt"
sed -i 's/^logFile.*/logFile = "irace_dryrun.log"/'   "$TMPDIR/scenario.txt"

export IRACE_NEIGHBOURHOOD=ijsp.makespan.n2

cd "$TMPDIR"
echo "Starting irace dry-run for N2 (80 experiments, 4 parallel)..."
"$RSCRIPT" -e "library(irace); irace_main(scenario=readScenario('scenario.txt'))" 2>&1
EXIT=$?
echo ""
echo "irace exit code: $EXIT"

if [ $EXIT -eq 0 ] && [ -f "$TMPDIR/irace.Rdata" ]; then
    echo "--- Best configuration ---"
    "$RSCRIPT" -e "
        library(irace)
        load('irace.Rdata')
        print(getFinalElites(iraceResults, n=1))
    " 2>&1
fi

rm -rf "$TMPDIR"
