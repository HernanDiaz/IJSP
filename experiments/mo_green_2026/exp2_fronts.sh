#!/bin/bash
#
# exp2_fronts.sh — Experiment 2: extended exact epsilon-constraint fronts.
# 6 small/medium instances x 11 epsilon points (fine permil grid near the
# makespan-optimal end), 120 s CP-SAT per point => worst case ~2.5 h total,
# single job (CP-SAT uses 4 workers internally). Appends machine-readable
# points to results/fronts.csv for the distance-to-front analysis.
#
# Launch detached (survives window close / WSL teardown):
#   wsl bash -c "setsid nohup bash /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/exp2_fronts.sh > /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026/exp2_fronts.log 2>&1 < /dev/null &"
#
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
cd "$REPO"
export PROBE_TIME=120
export PROBE_CSV="$REPO/experiments/mo_green_2026/results/fronts.csv"

~/ortools-venv/bin/python3 experiments/mo_green_2026/epsilon_probe.py \
    F0.15.0.ft10_10 \
    F0.15.0.la21_04 \
    F0.15.0.la24_03 \
    F0.15.0.la25_04 \
    F0.15.0.la29_03 \
    tai15_15_01.F.15_01

echo "EXP2 DONE"
