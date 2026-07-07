#!/bin/bash
# Pareto engine pilot: ABC archive-based vs MA with true NSGA-II survival.
# One instance per size class x 2 engines, 1 run x 900 s, N2ME (restricted
# + lazy) as the lexicographic-aware LS in both. 8-parallel (machine free).
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
OUT="$REPO/experiments/mo_green_2026/results/pilot_pareto"
mkdir -p "$OUT/abc" "$OUT/ma" "$OUT/logs"

BASE=experiments/mo_green_2026/setup/setup_LexME_N2ME_tuned.txt
sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
    -e 's/^replacement = .*/replacement = nsga2/' \
    -e 's/^runs = .*/runs = 1/' "$BASE" > "$OUT/s_abc.txt"
sed -e 's/^algorithm = .*/algorithm = MEMETIC-PARETO/' \
    -e 's/^replacement = .*/replacement = nsga2/' \
    -e 's/^runs = .*/runs = 1/' "$BASE" > "$OUT/s_ma.txt"

INSTS="F0.15.0.ft10_10 tai15_15_01.F.15_01 tai20_15_01.F.15_01 tai20_20_01.F.15_01 tai30_15_01.F.15_01 tai30_20_01.F.15_01 tai50_15_01.F.15_01 tai50_20_01.F.15_01"

MAX=8
for inst in $INSTS; do
  for eng in abc ma; do
    while (( $(jobs -rp | wc -l) >= MAX )); do wait -n 2>/dev/null || sleep 5; done
    echo "[$(date '+%H:%M:%S')] launch $eng / $inst"
    timeout 2400 "$EXE" "$OUT/s_$eng.txt" \
      "SelectosYTaillardIntervalosEnergia/$inst.txt" "$OUT/$eng/" \
      > "$OUT/logs/${eng}__${inst}.log" 2>&1 &
  done
done
wait
echo "PILOT_DONE"
