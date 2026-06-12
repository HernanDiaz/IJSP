#!/bin/bash
# Measures the CPU-vs-wall accrual ratio under irace-like 24-way contention:
# launches N parallel tuning-style runs (timelimit=300, postexecution=no) on
# tai50_20 and reports per-run wall time vs reported CPU runtime.
set -u
N="${1:-24}"
REPO="$(cd "$(dirname "$0")/../../.." && pwd)"
OUT=/tmp/audit_contention
rm -rf "$OUT"; mkdir -p "$OUT"

sed -e 's/^timelimit = 900/timelimit = 300/' \
    -e 's/IRACE_SEED/123/' -e 's/IRACE_POOL_SIZE/10/' \
    -e 's/IRACE_POOL_MINDIST/0.05/' -e 's/IRACE_PATIENCE/50/' \
    -e 's/IRACE_PERTURB/0.15/' -e 's/IRACE_TABU_ITER/200/' \
    -e 's/IRACE_TABU_MAX/20/' -e 's/IRACE_MAX_ROUNDS/40/' \
    "$(dirname "$0")/base_setup_iprts.txt" > "$OUT/setup.txt"

for i in $(seq 1 "$N"); do
    (
        mkdir -p "$OUT/run$i"
        s=$(date +%s)
        /home/diazhernan/FuzzyFW_iprts "$OUT/setup.txt" \
            "$REPO/SelectosYTaillardIntervalos/tai50_20_03.F.15_01.txt" \
            "$OUT/run$i/" > /dev/null 2>&1
        e=$(date +%s)
        cpu=$(grep -h 'Total runtime;' "$OUT/run$i/"*[0-9].csv 2>/dev/null | cut -d';' -f2)
        sols=$(ls "$OUT/run$i/"*Sols.csv 2>/dev/null | wc -l)
        echo "run$i wall=$((e-s))s cpu=${cpu:-NONE}s sols=$sols" >> "$OUT/results.txt"
    ) &
done
wait
echo "==== AUDIT DONE ===="
sort -t= -k2 -n "$OUT/results.txt"
