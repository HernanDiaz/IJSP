#!/bin/bash
# Monitor de los baselines tuneados (EXP4_TUNED). Snapshot: runs activos +
# progreso de las 6 instancias pesadas que quedan (tai50x20). Reejecutable.
cd /mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP/experiments/mo_green_2026 || exit 1
A=$(ps -ef | grep 'Fable/FuzzyFW' | grep -v timeout | grep -v grep | wc -l)
echo "== baselines: $A runs activos   ($(date '+%H:%M:%S %d-%b')) =="
for arm in ABC-P MA-P; do
  printf "%-6s" "$arm"
  for i in 05 06 07 08 09 10; do
    n=$(ls results/EXP4_TUNED/$arm/tai50_20_${i}.F.15_01_*_Front.csv 2>/dev/null | wc -l)
    printf "  %s:%2s/30" "$i" "$n"
  done
  echo
done
if [ "$A" -eq 0 ]; then
  # comprobar completitud global 82/82 x 30
  tot=0
  for arm in ABC-P MA-P; do
    c=$(ls results/EXP4_TUNED/$arm/*_Front.csv 2>/dev/null | wc -l); tot=$((tot+c))
  done
  echo ">>> TODO TERMINADO — $tot ficheros _Front (esperado ~4920 = 2x82x30) <<<"
fi
