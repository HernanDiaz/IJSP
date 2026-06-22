#!/usr/bin/env bash
# Seeding-test A/B: same IPRTS cfg187, two warm banks (N2N8 vs N2N8+OR-Tools),
# on the 3 highest-novelty tai50x20 instances. 3 runs x 180 s each, all 6
# invocations in parallel. Compares best makespan to see if the OR-Tools seeds
# change record-best despite the weak diversity gate.
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
BIN=../FuzzyFW
SET=experiments/iprts_2026/setup
OUT=experiments/iprts_2026/results/seedtest
INSTS="tai50_20_01.F.15_01 tai50_20_02.F.15_01 tai50_20_07.F.15_01"
for bank in n2n8 combo; do
    mkdir -p "$OUT/$bank"
done
for inst in $INSTS; do
    for bank in n2n8 combo; do
        "$BIN" "$SET/setup_seedtest_$bank.txt" \
            "SelectosYTaillardIntervalos/$inst.txt" "$OUT/$bank/" \
            > "$OUT/$bank/$inst.log" 2>&1 &
    done
done
wait
echo "DONE seedtest A/B"
