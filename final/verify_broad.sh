#!/usr/bin/env bash
# VERIFICACION AMPLIA del makespan: re-decodifica soluciones almacenadas con una
# implementacion independiente (insertion SGS + maximo componentwise) y compara
# con el E[Cmax] reportado. La version anterior cubria 4 celdas; esta barre los
# 4 solvers x 7 brazos x una instancia de cada clase de tamano.
# Da PRIORIDAD A LOS CHUNKS, como el resto del pipeline ya corregido.
cd "$(dirname "$0")/.."
INSTS="ft10 tai15_15_03 tai20_20_05 tai30_15_07 tai30_20_01 tai50_15_04 tai50_20_09"
ARMS="A0 V2H V2 MOR GT GP MIX"
tot=0; bad=0; cells=0; sincells=0
for algo in ga abce3 feabcls tsn2; do
  for inst in $INSTS; do
    for arm in $ARMS; do
      base="final/phase2/$algo/$inst/$arm"
      if [ -n "$(ls -d "$base"/c* 2>/dev/null)" ]; then
        s=$(ls "$base"/c1/*_Sols.csv 2>/dev/null | head -1)
      else
        s=$(ls "$base"/*_Sols.csv 2>/dev/null | head -1)
      fi
      [ -z "$s" ] && { sincells=$((sincells+1)); continue; }
      if [ "$inst" = ft10 ]; then f="SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt"
      else f="SelectosYTaillardIntervalos/$inst.F.15_01.txt"; fi
      out=$(./verify_solution "$f" "$s" insertion 2>&1)
      n=$(echo "$out" | grep -oE "runs verificados: [0-9]+" | grep -oE "[0-9]+")
      d=$(echo "$out" | grep -oE "discrepancias: [0-9]+" | grep -oE "[0-9]+")
      [ -z "$n" ] && n=0; [ -z "$d" ] && d=0
      tot=$((tot + n)); bad=$((bad + d)); cells=$((cells+1))
      [ "$d" -gt 0 ] && echo "  DISCREPANCIA en $algo/$inst/$arm: $d de $n"
    done
  done
done
echo ""
echo "celdas verificadas: $cells   (sin datos: $sincells)"
echo "soluciones re-decodificadas: $tot"
echo "discrepancias: $bad"
