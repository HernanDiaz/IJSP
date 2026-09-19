#!/bin/bash
#
# paired_setups.sh — run two setups (two configurations of the same build) side
# by side on the same instances, so that neither arm gets a faster machine than
# the other.
#
# Usage: paired_setups.sh <setup-A> <setup-B> <tag-A> <tag-B> <instance>...
#
# Every setup here stops on wall-clock time, so how much search fits in a run
# depends on what the machine is doing during that run -- and this machine
# drifts by about 9 % between one batch and the next (see JOURNAL.md,
# 2026-09-19). A comparison of two configurations run one after the other is
# therefore confounded with when each ran, exactly as a comparison of two builds
# is. The cure is the same: run them at the same time. Each arm gets half the
# cores; the instance list is walked in groups, and within a group both arms run
# the same instances at the same moment. Whatever the machine does to one, it
# does to the other.
#
# Solution quality is what is compared afterwards (scripts/compare.py, paired by
# instance), not speed; this script only makes sure the budgets were equal in
# fact and not just on paper.
#
set -u

if [ $# -lt 5 ]; then
    echo "usage: $0 <setup-A> <setup-B> <tag-A> <tag-B> <instance> [instance ...]" >&2
    exit 2
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
RUN="${ROOT}/experiments/classic_jsp_2026/scripts/run_jsp.sh"
A="$1"; B="$2"; TAG_A="$3"; TAG_B="$4"; shift 4
INSTANCES=("$@")
CORES="${CORES:-$(nproc 2>/dev/null || echo 4)}"
PER_ARM=$(( CORES / 2 ))
[ "$PER_ARM" -lt 1 ] && PER_ARM=1

for s in "$A" "$B"; do
    [ -f "$s" ] || { echo "error: setup $s not found" >&2; exit 1; }
done

echo "A: $A -> results/$TAG_A"
echo "B: $B -> results/$TAG_B"
echo "${PER_ARM} instances per arm at a time, $(date)"

i=0
while [ "$i" -lt "${#INSTANCES[@]}" ]; do
    group=("${INSTANCES[@]:$i:$PER_ARM}")
    echo "=== ${group[*]}  ($(date +%H:%M)) ==="
    MAX_PARALLEL="$PER_ARM" bash "$RUN" "$A" "$TAG_A" "${group[@]}" > /dev/null 2>&1 &
    pa=$!
    MAX_PARALLEL="$PER_ARM" bash "$RUN" "$B" "$TAG_B" "${group[@]}" > /dev/null 2>&1 &
    pb=$!
    wait "$pa" "$pb"
    i=$(( i + PER_ARM ))
done
echo "done $(date)"
