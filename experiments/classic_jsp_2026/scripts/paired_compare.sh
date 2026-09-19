#!/bin/bash
#
# paired_compare.sh — compare two builds of the solver without confounding the
# comparison with when each one ran.
#
# Usage: paired_compare.sh <repo-A> <repo-B> <setup-file> <tag> <instance>...
#
# Why this exists. Running configuration A as one batch and configuration B as
# another does not work on a machine that is doing anything else. Measured here
# on 2026-09-19: one binary, identical by md5, reported 4.122 gen/s in one batch
# and 3.812 in another -- a 9 % swing with 100 runs behind each figure. A
# link-time-optimisation build looked like a 9.4 % win under that design and
# turned out to be worth nothing when the two builds were run side by side.
#
# So this runs both builds at the same time, on the same instances. Each build
# gets half the instances of a group in flight at once, which keeps the total
# process count within the core count; whatever the machine is doing to one
# build it is doing to the other at the same moment.
#
# The two repositories must be separate trees, because the Makefile puts the
# executable at ../FuzzyFW: two sibling checkouts would share one binary.
#
set -u

if [ $# -lt 5 ]; then
    echo "usage: $0 <repo-A> <repo-B> <setup-file> <tag> <instance> [instance ...]" >&2
    exit 2
fi

A="$1"; B="$2"; SETUP="$3"; TAG="$4"; shift 4
INSTANCES=("$@")
CORES="${CORES:-$(nproc 2>/dev/null || echo 4)}"
# Half the cores per build, so the two together do not oversubscribe.
PER_BUILD=$(( CORES / 2 ))
[ "$PER_BUILD" -lt 1 ] && PER_BUILD=1

for repo in "$A" "$B"; do
    if [ ! -x "${repo}/../FuzzyFW" ]; then
        echo "error: no solver at ${repo}/../FuzzyFW" >&2
        exit 1
    fi
done
if [ "$(cd "$A" && pwd)" = "$(cd "$B" && pwd)" ]; then
    echo "error: both arguments name the same tree" >&2
    exit 1
fi

echo "A: $A  ($(md5sum "$A/../FuzzyFW" | cut -c1-12))"
echo "B: $B  ($(md5sum "$B/../FuzzyFW" | cut -c1-12))"
echo "running ${PER_BUILD} instances per build at a time"

# Split the instance list into groups of PER_BUILD and run each group on both
# builds simultaneously.
i=0
while [ "$i" -lt "${#INSTANCES[@]}" ]; do
    group=("${INSTANCES[@]:$i:$PER_BUILD}")
    echo "=== ${group[*]} ==="
    ( cd "$A" && MAX_PARALLEL="$PER_BUILD" \
        bash experiments/classic_jsp_2026/scripts/run_jsp.sh \
        "$SETUP" "${TAG}_A" "${group[@]}" > /dev/null 2>&1 ) &
    pa=$!
    ( cd "$B" && MAX_PARALLEL="$PER_BUILD" \
        bash experiments/classic_jsp_2026/scripts/run_jsp.sh \
        "$SETUP" "${TAG}_B" "${group[@]}" > /dev/null 2>&1 ) &
    pb=$!
    wait "$pa" "$pb"
    i=$(( i + PER_BUILD ))
done

echo
python3 "$B/experiments/classic_jsp_2026/scripts/generations.py" \
    "$A/experiments/classic_jsp_2026/results/${TAG}_A" \
    "$B/experiments/classic_jsp_2026/results/${TAG}_B"
