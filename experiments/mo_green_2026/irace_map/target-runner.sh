#!/bin/bash
###############################################################################
# irace target runner for the ABC-P baseline (ABCPSO-Pareto, free Pareto).
# One full FuzzyFW run (900 s) with the config's engine params; returns -HV
# of its _Front.csv vs the instance's fixed reference point (ref_points.txt).
# Isolated temp workspace (mktemp) — overwrites nothing.
###############################################################################
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP"
EXE="/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW"
BASE="$HERE/base_setup.txt"
TIMELIMIT="${IRACE_DEBUG_TL:-900}"

CONFIG_ID="$1"; INSTANCE_ID="$2"; SEED="$3"; INSTANCE="$4"; shift 4
while [[ $# -gt 0 ]]; do eval "p_${1#--}=\"$2\""; shift 2; done

stem="$(basename "$INSTANCE" .txt)"
ref=$(awk -v s="$stem" '$1==s{print $2, $3; exit}' "$HERE/ref_points.txt")
[ -z "$ref" ] && { echo "1e12"; exit 0; }
read -r CREF EREF <<< "$ref"

TMP=$(mktemp -d /tmp/iraceB_XXXXXX); trap "rm -rf $TMP" EXIT
mkdir -p "$TMP/out"
sed -e "s/IRACE_SEED/$SEED/" \
    -e "s/IRACE_ELITE_SIZE/${p_elite_size}/" \
    -e "s/IRACE_ELITE_SEL/${p_elite_sel}/" \
    -e "s/IRACE_MAXTRIALS/${p_maxtrials}/" \
    -e "s/IRACE_LS_TARGET/${p_ls_target}/" \
    -e "s/IRACE_TABU_ITER/${p_tabu_iter}/" \
    -e "s/IRACE_CROSSOVER/${p_crossover}/" \
    -e "s/IRACE_MUTATION/${p_mutation}/" \
    -e "s/^timelimit = .*/timelimit = ${TIMELIMIT}/" "$BASE" > "$TMP/s.txt"
grep -q '^timelimit' "$TMP/s.txt" || printf 'timelimit = %s\n' "$TIMELIMIT" >> "$TMP/s.txt"

timeout $((TIMELIMIT*3)) "$EXE" "$TMP/s.txt" "$INSTANCE" "$TMP/out/" > /dev/null 2>&1

MERGED="$TMP/merged.csv"
: > "$MERGED"
for f in "$TMP"/out/${stem}_*_Front.csv; do [ -f "$f" ] && tail -n +2 "$f" >> "$MERGED"; done
python3 "$HERE/hv_cost.py" "$MERGED" "$CREF" "$EREF"
