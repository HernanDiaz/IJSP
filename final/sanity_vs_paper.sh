#!/usr/bin/env bash
# Contraste de cordura: nuestros resultados frente a los datos publicados del
# grupo para las MISMAS instancias (Papers/COR_Tabu/data/phaseA/all_results_full.csv,
# que trae best_mid = mejor E[Cmax] encontrado por cada configuracion del paper).
cd "$(dirname "$0")/.."
SRC="/mnt/c/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu/data/phaseA/all_results_full.csv"
echo "=== rango de best_mid publicado por instancia (todas las configs del paper) ==="
awk -F',' 'NR>1{gsub(/"/,""); inst=$1; sub(/\.F\.15_01$/,"",inst);
  v=$3+0;  # best_mid
  if (!(inst in mn) || v<mn[inst]) mn[inst]=v;
  if (!(inst in mx) || v>mx[inst]) mx[inst]=v;
  bk[inst]=$13+0 }
  END{ for (i in mn) if (i ~ /^tai(30_20|50_20)_0[1-3]$/)
        printf "  %-14s publicado: mejor=%.1f peor=%.1f | best_known=%.1f\n", i, mn[i], mx[i], bk[i] }' "$SRC" | sort
echo ""
echo "=== nuestro mejor E[Cmax] (control A0, sin sembrar) en esas instancias ==="
for algo in abce3 ga feabcls; do
  for inst in tai30_20_01 tai30_20_02 tai30_20_03 tai50_20_01 tai50_20_02 tai50_20_03; do
    f=final/phase2/results_${algo}.csv
    [ -f "$f" ] || continue
    awk -F',' -v a=$algo -v i=$inst '$1==a && $2==i && $4=="A0"{ if(b==""||$6<b) b=$6 }
      END{ if(b!="") printf "  %-8s %-14s mejor de 30 runs: %.1f\n", a, i, b }' "$f"
  done
done
