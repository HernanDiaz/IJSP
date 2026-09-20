#!/bin/bash
#
# sequential_compare.sh — run two arms of a comparison through one queue.
#
# Usage: sequential_compare.sh <setup-A> <setup-B> <tag-A> <tag-B> <instance>...
#
# Why a queue rather than two batches. Every setup here stops on wall-clock
# time, so how much search fits in a run depends on what else the machine is
# doing, and this machine drifts by about 9 % from one batch to the next
# (JOURNAL.md, 2026-09-19: the same binary, same md5, measured 4.122 and 3.812
# generations per second in two batches). Running arm A to completion and then
# arm B would confound the arms with when each ran -- the mistake that produced
# this directory's one retraction. Running both at once, as paired_setups.sh
# does, cures that but stacks two batches on the machine.
#
# This script does neither. There is one queue holding every job of both arms,
# alternating between them, and at most SLOTS jobs run at a time -- one solver
# per core, never two batches stacked. A job finishes, the next one starts.
# The arms are spread evenly over the whole wall-clock window, so whatever the
# machine does to one it does to the other, and the load never exceeds what a
# single batch would put on it.
#
# One job is one setup on one instance, which is all of that setup's runs on
# it, because the solver seeds its runs from the setup's seed and splitting
# them across processes would change the seeds.
#
# SLOTS defaults to the number of cores. The script refuses to start while
# another solver or an irace tuning is running, so that two experiments cannot
# end up sharing the machine by accident.
#
set -u

if [ $# -lt 5 ]; then
    echo "usage: $0 <setup-A> <setup-B> <tag-A> <tag-B> <instance> [instance ...]" >&2
    exit 2
fi

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
EXE="${ROOT}/../FuzzyFW"
INSTANCES_DIR="${ROOT}/TaillardJSP"
EXPERIMENT_DIR="${ROOT}/experiments/classic_jsp_2026"
SCRIPTS_DIR="${EXPERIMENT_DIR}/scripts"
SLOTS="${SLOTS:-$(nproc 2>/dev/null || echo 4)}"

SETUP_A="$1"; SETUP_B="$2"; TAG_A="$3"; TAG_B="$4"; shift 4
INSTANCES=("$@")

[ -x "${EXE}" ] || { echo "error: solver not built at ${EXE}" >&2; exit 1; }
for s in "${SETUP_A}" "${SETUP_B}"; do
    [ -f "$s" ] || { echo "error: setup $s not found" >&2; exit 1; }
done

# Nothing else may be on the machine: a wall-clock budget shared with another
# experiment is a different budget.
busy=$(pgrep -c FuzzyFW 2>/dev/null || echo 0)
tuning=$(pgrep -fc 'library\(irace\)' 2>/dev/null || echo 0)
if [ "${busy}" -gt 0 ] || [ "${tuning}" -gt 0 ]; then
    echo "error: the machine is busy (${busy} solver(s), ${tuning} irace" \
         "process(es)). This comparison runs on an idle machine or not at" \
         "all; wait for the other experiment to finish." >&2
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

RUNS_A=$(runs_asked "${SETUP_A}")
RUNS_B=$(runs_asked "${SETUP_B}")
DIR_A="${EXPERIMENT_DIR}/results/${TAG_A}"
DIR_B="${EXPERIMENT_DIR}/results/${TAG_B}"
mkdir -p "${DIR_A}" "${DIR_B}"

# The queue: every job of both arms, alternating, so that neither arm is
# concentrated at one end of the wall-clock window.
jobs_setup=(); jobs_dir=(); jobs_tag=(); jobs_instance=(); jobs_runs=()
for instance in "${INSTANCES[@]}"; do
    jobs_setup+=("${SETUP_A}" "${SETUP_B}")
    jobs_dir+=("${DIR_A}" "${DIR_B}")
    jobs_tag+=("${TAG_A}" "${TAG_B}")
    jobs_instance+=("${instance}" "${instance}")
    jobs_runs+=("${RUNS_A}" "${RUNS_B}")
done

echo "A: ${SETUP_A} -> results/${TAG_A} (${RUNS_A} runs/instance)"
echo "B: ${SETUP_B} -> results/${TAG_B} (${RUNS_B} runs/instance)"
echo "${#jobs_setup[@]} jobs, ${SLOTS} at a time, started $(date)"
echo

started=0; skipped=0
for i in "${!jobs_setup[@]}"; do
    instance="${jobs_instance[$i]}"
    dir="${jobs_dir[$i]}"
    want="${jobs_runs[$i]}"
    have=$(completed_runs "${dir}/${instance}_*_Certificate.csv")
    if [ "${have}" -ge "${want}" ]; then
        echo "skip  ${jobs_tag[$i]}/${instance} (${have}/${want} runs done)"
        skipped=$((skipped + 1))
        continue
    fi
    if [ "${have}" -gt 0 ]; then
        echo "redo  ${jobs_tag[$i]}/${instance} (only ${have}/${want} runs;" \
             "a previous attempt was interrupted)"
        rm -f "${dir}/${instance}"_*_Certificate.csv "${dir}/${instance}"_*_Sols.csv
    fi
    while [ "$(jobs -rp | wc -l)" -ge "${SLOTS}" ]; do wait -n; done
    echo "start ${jobs_tag[$i]}/${instance} ($(date +%H:%M:%S))"
    "${EXE}" "${jobs_setup[$i]}" "${INSTANCES_DIR}/${instance}.txt" "${dir}" \
        > "${dir}/${instance}.log" 2>&1 &
    started=$((started + 1))
done
wait
echo
echo "${started} jobs run, ${skipped} already done, finished $(date)"

echo
echo "=== verification ==="
status=0
for i in "${!jobs_setup[@]}"; do
    instance="${jobs_instance[$i]}"
    dir="${jobs_dir[$i]}"
    certificate=$(ls -t "${dir}/${instance}"_*_Certificate.csv 2>/dev/null | head -1)
    if [ -z "${certificate}" ]; then
        echo "${jobs_tag[$i]}/${instance}: NO SCHEDULE (see ${instance}.log)"
        status=1
        continue
    fi
    echo -n "${jobs_tag[$i]}/"
    python3 "${SCRIPTS_DIR}/verify_certificate.py" \
        --orlib "${EXPERIMENT_DIR}/reference/taillard_orlib.txt" \
        --instance "${instance}" \
        --certificate "${certificate}" \
        --bounds "${EXPERIMENT_DIR}/taillard_bounds.csv" \
        --quiet || status=1
done
exit "${status}"
