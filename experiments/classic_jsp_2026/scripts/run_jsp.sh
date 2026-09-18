#!/bin/bash
#
# run_jsp.sh — run one setup over a list of classic-JSP Taillard instances and
# verify every schedule it produces.
#
# Usage: run_jsp.sh <setup-file> <results-subdir> <instance> [instance ...]
#
# Each instance runs as its own process; at most MAX_PARALLEL run at a time.
#
# Re-running the script resumes: an instance is skipped only when its
# certificate already holds every run the setup asks for. A certificate left
# behind by a killed process holds fewer, and counting them rather than merely
# checking that the file exists is what keeps a half-finished instance from
# being mistaken for a finished one -- which is how a comparison ends up with
# 54 runs on one configuration and 50 on the other without anything looking
# wrong.
#
# MAX_PARALLEL defaults to the number of cores. Each solver process is
# single-threaded, so one per core is the right setting; override it to leave
# headroom on a shared machine.
#
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
EXE="${ROOT}/../FuzzyFW"
INSTANCES_DIR="${ROOT}/TaillardJSP"
EXPERIMENT_DIR="${ROOT}/experiments/classic_jsp_2026"
SCRIPTS_DIR="${EXPERIMENT_DIR}/scripts"
MAX_PARALLEL="${MAX_PARALLEL:-$(nproc 2>/dev/null || echo 4)}"

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

# How many runs this setup performs, so a partial certificate can be spotted.
EXPECTED_RUNS=$(sed -n 's/^[[:space:]]*runs[[:space:]]*=[[:space:]]*\([0-9]*\).*/\1/p' \
    "${SETUP}" | head -1)
: "${EXPECTED_RUNS:=1}"

completed_runs() {
    local pattern="$1" total=0 file count
    for file in ${pattern}; do
        [ -f "${file}" ] || continue
        count=$(awk -F';' 'NR > 1 { print $1 }' "${file}" | sort -u | wc -l)
        total=$((total + count))
    done
    echo "${total}"
}

for instance in "$@"; do
    done_runs=$(completed_runs "${RESULTS}/${instance}_*_Certificate.csv")
    if [ "${done_runs}" -ge "${EXPECTED_RUNS}" ]; then
        echo "skip ${instance} (${done_runs}/${EXPECTED_RUNS} runs done)"
        continue
    fi
    if [ "${done_runs}" -gt 0 ]; then
        echo "redo ${instance} (only ${done_runs}/${EXPECTED_RUNS} runs; a "\
            "previous attempt was interrupted)"
        rm -f "${RESULTS}/${instance}"_*_Certificate.csv \
              "${RESULTS}/${instance}"_*_Sols.csv
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
