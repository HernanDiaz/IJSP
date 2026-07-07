#!/bin/bash
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
OUT="$REPO/experiments/mo_green_2026/results/pilot_sweep"
mkdir -p "$OUT/sweep" "$OUT/logs"
sed -e 's/^algorithm = .*/algorithm = SWEEP-PARETO/' \
    -e 's/^runs = .*/runs = 1/' -e 's/^timelimit = .*/timelimit = 600/' \
  experiments/mo_green_2026/setup/setup_LexME_N2ME_tuned.txt > "$OUT/s_sweep.txt"
INSTS="F0.15.0.ft10_10 tai15_15_01.F.15_01 tai20_15_01.F.15_01 tai20_20_01.F.15_01 tai30_15_01.F.15_01 tai30_20_01.F.15_01 tai50_15_01.F.15_01 tai50_20_01.F.15_01"
for inst in $INSTS; do
  while (( $(jobs -rp | wc -l) >= 8 )); do wait -n 2>/dev/null || sleep 5; done
  echo "[$(date '+%H:%M:%S')] launch sweep / $inst"
  timeout 2400 "$EXE" "$OUT/s_sweep.txt" \
    "SelectosYTaillardIntervalosEnergia/$inst.txt" "$OUT/sweep/" \
    > "$OUT/logs/sweep__${inst}.log" 2>&1 &
done
wait
echo "SWEEP_PILOT_DONE"
