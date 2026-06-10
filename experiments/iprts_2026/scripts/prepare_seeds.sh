#!/bin/bash
# Copies the paper's stored solutions (<instance>_<timestamp>_Sols.csv) into
# canonical names (<instance>_Sols.csv) so CreationFromFileSchedule can
# resolve them from the problem name.
# Usage: bash prepare_seeds.sh <source_dir> [target_dir]
set -eu

SRC="$1"
HERE="$(cd "$(dirname "$0")/.." && pwd)"
DST="${2:-$HERE/seeds/$(basename "$SRC")}"

mkdir -p "$DST"
count=0
for f in "$SRC"/*_Sols.csv; do
    base="$(basename "$f")"
    inst="$(echo "$base" | sed -E 's/_[0-9]{14}_Sols\.csv$//')"
    cp "$f" "$DST/${inst}_Sols.csv"
    count=$((count + 1))
done
echo "copied $count solution files to $DST"
