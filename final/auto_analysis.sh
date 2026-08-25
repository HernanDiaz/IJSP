#!/usr/bin/env bash
# Espera a que TERMINE la Fase 2C y solo entonces lanza el analisis completo.
#
# Por que asi:
#  - El sondeo es GRATIS en E/S: mira el contador de procesos (ps), NO lista
#    miles de ficheros en /mnt/c. Recorrer /mnt/c en WSL1 le roba E/S a los
#    chunks en vuelo, y con presupuesto POR TIEMPO eso les resta busqueda real.
#  - Solo se considera terminado cuando no queda ni un FuzzyFW ni el runner.
#  - Se exige ademas que esten los 2562 trozos; si faltan, se avisa y se
#    analiza igualmente lo que haya, dejandolo dicho en el informe.
cd "$(dirname "$0")/.."
LOG=final/auto_analysis.log
R="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"
say() { echo "[$(date '+%m-%d %H:%M:%S')] $*" | tee -a "$LOG"; }

say "=== vigilando el final de la Fase 2C (sondeo cada 3 min, sin tocar /mnt/c) ==="
while :; do
  n=$(ps -eo comm --no-headers | grep -c '^FuzzyFW$')
  r=$(ps -eo args --no-headers | grep -c '[r]un_phase2c')
  [ "$n" -eq 0 ] && [ "$r" -eq 0 ] && break
  sleep 180
done
say "ejecucion terminada (0 FuzzyFW, 0 runner). Esperando 60 s a que se cierren ficheros."
sleep 60

fin=$(ls final/phase2/tsn2/*/*/c*/*_Sols.csv 2>/dev/null | wc -l)
say "chunks TSN2 presentes: $fin / 2562"
[ "$fin" -lt 2562 ] && say "AVISO: faltan $((2562-fin)) trozos; el analisis se hace sobre lo disponible."

say "--- 1/4 extraccion de TSN2 (se descarta el CSV a medias anterior) ---"
rm -f final/phase2/results_tsn2.csv
bash final/extract_phase2c.sh tsn2 2>&1 | tee -a "$LOG"

say "--- comprobacion de integridad: 30 runs por instancia y brazo ---"
awk -F, 'NR>1{c[$2","$4]++} END{
  for(x in c){ n[c[x]]++; if(c[x]!=30) bad=bad" "x }
  for(k in n) printf "  %d runs: %d combinaciones\n", k, n[k]
  if (bad!="") printf "  INCOMPLETAS:%s\n", bad
}' final/phase2/results_tsn2.csv 2>&1 | tee -a "$LOG"

say "--- 2/4 RPD de los cuatro algoritmos sobre las cotas inferiores publicadas ---"
"$R" final/rpd_vs_lb.R 2>&1 | tee -a "$LOG"

say "--- 3/4 Wilcoxon pareado + Holm + A12, por algoritmo ---"
for a in ga abce3 feabcls tsn2; do
  f="final/phase2/results_${a}.csv"
  [ -f "$f" ] || continue
  echo "" | tee -a "$LOG"; say "### $a ###"
  "$R" final/phase2_stats.R "$f" 2>&1 | tee -a "$LOG"
done

say "--- 4/4 tablas y figuras (fig_tables.R exige el algoritmo como argumento) ---"
for a in ga abce3 feabcls tsn2; do
  [ -f "final/phase2/results_${a}.csv" ] || continue
  "$R" final/fig_tables.R "$a" 2>&1 | tee -a "$LOG"
done

say "=== ANALISIS COMPLETO ==="
