#!/bin/bash
# Population-based epsilon-ladder pilot (variant b): per instance and cap
# level, a short clamped ABC-Pareto run (90 s); fronts merged offline.
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
OUT="$REPO/experiments/mo_green_2026/results/pilot_ladder"
mkdir -p "$OUT/logs"
CAPS="$OUT/caps.txt"
python3 experiments/mo_green_2026/ladder_caps.py \
  F0.15.0.ft10_10 tai15_15_01.F.15_01 tai20_15_01.F.15_01 \
  tai20_20_01.F.15_01 tai30_15_01.F.15_01 tai30_20_01.F.15_01 \
  tai50_15_01.F.15_01 tai50_20_01.F.15_01 > "$CAPS"
cat "$CAPS"

while read -r stem clo chi; do
  for pm in 10 20 40 60 80 100; do
    d="$OUT/L$pm"; mkdir -p "$d"
    s="$d/s_${stem}.txt"
    sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
        -e 's/^replacement = .*/replacement = nsga2/' \
        -e 's/^runs = .*/runs = 1/' -e 's/^timelimit = .*/timelimit = 90/' \
      experiments/mo_green_2026/setup/setup_LexME_N2ME_tuned.txt > "$s"
    printf '\nenergy.goal-cmax-lo = %d\nenergy.goal-cmax-hi = %d\n' \
      $(( clo * (1000 + pm) / 1000 )) $(( chi * (1000 + pm) / 1000 )) >> "$s"
    while (( $(jobs -rp | wc -l) >= 8 )); do wait -n 2>/dev/null || sleep 2; done
    timeout 400 "$EXE" "$s" "SelectosYTaillardIntervalosEnergia/$stem.txt" \
      "$d/" > "$OUT/logs/L${pm}__${stem}.log" 2>&1 &
  done
done < "$CAPS"
wait
echo "LADDER_DONE"
