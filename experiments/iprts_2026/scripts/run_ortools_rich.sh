#!/usr/bin/env bash
# Richer OR-Tools seed generation for the diversity-gate re-evaluation:
# lo/mid/up coverage + interval SAMPLING (the real diversity lever) per x20
# instance. The two classes run in parallel. ~25 min wall.
# Out: seeds/ortools_x20_rich
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
PY="$HOME/ortools-venv/bin/python3"
GEN=experiments/iprts_2026/scripts/ortools_seeds.py
OUT=experiments/iprts_2026/seeds/ortools_x20_rich
run() {
    echo "=== $1 ==="
    "$PY" "$GEN" --instances "SelectosYTaillardIntervalos/$1_*.txt" \
        --out "$OUT" --scalarizations lo,mid,up,sample --seeds-per 6 --time-limit 15
}
run tai30_20 &
run tai50_20 &
wait
echo "DONE rich generation"
