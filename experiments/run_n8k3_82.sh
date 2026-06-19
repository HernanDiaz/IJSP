#!/bin/bash
# Full 82-instance run of the published TS-N2 config with the N8 (k=3) mutation.
# Same protocol as the paper: 30 runs x 900 s, noimprovement=20.
# One process per instance (30 internal runs), throttled to MAXJOBS concurrent.
# Resumable: an instance whose .log already says "finished" is skipped.
#
# Launch DETACHED from PowerShell so it survives the terminal closing:
#   wsl bash -lc "setsid nohup bash /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/run_n8k3_82.sh > /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/n8k3_results/launch.log 2>&1 < /dev/null &"
# (the output dir is created below; create it first if launching detached, or
#  just run:  wsl bash .../run_n8k3_82.sh   to see progress live)

set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
BIN="$REPO/../FuzzyFW"
SETUP="$REPO/experiments/setup_N2_n8k3.txt"
INSTDIR="$REPO/SelectosYTaillardIntervalos"
OUT="$REPO/experiments/n8k3_results"
MAXJOBS=26

cd "$REPO" || exit 1
mkdir -p "$OUT"

# --- the 82 instances: 12 classical (selectas) + 70 Taillard ----------------
CLASSIC=(
  F0.15.0.abz7_06 F0.15.0.abz8_05 F0.15.0.abz9_10
  F0.15.0.ft10_10 F0.15.0.ft20_05
  F0.15.0.la21_04 F0.15.0.la24_03 F0.15.0.la25_04
  F0.15.0.la27_09 F0.15.0.la29_03 F0.15.0.la38_06 F0.15.0.la40_05
)
insts=()
for c in "${CLASSIC[@]}"; do
  insts+=("$INSTDIR/$c.txt")
done
for cls in tai15_15 tai20_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for k in 01 02 03 04 05 06 07 08 09 10; do
    insts+=("$INSTDIR/${cls}_${k}.F.15_01.txt")
  done
done

echo "=== N8-k3 full run start $(date) -- ${#insts[@]} instances, setup: $SETUP"
echo "=== binary: $BIN ($(ls -la --time-style=+%H:%M:%S "$BIN" 2>/dev/null | awk '{print $5,$6}'))"

for f in "${insts[@]}"; do
  stem="$(basename "${f%.txt}")"
  if [ ! -e "$f" ]; then
    echo "MISSING $f"; continue
  fi
  if grep -q 'The process has finished' "$OUT/$stem.log" 2>/dev/null; then
    echo "skip (done) $stem"; continue
  fi
  # throttle to MAXJOBS concurrent
  while [ "$(jobs -rp | wc -l)" -ge "$MAXJOBS" ]; do
    wait -n
  done
  rm -f "$OUT/$stem"*.csv "$OUT/$stem.log" 2>/dev/null
  ( "$BIN" "$SETUP" "$f" "$OUT/" > "$OUT/$stem.log" 2>&1; \
    echo "[$(date +%H:%M:%S)] done $stem" ) &
  echo "[$(date +%H:%M:%S)] launched $stem"
done

wait
echo "=== N8-k3 full run COMPLETE $(date)"
