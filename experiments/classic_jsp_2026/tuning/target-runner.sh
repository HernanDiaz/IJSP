#!/bin/bash
#
# irace target runner for the crisp solver. Called by irace as
#   target-runner.sh <configuration-id> <instance-id> <seed> <instance> --key value ...
# and expected to print one number: the cost of that configuration on that
# instance with that seed. Lower is better.
#
# The named parameters are substituted into a base setup template (one per
# algorithm, chosen with TEMPLATE) at the IRACE_<KEY> placeholders. The cost is
# the makespan of the best schedule of the run, recomputed from the schedule
# itself -- the maximum completion time in the *_Certificate.csv the solver
# writes -- so a configuration cannot be scored on a number its schedule does
# not support.
#
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXE="${EXE:-$HERE/../../../../FuzzyFW}"
TEMPLATE="${TEMPLATE:?set TEMPLATE to the base setup template}"

CONFIG_ID="$1"; INSTANCE_ID="$2"; SEED="$3"; INSTANCE="$4"; shift 4

WORK=$(mktemp -d /tmp/irace_XXXXXX)
# KEEP=1 leaves the work directory behind, to see why a configuration failed.
[ "${KEEP:-0}" = "1" ] || trap 'rm -rf "$WORK"' EXIT
[ "${KEEP:-0}" = "1" ] && echo "work dir: $WORK" >&2
SETUP="$WORK/setup.txt"
OUT="$WORK/out"; mkdir -p "$OUT"

# Every --key value pair becomes an IRACE_KEY substitution, key upper-cased.
cp "$TEMPLATE" "$SETUP"
sed -i "s/IRACE_SEED/${SEED}/" "$SETUP"
while [ $# -gt 0 ]; do
    key=$(echo "${1#--}" | tr '[:lower:]' '[:upper:]')
    val="$2"
    sed -i "s|IRACE_${key}\b|${val}|g" "$SETUP"
    shift 2
done
# irace passes a conditional parameter only when its condition holds --
# selection.tournament-size only with tournament selection, and so on -- so a
# placeholder left over belongs to a key the chosen configuration does not
# use. Drop the line; the solver never reads it.
sed -i '/IRACE_/d' "$SETUP"

# The solver stops on its own time limit; the guard is for a hung process.
timeout --signal=KILL "${GUARD:-1800}" "$EXE" "$SETUP" "$INSTANCE" "$OUT" > "$WORK/run.log" 2>&1

CERT=$(ls "$OUT"/*_Certificate.csv 2>/dev/null | head -1)
if [ -z "$CERT" ] || [ "$(wc -l < "$CERT")" -lt 2 ]; then
    # A configuration the solver rejects scores Inf and irace drops it, but
    # the reason should be visible in irace's log rather than lost.
    echo "config $CONFIG_ID on $(basename "$INSTANCE"): no schedule -- $(tail -1 "$WORK/run.log")" >&2
    echo Inf; exit 0
fi
# best over runs of (max completion time within the run)
awk -F';' 'NR > 1 { if ($8 > m[$1]) m[$1] = $8 }
           END { best = -1; for (r in m) if (best < 0 || m[r] < best) best = m[r]; print best }' "$CERT"
