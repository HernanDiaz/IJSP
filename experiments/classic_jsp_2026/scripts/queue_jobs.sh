#!/bin/bash
#
# queue_jobs.sh — run a list of (setup, results-tag, instance) jobs through
# one queue, at most one solver per core, and verify every schedule.
#
# Usage: queue_jobs.sh <jobs.tsv>
#   each line:  <setup-file> <TAB> <results-tag> <TAB> <instance>
#   results go to experiments/classic_jsp_2026/results/<results-tag>/
#
# The generalisation of sequential_compare.sh to any number of cells: the
# caller decides the order of the lines, and interleaving the cells is what
# spreads them evenly over the wall-clock window so that the machine's ~9 %
# drift between batches hits every cell alike. At most SLOTS jobs run at a
# time (default: the number of cores), never two batches stacked, and the
# script refuses to start while another solver or an irace tuning is
# running: a wall-clock budget shared with another experiment is a different
# budget.
#
# Re-running resumes: a job is skipped only when its schedule file already
# holds every run the setup asks for; a partial one is redone.
#
set -u

[ $# -eq 1 ] && [ -f "$1" ] || { echo "usage: $0 <jobs.tsv>" >&2; exit 2; }
JOBS="$1"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
EXE="${ROOT}/../FuzzyFW"
INSTANCES_DIR="${ROOT}/TaillardJSP"
EXPERIMENT_DIR="${ROOT}/experiments/classic_jsp_2026"
SCRIPTS_DIR="${EXPERIMENT_DIR}/scripts"
SLOTS="${SLOTS:-$(nproc 2>/dev/null || echo 4)}"

[ -x "${EXE}" ] || { echo "error: solver not built at ${EXE}" >&2; exit 1; }

busy=$(pgrep -c FuzzyFW 2>/dev/null || echo 0)
tuning=$(pgrep -fc 'library\(irace\)' 2>/dev/null || echo 0)
if [ "${busy}" -gt 0 ] || [ "${tuning}" -gt 0 ]; then
    echo "error: the machine is busy (${busy} solver(s), ${tuning} irace" \
         "process(es)); one experiment at a time." >&2
    exit 1
fi

runs_asked() {
    local n
    n=$(sed -n 's/^[[:space:]]*runs[[:space:]]*=[[:space:]]*\([0-9]*\).*/\1/p' "$1" | head -1)
    echo "${n:-1}"
}
completed_runs() {
    local pattern="$1" total=0 file count
    for file in ${pattern}; do
        [ -f "${file}" ] || continue
        count=$(awk -F';' 'NR > 1 { print $1 }' "${file}" | sort -u | wc -l)
        total=$((total + count))
    done
    echo "${total}"
}

mapfile -t LINES < <(grep -v '^\s*#' "${JOBS}" | grep -v '^\s*$')
echo "${#LINES[@]} jobs, ${SLOTS} at a time, started $(date)"

started=0; skipped=0
for line in "${LINES[@]}"; do
    IFS=$'\t' read -r setup tag instance <<< "${line}"
    [ -f "${setup}" ] || { echo "error: setup ${setup} not found" >&2; exit 1; }
    dir="${EXPERIMENT_DIR}/results/${tag}"; mkdir -p "${dir}"
    want=$(runs_asked "${setup}")
    have=$(completed_runs "${dir}/${instance}_*_Certificate.csv")
    if [ "${have}" -ge "${want}" ]; then
        skipped=$((skipped + 1)); continue
    fi
    if [ "${have}" -gt 0 ]; then
        echo "redo  ${tag}/${instance} (only ${have}/${want} runs)"
        rm -f "${dir}/${instance}"_*.csv
    fi
    while [ "$(jobs -rp | wc -l)" -ge "${SLOTS}" ]; do wait -n; done
    echo "start ${tag}/${instance} ($(date +%H:%M:%S))"
    "${EXE}" "${setup}" "${INSTANCES_DIR}/${instance}.txt" "${dir}" \
        > "${dir}/${instance}.log" 2>&1 &
    started=$((started + 1))
done
wait
echo "${started} jobs run, ${skipped} already done, finished $(date)"

echo
echo "=== verification ==="
status=0
for line in "${LINES[@]}"; do
    IFS=$'\t' read -r setup tag instance <<< "${line}"
    dir="${EXPERIMENT_DIR}/results/${tag}"
    certificate=$(ls -t "${dir}/${instance}"_*_Certificate.csv 2>/dev/null | head -1)
    if [ -z "${certificate}" ]; then
        echo "${tag}/${instance}: NO SCHEDULE (see ${instance}.log)"; status=1; continue
    fi
    echo -n "${tag}/"
    python3 "${SCRIPTS_DIR}/verify_certificate.py" \
        --orlib "${EXPERIMENT_DIR}/reference/taillard_orlib.txt" \
        --instance "${instance}" --certificate "${certificate}" \
        --bounds "${EXPERIMENT_DIR}/taillard_bounds.csv" --quiet || status=1
done
exit "${status}"
