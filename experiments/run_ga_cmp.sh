#!/bin/bash
# GA comparison: insertion vs N8 (k=1, k=3) on one instance, in parallel.
# Pure GA (generational replacement) so mutations persist and can diversify.
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP || exit 1
echo "NPROC=$(nproc)"
INST=SelectosYTaillardIntervalos/tai20_20_01.F.15_01.txt
BASE=experiments/setup_ga_base.txt
W=/tmp/gacmp
rm -rf $W
mkdir -p $W

# insertion baseline
sed 's/^mutation = ijsp.n8/mutation = insertion/' $BASE > $W/setup_ins.txt
mkdir -p $W/ins
../FuzzyFW $W/setup_ins.txt "$INST" $W/ins/ >$W/ins.log 2>&1 &

# N8 with k = 1 and k = 3
for k in 1 3 ; do
  cp $BASE $W/setup_k$k.txt
  echo "mutation.n8.moves = $k" >> $W/setup_k$k.txt
  mkdir -p $W/k$k
  ../FuzzyFW $W/setup_k$k.txt "$INST" $W/k$k/ >$W/k$k.log 2>&1 &
done

wait
echo ALLDONE
for c in ins k1 k3 ; do
  echo "==$c=="
  grep -hE 'Best solution' $W/$c/*.csv 2>/dev/null
  grep -hE 'Total runtime' $W/$c/*.csv 2>/dev/null
done
echo SCRIPT_END
