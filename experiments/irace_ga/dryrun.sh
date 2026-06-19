#!/bin/bash
# Validate the irace_ga chain before launching the full tuning.
cd "$(dirname "$0")"
echo "=== syntax ==="
bash -n target-runner.sh && echo "target-runner.sh ok"
bash -n run_irace_ga.sh && echo "run_irace_ga.sh ok"

echo "=== binary copy ==="
cp /mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW /home/diazhernan/FuzzyFW_ga
chmod +x /home/diazhernan/FuzzyFW_ga
ls -la /home/diazhernan/FuzzyFW_ga

echo "=== dry-run target-runner (mutation = N8) ==="
sed 's|MUT="MUT_OPERATOR"|MUT="ijsp.n8"|' target-runner.sh > ./tr_test_tmp.sh
chmod +x ./tr_test_tmp.sh
echo -n "objective returned: "
bash ./tr_test_tmp.sh 1 1 1 \
  /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/SelectosYTaillardIntervalos/tai20_20_01.F.15_01.txt \
  --pop 200 --cxpb 0.9 --mutpb 0.1 --tourn 3 --crossover ijsp.jox --n8moves 3
echo ""
echo "(a number ~1700-1850 = OK; 'Inf' = something is wrong)"
rm -f ./tr_test_tmp.sh
