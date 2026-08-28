#!/usr/bin/env bash
# VERIFICACION AMPLIA con el decodificador INDEPENDIENTE (verify_independent),
# que no enlaza ninguna clase del solver. Recorre la misma muestra sistematica
# que verify_broad.sh --- 4 solvers x 7 brazos x una instancia de cada clase ---
# de modo que ambos resultados son directamente comparables.
#
# Cada invocacion se ejecuta con LIMITES DUROS de memoria y tiempo: un fallo en
# el verificador debe matar su proceso, no la maquina.
cd "$(dirname "$0")/.."
INSTS="ft10 tai15_15_03 tai20_20_05 tai30_15_07 tai30_20_01 tai50_15_04 tai50_20_09"
ARMS="A0 V2H V2 MOR GT GP MIX MIXH"
MODO=${1:-insertion}
LIMV=${LIMV:-1048576}     # 1 GB de memoria virtual por proceso
LIMT=${LIMT:-60}          # 60 s de CPU por proceso

tot=0; bad=0; cells=0; fallos=0
: > final/verify_indep_detalle.txt
for algo in ga abce3 feabcls tsn2; do
  for inst in $INSTS; do
    if [ "$inst" = ft10 ]; then f="SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt"
    else f="SelectosYTaillardIntervalos/$inst.F.15_01.txt"; fi
    for arm in $ARMS; do
      base="final/phase2/$algo/$inst/$arm"
      if [ -n "$(ls -d "$base"/c* 2>/dev/null)" ]; then
        s=$(ls "$base"/c1/*_Sols.csv 2>/dev/null | head -1)
      else
        s=$(ls "$base"/*_Sols.csv 2>/dev/null | head -1)
      fi
      [ -z "$s" ] && continue
      out=$( ( ulimit -v "$LIMV" -t "$LIMT"; timeout $((LIMT+15)) \
               ./verify_independent "$f" "$s" "$MODO" ) 2>&1 )
      rc=$?
      if [ $rc -ge 124 ] || echo "$out" | grep -qi 'bad_alloc\|Killed\|terminate'; then
        echo "  ABORTADO $algo/$inst/$arm (rc=$rc)" | tee -a final/verify_indep_detalle.txt
        fallos=$((fallos+1)); continue
      fi
      n=$(echo "$out" | grep -oE 'runs verificados: [0-9]+' | grep -oE '[0-9]+')
      d=$(echo "$out" | grep -oE 'discrepancias: [0-9]+' | grep -oE '[0-9]+')
      [ -z "$n" ] && n=0; [ -z "$d" ] && d=0
      tot=$((tot+n)); bad=$((bad+d)); cells=$((cells+1))
      if [ "$d" -gt 0 ]; then
        echo "DISCREPANCIA $algo/$inst/$arm: $d de $n" >> final/verify_indep_detalle.txt
        echo "$out" | awk '$4 != "+0.00" && $1 ~ /^[0-9]+$/' >> final/verify_indep_detalle.txt
        echo "  DISCREPANCIA en $algo/$inst/$arm: $d de $n"
      fi
    done
  done
done
echo ""
echo "modo: $MODO"
echo "celdas verificadas: $cells   (procesos abortados: $fallos)"
echo "soluciones re-decodificadas: $tot"
echo "discrepancias: $bad"
echo "detalle en final/verify_indep_detalle.txt"
