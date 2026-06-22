#!/usr/bin/env bash
# Regenerate the N2N8 warm-seed bank from the COR paper's stored solutions
# (best-of-run task orders of tuned TS-N2 and TS-N8, in the IJSP clone's exp4).
# Pure copy + concatenation, no solving. Out: seeds/{N2_tuned,N8_tuned,N2N8_tuned}.
set -eu
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/iprts_2026
EXP4=/mnt/c/Users/diazhernan/CLionProjects/IJSP/experiments/results/exp4
prep() { bash <(tr -d '\r' < scripts/prepare_seeds.sh) "$1" "$2"; }
prep "$EXP4/N2_tuned" seeds/N2_tuned
prep "$EXP4/N8_tuned" seeds/N8_tuned
bash <(tr -d '\r' < scripts/merge_seeds.sh) seeds/N2_tuned seeds/N8_tuned seeds/N2N8_tuned
echo "--- counts ---"
echo "N2:        $(ls seeds/N2_tuned/*_Sols.csv 2>/dev/null | wc -l)"
echo "N8:        $(ls seeds/N8_tuned/*_Sols.csv 2>/dev/null | wc -l)"
echo "N2N8:      $(ls seeds/N2N8_tuned/*_Sols.csv 2>/dev/null | wc -l)"
echo "N2N8 x20:  $(ls seeds/N2N8_tuned/tai30_20_*_Sols.csv seeds/N2N8_tuned/tai50_20_*_Sols.csv 2>/dev/null | wc -l)"
