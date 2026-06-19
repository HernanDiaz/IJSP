#!/bin/bash
# Validate the 4 irace-tuned GA configs (best-vs-best) on the 20 instances.
set -u
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
BASE=experiments/irace_ga/base_setup_ga.txt
EXE=../FuzzyFW
W=/tmp/valga
RUNS=10
MAXJOBS=24
rm -rf "$W"; mkdir -p "$W"

mapfile -t INSTANCES < <(grep -v '^#' experiments/irace_ga/instances.txt | grep -v '^[[:space:]]*$')

# name  mutation   crossover  pop  cxpb    mutpb   tourn  n8moves
CONFIGS=(
  "N8 ijsp.n8 ijsp.jox 384 0.9857 0.2436 2 2"
  "Swap swap ijsp.gox 340 0.9684 0.3793 4 1"
  "Insertion insertion ijsp.gox 372 0.9871 0.3554 2 1"
  "Inversion inversion ijsp.gox 318 0.9695 0.3729 2 1"
)

for cfg in "${CONFIGS[@]}"; do
  read -r name mut cx pop cxpb mutpb tourn k <<< "$cfg"
  setup="$W/setup_${name}.txt"
  sed -e "s/IRACE_SEED/1/" -e "s|IRACE_MUTATION|${mut}|" -e "s|IRACE_CROSSOVER|${cx}|" \
      -e "s/IRACE_N8MOVES/${k}/" -e "s/IRACE_POP/${pop}/" -e "s/IRACE_CXPB/${cxpb}/" \
      -e "s/IRACE_MUTPB/${mutpb}/" -e "s/IRACE_TOURN/${tourn}/" \
      -e "s/^runs = 1$/runs = ${RUNS}/" "$BASE" > "$setup"
  for inst in "${INSTANCES[@]}"; do
    stem="$(basename "${inst%.txt}")"
    mkdir -p "$W/${name}/${stem}"
    while [ "$(jobs -rp | wc -l)" -ge "$MAXJOBS" ]; do wait -n; done
    ( "$EXE" "$setup" "$inst" "$W/${name}/${stem}/" >/dev/null 2>&1 ) &
  done
done
wait
echo "RUNS_DONE"

for cfg in "${CONFIGS[@]}"; do
  read -r name rest <<< "$cfg"
  for inst in "${INSTANCES[@]}"; do
    stem="$(basename "${inst%.txt}")"
    line=$(grep -hE 'Best solution' "$W/${name}/${stem}"/*.csv 2>/dev/null | head -1)
    echo "${name}|${stem}|${line}"
  done
done
echo "SCRIPT_END"
