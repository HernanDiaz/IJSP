#!/bin/bash
#
# run_jsp.sh — run one setup over a list of classic-JSP Taillard instances and
# verify every schedule it produces.
#
# Usage: run_jsp.sh <setup-file> <results-subdir> <instance> [instance ...]
#
# Each instance runs as its own process; at most MAX_PARALLEL run at a time.
# An instance whose certificate already exists is skipped, so the script is
# safe to re-run after an interruption.
#
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
EXE="${ROOT}/../FuzzyFW"
INSTANCES_DIR="${ROOT}/TaillardJSP"
EXPERIMENT_DIR="${ROOT}/experiments/classic_jsp_2026"
SCRIPTS_DIR="${EXPERIMENT_DIR}/scripts"
MAX_PARALLEL="${MAX_PARALLEL:-4}"

if [ $# -lt 3 ]; then
    echo "usage: $0 <setup-file> <results-subdir> <instance> [instance ...]" >&2
    exit 2
fi

SETUP="$1"; shift
RESULTS="${EXPERIMENT_DIR}/results/$1"; shift

if [ ! -x "${EXE}" ]; then
    echo "error: solver not built at ${EXE} (run make first)" >&2
    exit 1
fi
if [ ! -f "${SETUP}" ]; then
    echo "error: setup file ${SETUP} not found" >&2
    exit 1
fi

mkdir -p "${RESULTS}"

for instance in "$@"; do
    if compgen -G "${RESULTS}/${instance}_*_Certificate.csv" > /dev/null; then
        echo "skip ${instance} (already has a certificate)"
        continue
    fi
    while [ "$(jobs -rp | wc -l)" -ge "${MAX_PARALLEL}" ]; do wait -n; done
    echo "start ${instance}"
    "${EXE}" "${SETUP}" "${INSTANCES_DIR}/${instance}.txt" "${RESULTS}" \
        > "${RESULTS}/${instance}.log" 2>&1 &
done
wait

echo
echo "=== verification ==="
status=0
for instance in "$@"; do
    certificate=$(ls -t "${RESULTS}/${instance}"_*_Certificate.csv 2>/dev/null \
        | head -1)
    if [ -z "${certificate}" ]; then
        echo "${instance}: NO CERTIFICATE (run failed, see ${instance}.log)"
        status=1
        continue
    fi
    python3 "${SCRIPTS_DIR}/verify_certificate.py" \
        --orlib "${EXPERIMENT_DIR}/reference/taillard_orlib.txt" \
        --instance "${instance}" \
        --certificate "${certificate}" \
        --bounds "${EXPERIMENT_DIR}/taillard_bounds.csv" \
        --quiet || status=1
done
exit ${status}
