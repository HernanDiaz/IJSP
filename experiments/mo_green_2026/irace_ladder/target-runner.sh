#!/bin/bash
###############################################################################
# irace target runner for the LADDER arm.
# Runs the full self-contained pipeline (random anchor -> chained clamped
# levels, size-adaptive budgets as in EXP4_DESIGN) with the config's engine
# parameters, merges all fronts and returns -HV vs the instance's fixed
# reference point (ref_points.txt).
#
# irace call: target-runner.sh <configID> <instanceID> <seed> <instance> --p v...
###############################################################################
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP"
EXE="/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW"
BASE="$HERE/base_setup_ladder.txt"

CONFIG_ID="$1"; INSTANCE_ID="$2"; SEED="$3"; INSTANCE="$4"; shift 4
while [[ $# -gt 0 ]]; do eval "p_${1#--}=\"$2\""; shift 2; done

stem="$(basename "$INSTANCE" .txt)"
ref=$(awk -v s="$stem" '$1==s{print $2, $3; exit}' "$HERE/ref_points.txt")
[ -z "$ref" ] && { echo "1e12"; exit 0; }   # unknown instance -> worst
read -r CREF EREF <<< "$ref"

case "$stem" in
  tai30*) TA=500; TL=65; PERMILS="5 10 20 30 50 100";;
  tai50*) TA=600; TL=60; PERMILS="5 10 20 50 100";;
  *)      TA=400; TL=60; PERMILS="2 5 10 15 20 30 50 70";;
esac
# Debug budget override (dry-run only)
[ -n "${IRACE_DEBUG_TA:-}" ] && TA=$IRACE_DEBUG_TA
[ -n "${IRACE_DEBUG_TL:-}" ] && TL=$IRACE_DEBUG_TL
[ -n "${IRACE_DEBUG_PERMILS:-}" ] && PERMILS="$IRACE_DEBUG_PERMILS"

TMP=$(mktemp -d /tmp/iraceL_XXXXXX); trap "rm -rf $TMP" EXIT
mkdir -p "$TMP/anchor" "$TMP/seeds"

mk_setup() {  # <timelimit> <seed> <extra-lines-file-or-empty> -> stdout
  sed -e "s/IRACE_SEED/$2/" \
      -e "s/IRACE_ELITE_SIZE/${p_elite_size}/" \
      -e "s/IRACE_ELITE_SEL/${p_elite_sel}/" \
      -e "s/IRACE_MAXTRIALS/${p_maxtrials}/" \
      -e "s/IRACE_LS_TARGET/${p_ls_target}/" \
      -e "s/IRACE_TABU_ITER/${p_tabu_iter}/" \
      -e "s/IRACE_CROSSOVER/${p_crossover}/" \
      -e "s/IRACE_MUTATION/${p_mutation}/" "$BASE"
  printf 'timelimit = %s\n' "$1"
  [ -n "${3:-}" ] && cat "$3"
}

# ---- Anchor ----
mk_setup "$TA" "$SEED" "" > "$TMP/s_anchor.txt"
timeout $((TA*3)) "$EXE" "$TMP/s_anchor.txt" "$INSTANCE" "$TMP/anchor/" \
  > /dev/null 2>&1
sols=$(ls "$TMP"/anchor/${stem}_*_Sols.csv 2>/dev/null | head -1)
[ -z "$sols" ] && { echo "0.0"; exit 0; }
read -r CLO CHI <<< "$(awk -F';' 'NR==2{match($3,/\(([0-9]+), *([0-9]+)\)/,a); print a[1], a[2]; exit}' "$sols")"
[ -z "${CLO:-}" ] && { echo "0.0"; exit 0; }

MERGED="$TMP/merged.csv"
{ for f in "$TMP"/anchor/*_Front.csv; do [ -f "$f" ] && tail -n +2 "$f"; done; } > "$MERGED"
seedf="$TMP/seeds/${stem}_Sols.csv"
{ awk -F';' 'NR>1&&NF>=2{print NR-1";"$2";(0, 0)"}' "$sols"
  for f in "$TMP"/anchor/*_Front.csv; do [ -f "$f" ] && awk -F';' 'NR>1&&NF>=5{print NR";"$5";(0, 0)"}' "$f"; done; } > "$seedf"

# ---- Chained clamped levels ----
for pm in $PERMILS; do
  ld="$TMP/L$pm"; mkdir -p "$ld"
  { printf 'creation = ijsp.solutions-file\ncreation.solutions-dir = %s\ncreation.seed-selection = maxmin\nenergy.goal-cmax-lo = %d\nenergy.goal-cmax-hi = %d\n' \
      "$TMP/seeds" $((CLO*(1000+pm)/1000)) $((CHI*(1000+pm)/1000)); } > "$ld/extra.txt"
  mk_setup "$TL" "$((SEED*1000+pm))" "$ld/extra.txt" > "$ld/s.txt"
  timeout $((TL*4)) "$EXE" "$ld/s.txt" "$INSTANCE" "$ld/" > /dev/null 2>&1
  front=$(ls "$ld"/${stem}_*_Front.csv 2>/dev/null | head -1)
  if [ -n "${front:-}" ]; then
    tail -n +2 "$front" >> "$MERGED"
    { awk -F';' 'NR>1&&NF>=5{print NR";"$5";(0, 0)"}' "$front"; cat "$seedf"; } > "$seedf.t" && mv "$seedf.t" "$seedf"
  fi
done

python3 "$HERE/hv_cost.py" "$MERGED" "$CREF" "$EREF"
