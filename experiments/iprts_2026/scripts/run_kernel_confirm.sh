#!/usr/bin/env bash
# Full-budget SEQUENTIAL confirmation (one process at a time -- safe for the
# machine): does a stronger LS kernel beat the warm best with real budget?
# n3, n8, nh at 180 s / 1 run on tai50_20_01 (N2 baseline best = 2881.5).
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
BIN=../FuzzyFW
SET=experiments/iprts_2026/setup
OUT=experiments/iprts_2026/results/kernelconfirm
BASE="$SET/setup_seedtest_n2n8.txt"
INST=tai50_20_01.F.15_01
rm -rf "$OUT"; mkdir -p "$OUT"
for kern in n3 n8 nh; do
    s="$SET/_confirm_$kern.txt"
    sed -e "s|^localsearch.neighbourhood = .*|localsearch.neighbourhood = ijsp.makespan.$kern|" \
        -e "s|^runs = .*|runs = 1|" \
        -e "s|^timelimit = .*|timelimit = 180|" "$BASE" > "$s"
    od="$OUT/$kern"; mkdir -p "$od"
    echo "=== running $kern (180 s, 1 run) ==="
    "$BIN" "$s" "SelectosYTaillardIntervalos/$INST.txt" "$od/" > "$od/run.log" 2>&1
    best=$(grep -h '^Best solution;' "$od"/*.csv 2>/dev/null | head -1)
    echo "$kern done :: ${best:-NO-CSV}"
done
echo "=== ALL DONE (N2 baseline = 2881.5) ==="
