#!/bin/bash
# Runs the QEA 30x on classical instances.
#   QEA_SETUP : setup file (default setup_qea30.txt)
#   QEA_OUT   : output root  (default run30)
# With no positional args -> all 12 instances (cleans QEA_OUT first).
# With args -> only those instance file-stems (for quick testing).
cd /mnt/c/Users/diazhernan/CLionProjects/IJSP

SETUP="${QEA_SETUP:-experiments/qea_test/setup_qea30.txt}"
OUTROOT="${QEA_OUT:-experiments/qea_test/run30}"

if [ -z "$1" ]; then
  INSTS="abz7_06 abz8_05 abz9_10 ft10_10 ft20_05 la21_04 la24_03 la25_04 la27_09 la29_03 la38_06 la40_05"
  rm -rf "$OUTROOT"
else
  INSTS="$@"
fi

for inst in $INSTS; do
  ld="$OUTROOT/$inst"
  rm -rf "$ld"
  mkdir -p "$ld"
  ( ../FuzzyFW "$SETUP" \
      "SelectosYTaillardIntervalos/F0.15.0.$inst.txt" \
      "$ld" > "$ld/stdout.log" 2>&1 ) &
done
wait
echo ALL_DONE
