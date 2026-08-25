#!/usr/bin/env bash
# RELANZAMIENTO HOMOGENEO de la Fase 2: los CUATRO solvers por trozos de 5, con
# creation.seed.offset, que hace que trocear asigne las mismas semillas que no
# trocear. Protocolo identico en las 1708 celdas.
#
# Se corrigen a la vez los dos defectos conocidos:
#   - heterogeneidad de protocolo (GA/ABCE3 monoliticos, el GA ademas partido);
#   - reinicio del contador de semillas por proceso (5 poblaciones en vez de 30).
#
# Nada se borra: lo anterior se archiva para poder contrastar ambos regimenes.
#
# Coste (presupuestos de la Fase 2, 7 brazos x 30 runs):
#   GA 550 h, ABCE3 214 h, fEABCLS 1869 h, TSN2 2264 h  ->  ~4900 h de CPU
#   con 14 procesos, del orden de 15 dias.
set -e
cd "$(dirname "$0")/.."
PAR="${PAR:-14}"

echo "=== 0/5 PREVUELO (aborta si algo no esta en orden) ==="
if ! bash final/preflight.sh; then
  echo ""
  echo "El prevuelo señala el archivado pendiente; se hace en el paso 1 y se repite."
fi

STAMP=$(date +%Y%m%d_%H%M%S)
ARCH="final/phase2/_archivo_regimen_previo_$STAMP"
echo ""
echo "=== 1/5 archivando el experimento anterior ==="
mkdir -p "$ARCH"
for a in ga abce3 feabcls tsn2; do
  [ -d "final/phase2/$a" ] && { mv "final/phase2/$a" "$ARCH/$a"; echo "  $a -> $ARCH/$a"; }
  for f in results anytime; do
    [ -f "final/phase2/${f}_$a.csv" ] && cp "final/phase2/${f}_$a.csv" "$ARCH/${f}_$a.csv"
  done
done

echo ""
echo "=== 2/5 prevuelo de nuevo, ahora sin ficheros que puedan eclipsar ==="
bash final/preflight.sh || { echo "PREVUELO FALLIDO: se aborta."; exit 1; }

echo ""
echo "=== 3/5 arrancando el vigilante de memoria ==="
# OJO: powershell.exe NO resuelve rutas relativas de WSL. Hay que convertirla
# con wslpath, o el guard no arranca y el script lo da por lanzado igualmente.
GPS1=$(wslpath -w final/guard.ps1)
before=$(wc -l < final/guard.csv 2>/dev/null || echo 0)
powershell.exe -NoProfile -File "$GPS1" > /dev/null 2>&1 &
sleep 20
after=$(wc -l < final/guard.csv 2>/dev/null || echo 0)
if [ "$after" -gt "$before" ]; then
  echo "  guard ACTIVO y escribiendo (muestras: $before -> $after)"
else
  echo "  AVISO: el guard NO esta escribiendo. La corrida sigue, pero sin red de"
  echo "         seguridad de memoria. Arrancalo a mano en PowerShell:"
  echo "         powershell -NoProfile -File \"$GPS1\""
fi

echo ""
echo "=== 4/5 ejecutando los cuatro solvers, mismo protocolo ==="
echo "    inicio: $(date)"
ALGOS="ga abce3 feabcls tsn2" CH=5 PAR="$PAR" bash final/run_phase2c.sh

echo ""
echo "=== 5/5 extraccion, integridad y curvas ==="
for a in ga abce3 feabcls tsn2; do
  rm -f "final/phase2/results_$a.csv"
  bash final/extract_phase2c.sh "$a"
  awk -F, -v A="$a" 'NR>1{c[$2","$4]++} END{
    for(x in c){ n[c[x]]++; if(c[x]!=30) bad=bad" "x }
    for(k in n) printf "  %s -> %s runs: %d combinaciones\n", A, k, n[k]
    if (bad!="") printf "  %s INCOMPLETAS:%s\n", A, bad
  }' "final/phase2/results_$a.csv"
  bash final/resample_anytime.sh "$a"
done
bash final/verify_broad.sh | tail -4
echo ""
echo "RELANZAMIENTO COMPLETO: $(date)"
echo "Regimen anterior conservado en $ARCH para el control de robustez."
