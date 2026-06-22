#!/usr/bin/env bash
# Quick screen: IPRTS warm (N2N8 bank) with different LS kernels, to see if a
# stronger neighbourhood than N2 breaks the record from the same seeds.
# 1 run x 40 s on tai50_20_01 (N2 baseline best = 2881.5). PR stays on N1.
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
BIN=../FuzzyFW
SET=experiments/iprts_2026/setup
OUT=experiments/iprts_2026/results/kernelscreen
INST=tai50_20_01.F.15_01
BASE="$SET/setup_seedtest_n2n8.txt"
rm -rf "$OUT"; mkdir -p "$OUT"
for kern in n2 n3 n8 next n2plus nh; do
    s="$SET/_screen_$kern.txt"
    sed -e "s|^localsearch.neighbourhood = .*|localsearch.neighbourhood = ijsp.makespan.$kern|" \
        -e "s|^runs = .*|runs = 1|" \
        -e "s|^timelimit = .*|timelimit = 40|" "$BASE" > "$s"
    od="$OUT/$kern"; mkdir -p "$od"
    "$BIN" "$s" "SelectosYTaillardIntervalos/$INST.txt" "$od/" > "$od/run.log" 2>&1 &
done
wait
echo "=== screen done (N2 baseline best = 2881.5) ==="
for kern in n2 n3 n8 next n2plus nh; do
    line=$(grep -h '^Best solution;' "$OUT/$kern"/*.csv 2>/dev/null | head -1)
    err=$(grep -hiE 'error|exception|invalid|not found' "$OUT/$kern/run.log" 2>/dev/null | head -1)
    printf '%-8s :: %s %s\n' "$kern" "${line:-NO-CSV}" "${err:+| ERR: $err}"
done
