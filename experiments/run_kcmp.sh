#!/bin/bash
# Parallel comparison: insertion vs N8 (k=1,2,3) on one instance.
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
echo "NPROC=$(nproc)"
rm -rf /tmp/par
mkdir -p /tmp/par/ins /tmp/par/k1 /tmp/par/k2 /tmp/par/k3
INST=SelectosYTaillardIntervalos/tai20_20_01.F.15_01.txt

../FuzzyFW experiments/setup_cmp_ins.txt   "$INST" /tmp/par/ins/ >/tmp/par/ins.log 2>&1 &
../FuzzyFW experiments/setup_cmp_n8.txt     "$INST" /tmp/par/k1/  >/tmp/par/k1.log  2>&1 &
../FuzzyFW experiments/setup_cmp_n8_k2.txt  "$INST" /tmp/par/k2/  >/tmp/par/k2.log  2>&1 &
../FuzzyFW experiments/setup_cmp_n8_k3.txt  "$INST" /tmp/par/k3/  >/tmp/par/k3.log  2>&1 &
wait
echo ALLDONE

for c in ins k1 k2 k3 ; do
  echo "==$c=="
  grep -hE 'Best solution' /tmp/par/$c/*.csv 2>/dev/null
  grep -hE 'Total runtime' /tmp/par/$c/*.csv 2>/dev/null
done
echo NAMES
grep -hia reinsertion /tmp/par/k1/*.csv /tmp/par/k2/*.csv /tmp/par/k3/*.csv 2>/dev/null | sort -u
echo SCRIPT_END
