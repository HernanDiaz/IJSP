#!/bin/bash
# Quick wiring validation for N2Plus / N2Minus on ft10 (runs=1, timelimit=10).
# Not part of the real experiment — just checks the neighbourhoods are wired
# and produce a sane FT10 makespan (LB=930; expect ~930-950).
set -e
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
SM="$REPO/experiments/n2_worstcase_2026/results/smoke"
mkdir -p "$SM/N2Plus" "$SM/N2Minus"
for cfg in N2Plus N2Minus; do
  sed -e "s/^runs = .*/runs = 1/" -e "s/^timelimit = .*/timelimit = 10/" \
    "$REPO/experiments/n2_worstcase_2026/setup/setup_${cfg}_tuned.txt" > "$SM/${cfg}_smoke.txt"
done
INST=SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt
echo "=== N2Plus smoke ==="
timeout 120 "$EXE" "$SM/N2Plus_smoke.txt" "$INST" "$SM/N2Plus/" 2>&1 | tail -3
echo "=== N2Minus smoke ==="
timeout 120 "$EXE" "$SM/N2Minus_smoke.txt" "$INST" "$SM/N2Minus/" 2>&1 | tail -3
echo "=== result CSVs ==="
grep -h "Best solution" "$SM"/N2Plus/*.csv "$SM"/N2Minus/*.csv 2>/dev/null || \
  ls -la "$SM"/N2Plus "$SM"/N2Minus
