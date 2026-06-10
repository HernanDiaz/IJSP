#!/bin/bash
# Merges per-instance solution files from two seed source dirs (header lines
# are skipped by the loader, so plain concatenation is enough).
# Usage: bash merge_seeds.sh <dirA> <dirB> <target_dir>   (paths or names
# relative to the seeds/ folder next to this script's parent)
set -eu

HERE="$(cd "$(dirname "$0")/.." && pwd)"
A="${1:-$HERE/seeds/N2_tuned}"
B="${2:-$HERE/seeds/N8_tuned}"
DST="${3:-$HERE/seeds/N2N8_tuned}"

mkdir -p "$DST"
count=0
for f in "$A"/*_Sols.csv; do
    base="$(basename "$f")"
    if [ -f "$B/$base" ]; then
        cat "$f" "$B/$base" > "$DST/$base"
        count=$((count + 1))
    fi
done
echo "merged $count instances into $DST"
