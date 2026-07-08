#!/bin/bash
# Self-contained detached launcher for the Exp.4 baselines, classical tier.
# Survives session/window close (setsid nohup). Idempotent-ish: the runner
# skips (instance,arm) pairs that already have a final CSV.
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
D=experiments/mo_green_2026
tr -d '\r' < "$D/run_exp4_baselines.sh" > /tmp/exp4b.sh
setsid nohup bash /tmp/exp4b.sh classical 14 \
    > "$D/exp4_baselines_classical.out" 2>&1 < /dev/null &
echo "launched pid $! -> $D/exp4_baselines_classical.out"
