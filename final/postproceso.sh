#!/usr/bin/env bash
# Paso 5/5 del relanzamiento, que quedo sin ejecutar porque el script padre murio
# al cerrarse la terminal interactiva. El runner SI termino (PHASE2C DONE).
# Extrae los cuatro solvers, comprueba integridad, regenera curvas anytime y
# verifica makespans.
cd "$(dirname "$0")/.."
LOG=final/postproceso.log
say() { echo "[$(date '+%m-%d %H:%M:%S')] $*" | tee -a "$LOG"; }
R="/mnt/c/Program Files/R/R-4.5.3/bin/Rscript.exe"

say "=== 1/4 extraccion e integridad ==="
for a in ga abce3 feabcls tsn2; do
  rm -f "final/phase2/results_$a.csv"
  bash final/extract_phase2c.sh "$a" 2>&1 | tee -a "$LOG"
  awk -F, -v A="$a" 'NR>1{c[$2","$4]++} END{
    n=0; mal=0; for(x in c){n++; if(c[x]!=30){mal++; bad=bad" "x}}
    printf "  %s: %d celdas, %d con runs != 30%s\n", A, n, mal, (bad!=""?" ->"bad:"")
  }' "final/phase2/results_$a.csv" 2>&1 | tee -a "$LOG"
done

say "=== 2/4 curvas anytime ==="
for a in ga abce3 feabcls tsn2; do
  bash final/resample_anytime.sh "$a" 2>&1 | tail -1 | tee -a "$LOG"
done

say "=== 3/4 verificacion amplia de makespans ==="
bash final/verify_broad.sh 2>&1 | tail -5 | tee -a "$LOG"

say "=== 4/4 analisis ==="
"$R" final/rpd_vs_lb.R 2>&1 | tee -a "$LOG"
for a in ga abce3 feabcls tsn2; do
  echo "" | tee -a "$LOG"; say "### $a ###"
  "$R" final/phase2_stats.R "final/phase2/results_$a.csv" 2>&1 | head -12 | tee -a "$LOG"
done
"$R" final/generator_ranking.R 2>&1 | tee -a "$LOG"

say "=== POSTPROCESO COMPLETO ==="
