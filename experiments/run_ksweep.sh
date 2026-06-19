#!/bin/bash
# Sweep N8 perturbation strength k = 1..6 (+ insertion baseline) on one
# instance, all configs in parallel. runs=10.
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
echo "NPROC=$(nproc)"
INST=SelectosYTaillardIntervalos/tai20_20_01.F.15_01.txt
BASE=experiments/setup_sweep_base.txt
W=/tmp/sweep
rm -rf $W
mkdir -p $W

# insertion baseline (same budget)
sed 's/^mutation = ijsp.n8/mutation = insertion/' $BASE > $W/setup_ins.txt
mkdir -p $W/ins
../FuzzyFW $W/setup_ins.txt "$INST" $W/ins/ >$W/ins.log 2>&1 &

# k = 1..6
for k in 1 2 3 4 5 6 ; do
  cp $BASE $W/setup_k$k.txt
  echo "mutation.n8.moves = $k" >> $W/setup_k$k.txt
  mkdir -p $W/k$k
  ../FuzzyFW $W/setup_k$k.txt "$INST" $W/k$k/ >$W/k$k.log 2>&1 &
done

wait
echo ALLDONE

for c in ins k1 k2 k3 k4 k5 k6 ; do
  echo "==$c=="
  grep -hE 'Best solution' $W/$c/*.csv 2>/dev/null
  grep -hE 'Total runtime' $W/$c/*.csv 2>/dev/null
done
echo SCRIPT_END
