#!/usr/bin/env bash
cd "$(dirname "$0")/.."
for a in abce3 feabc feabcls; do
  echo "===== $a ====="
  rm -rf "repro/smoke_$a"; mkdir -p "repro/smoke_$a"
  ./FuzzyFW "repro/smoke_$a.txt" SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt "repro/smoke_$a" > "repro/smoke_$a/log.txt" 2>&1
  echo "exit=$?"
  grep -h -i -E 'error|exception|finished' "repro/smoke_$a/log.txt" | head -3
  s=$(ls "repro/smoke_$a"/*_Sols.csv 2>/dev/null | head -1)
  if [ -n "$s" ]; then
    echo "Objective values (E[Cmax] por run):"
    tail -n +2 "$s" | sed 's/.*;//'
  else
    echo "SIN _Sols.csv"
  fi
done
