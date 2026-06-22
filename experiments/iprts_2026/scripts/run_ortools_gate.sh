#!/usr/bin/env bash
# Gate-pass OR-Tools seed generation: lo/mid/up scalarisations (seeds-per 1)
# for the 20 x20 Taillard instances, into seeds/ortools_x20 (~16 min total).
# A lean first pass to measure whether cross-solver seeds de-cluster the bank
# before committing to a fuller, longer generation.
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
PY="$HOME/ortools-venv/bin/python3"
GEN=experiments/iprts_2026/scripts/ortools_seeds.py
OUT=experiments/iprts_2026/seeds/ortools_x20
for cls in tai30_20 tai50_20; do
    echo "=== $cls ==="
    "$PY" "$GEN" --instances "SelectosYTaillardIntervalos/${cls}_*.txt" \
        --out "$OUT" --time-limit 15 --seeds-per 1
done
echo "DONE gate generation"
