#!/bin/bash
# Restricted-E N2ME smoke on tai50_15_01: same budget head-to-head vs the
# plain LexME-N2 tie-break (1 run x 300 s each, sequential, single process).
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
OUT="$REPO/experiments/mo_green_2026/results/smoke_tai50"
mkdir -p "$OUT/n2me_r" "$OUT/n2"
INST=SelectosYTaillardIntervalosEnergia/tai50_15_01.F.15_01.txt

sed -e 's/^runs = .*/runs = 1/' -e 's/^timelimit = .*/timelimit = 300/' \
  experiments/mo_green_2026/setup/setup_LexME_N2ME_tuned.txt > "$OUT/s_n2me_r.txt"
sed -e 's/^runs = .*/runs = 1/' -e 's/^timelimit = .*/timelimit = 300/' \
  experiments/mo_green_2026/setup/setup_LexME_N2_tuned.txt > "$OUT/s_n2.txt"

echo "=== restricted N2ME (300s) ==="
timeout 600 "$EXE" "$OUT/s_n2me_r.txt" "$INST" "$OUT/n2me_r/" > /dev/null 2>&1
grep -h "Best solution" "$OUT"/n2me_r/*.csv | head -1
grep -A3 "Objective function values" "$OUT"/n2me_r/*.csv | tail -1 | cut -c1-80

echo "=== plain LexME-N2 (300s) ==="
timeout 600 "$EXE" "$OUT/s_n2.txt" "$INST" "$OUT/n2/" > /dev/null 2>&1
grep -h "Best solution" "$OUT"/n2/*.csv | head -1
grep -A3 "Objective function values" "$OUT"/n2/*.csv | tail -1 | cut -c1-80
echo "SMOKE_DONE"
