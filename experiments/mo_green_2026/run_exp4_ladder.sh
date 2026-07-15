#!/bin/bash
# Exp. 4 — LADDER arm: self-contained pipeline per (instance, run).
# Random-init lexicographic anchor -> chained clamped levels (seeding is
# INTERNAL to the run: level 1 from the anchor's solutions, level k from
# level k-1's front + anchor). Size-adaptive budgets within ~900 s.
# Usage: run_exp4_ladder.sh <tier> [par]   (resumable via DONE markers)
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
EXP="$REPO/experiments/mo_green_2026"
# Env-overridable so the tuned re-run writes to a separate tree with its own
# base setup, leaving the inherited-config EXP4 data untouched.
OUT="${OUT_DIR:-$EXP/results/EXP4/LADDER}"
BASESETUP="${BASE_SETUP:-$EXP/setup/setup_LexME_N2ME_tuned.txt}"
# Anchor phase may use a different (makespan-strong) config; default = same as
# the levels, so absent ANCHOR_SETUP the behaviour is byte-identical to before.
ANCHORSETUP="${ANCHOR_SETUP:-$BASESETUP}"
BASELINE_CSV="$REPO/experiments/cor_tabu_2026/statistical_results/runs_data.csv"
TIER="${1:-smoke}"; MAX="${2:-14}"
LOG="${LOG_FILE:-$EXP/run_exp4_ladder.log}"
mkdir -p "$OUT"

budget() {  # echo "<anchor_s> <level_s> <permil list>"
  case "$1" in
    tai30*) echo "500 65 5 10 20 30 50 100";;
    tai50*) echo "600 60 5 10 20 50 100";;
    *)      echo "400 60 2 5 10 15 20 30 50 70";;
  esac
}

run_pipeline() {  # stem run
  local stem=$1 run=$2
  local d="$OUT/$stem/r$run"
  [ -f "$d/DONE" ] && return 0
  rm -rf "$d"; mkdir -p "$d/anchor" "$d/seeds"
  read -r ta tl permils <<< "$(budget "$stem")"

  # ---- Anchor: random init, lexicographic, no cap ----
  sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
      -e 's/^replacement = .*/replacement = nsga2/' \
      -e 's/^runs = .*/runs = 1/' \
      -e "s/^timelimit = .*/timelimit = $ta/" \
      -e "s/^seed = .*/seed = $run/" \
    "$ANCHORSETUP" > "$d/s_anchor.txt"
  timeout $((ta*3)) "$EXE" "$d/s_anchor.txt" \
    "SelectosYTaillardIntervalosEnergia/$stem.txt" "$d/anchor/" \
    > "$d/anchor.log" 2>&1 || { echo "FAIL anchor $stem r$run"; return 1; }

  # C* of the run from the anchor's _Sols objective "(lo, up) | ..."
  local sols cstar_lo cstar_hi
  sols=$(ls "$d/anchor/${stem}"_*_Sols.csv 2>/dev/null | head -1)
  [ -n "${sols:-}" ] || { echo "FAIL no anchor sols $stem r$run"; return 1; }
  read -r cstar_lo cstar_hi <<< "$(awk -F';' 'NR==2{match($3,/\(([0-9]+), *([0-9]+)\)/,a); print a[1], a[2]; exit}' "$sols")"
  [ -n "${cstar_lo:-}" ] || { echo "FAIL no C* $stem r$run"; return 1; }

  # Seed pool: anchor solutions (Sols + Front)
  local seedf="$d/seeds/${stem}_Sols.csv"
  { awk -F';' 'NR>1 && NF>=2 {print NR-1";"$2";(0, 0)"}' "$sols"
    for f in "$d"/anchor/*_Front.csv; do
      [ -f "$f" ] && awk -F';' 'NR>1 && NF>=5 {print NR";"$5";(0, 0)"}' "$f"
    done; } > "$seedf"

  # ---- Chained clamped levels ----
  local pm
  for pm in $permils; do
    local ld="$d/L$pm"; mkdir -p "$ld"
    sed -e 's/^algorithm = .*/algorithm = ABCPSO-Pareto/' \
        -e 's/^replacement = .*/replacement = nsga2/' \
        -e 's/^runs = .*/runs = 1/' \
        -e "s/^timelimit = .*/timelimit = $tl/" \
        -e "s/^seed = .*/seed = $((run*1000+pm))/" \
        -e 's|^creation = .*|creation = ijsp.solutions-file|' \
      "$BASESETUP" > "$ld/s.txt"
    printf '\ncreation.solutions-dir = %s\ncreation.seed-selection = maxmin\nenergy.goal-cmax-lo = %d\nenergy.goal-cmax-hi = %d\n' \
      "$d/seeds" $(( cstar_lo * (1000 + pm) / 1000 )) \
      $(( cstar_hi * (1000 + pm) / 1000 )) >> "$ld/s.txt"
    timeout $((tl*4)) "$EXE" "$ld/s.txt" \
      "SelectosYTaillardIntervalosEnergia/$stem.txt" "$ld/" \
      > "$ld/run.log" 2>&1
    local front
    front=$(ls "$ld/${stem}"_*_Front.csv 2>/dev/null | head -1)
    if [ -n "${front:-}" ]; then
      { awk -F';' 'NR>1 && NF>=5 {print NR";"$5";(0, 0)"}' "$front"
        cat "$seedf"; } > "$seedf.tmp" && mv "$seedf.tmp" "$seedf"
    fi
  done
  touch "$d/DONE"
}

mapfile -t ALL < <(awk -F, '$2=="n2"{print $4}' "$BASELINE_CSV" | sort -u)
sel() { for i in "${ALL[@]}"; do case "$1" in
  smoke) [[ "$i" == *ft10* ]] && echo "$i";;
  classical) [[ "$i" == *abz* || "$i" == *ft* || "$i" == *la* ]] && echo "$i";;
  tai_small) [[ "$i" == tai15* || "$i" == tai20* ]] && echo "$i";;
  full) echo "$i";; esac; done; }
mapfile -t INSTS < <(sel "$TIER")

RUNS="${RUNS:-30}"
echo "EXP4 LADDER tier=$TIER par=$MAX insts=${#INSTS[@]} runs=$RUNS $(date)" | tee "$LOG"
for inst in "${INSTS[@]}"; do
  for r in $(seq 1 "$RUNS"); do
    [ -f "$OUT/$inst/r$r/DONE" ] && continue
    while (( $(jobs -rp | wc -l) >= MAX )); do wait -n 2>/dev/null || sleep 5; done
    echo "  [$(date '+%H:%M:%S')] $inst r$r" | tee -a "$LOG"
    run_pipeline "$inst" "$r" &
  done
done
wait
echo "EXP4 LADDER tier=$TIER DONE $(date)" | tee -a "$LOG"
