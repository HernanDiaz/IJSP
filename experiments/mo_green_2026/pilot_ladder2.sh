#!/bin/bash
# Ladder v2: warm-seeded clamped levels. Each instance runs its 6 levels
# SEQUENTIALLY (level k seeds from level k-1's front + the LexME anchors,
# via creation = ijsp.solutions-file); instances run in parallel (8).
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
OUT="$REPO/experiments/mo_green_2026/results/pilot_ladder2"
mkdir -p "$OUT/logs"
CAPS="$OUT/caps.txt"
python3 experiments/mo_green_2026/ladder_caps.py \
  F0.15.0.ft10_10 tai15_15_01.F.15_01 tai20_15_01.F.15_01 \
  tai20_20_01.F.15_01 tai30_15_01.F.15_01 tai30_20_01.F.15_01 \
  tai50_15_01.F.15_01 tai50_20_01.F.15_01 > "$CAPS"

run_instance() {
  local stem=$1 clo=$2 chi=$3
  local anchor
  anchor=$(ls "$REPO/experiments/mo_green_2026/results/LexME/${stem}"_*_Sols.csv 2>/dev/null | sort | tail -1)
  local seeddir="$OUT/seeds_$stem"
  mkdir -p "$seeddir"
  cp "$anchor" "$seeddir/${stem}_Sols.csv"
  for pm in 10 20 40 60 80 100; do
    local d="$OUT/L$pm"; mkdir -p "$d"
    local s="$d/s_${stem}.txt"
    sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
        -e 's/^replacement = .*/replacement = nsga2/' \
        -e 's/^runs = .*/runs = 1/' -e 's/^timelimit = .*/timelimit = 90/' \
        -e 's|^creation = .*|creation = ijsp.solutions-file|' \
      experiments/mo_green_2026/setup/setup_LexME_N2ME_tuned.txt > "$s"
    printf '\ncreation.solutions-dir = %s\ncreation.seed-selection = maxmin\nenergy.goal-cmax-lo = %d\nenergy.goal-cmax-hi = %d\n' \
      "$seeddir" $(( clo * (1000 + pm) / 1000 )) $(( chi * (1000 + pm) / 1000 )) >> "$s"
    timeout 400 "$EXE" "$s" "SelectosYTaillardIntervalosEnergia/$stem.txt" \
      "$d/" > "$OUT/logs/L${pm}__${stem}.log" 2>&1
    # Next level's seeds: this level's front solutions + the anchors
    local front
    front=$(ls "$d/${stem}"_*_Front.csv 2>/dev/null | sort | tail -1)
    if [ -n "${front:-}" ]; then
      { awk -F';' 'NR>1 && NF>=5 {print NR-1";"$5";(0, 0)"}' "$front"
        cat "$anchor"; } > "$seeddir/${stem}_Sols.csv"
    fi
  done
}

while read -r stem clo chi; do
  while (( $(jobs -rp | wc -l) >= 8 )); do wait -n 2>/dev/null || sleep 2; done
  run_instance "$stem" "$clo" "$chi" &
done < "$CAPS"
wait
echo "LADDER2_DONE"
