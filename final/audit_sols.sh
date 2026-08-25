#!/usr/bin/env bash
# AUDITORIA DE LOS FICHEROS DE SOLUCIONES (_Sols.csv).
# Formato: Run;Solution;Objective Value   con Solution = permutacion de IDs de
# TAREA (0-based, id = trabajo*m + operacion) y Objective Value = "(inf, sup)".
#
# Se comprueba, por fichero:
#   1. cabecera y numero de campos identicos
#   2. longitud de la solucion = n*m
#   3. cada TRABAJO aparece exactamente m veces (id/m da el trabajo)
#   4. IDs sin repetir y dentro de [0, n*m)
#   5. intervalo bien formado: inf <= sup, ambos positivos
#   6. numeracion de ejecuciones consecutiva desde 1
#   7. soluciones DUPLICADAS entre ejecuciones del mismo fichero
# Muestra representativa: 4 solvers x 7 brazos x una instancia de cada clase.
cd "$(dirname "$0")/.."
INSTS="ft10 tai15_15_03 tai20_20_05 tai30_15_07 tai30_20_01 tai50_15_04 tai50_20_09"
ARMS="A0 V2H V2 MOR GT GP MIX"
dims() { if [ "$1" = ft10 ]; then f="SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt"
  else f="SelectosYTaillardIntervalos/$1.F.15_01.txt"; fi
  tr -d '\r' < "$f" | awk '/NUMERO DE TRABAJOS/{g=1;next} g==1{n=$1;g=0}
    /NUMERO DE RECURSOS/{g=2;next} g==2{m=$1;g=0} END{print n, m}'; }

nfile=0; nsol=0
declare -A ERR
add() { ERR["$1"]=$(( ${ERR["$1"]:-0} + 1 )); }
dupfiles=0; duptot=0
for algo in ga abce3 feabcls tsn2; do
  for inst in $INSTS; do
    read -r n m <<< "$(dims "$inst")"
    for arm in $ARMS; do
      base="final/phase2/$algo/$inst/$arm"
      if [ -n "$(ls -d "$base"/c* 2>/dev/null)" ]; then fs=$(ls "$base"/c*/*_Sols.csv 2>/dev/null)
      else fs=$(ls "$base"/*_Sols.csv 2>/dev/null); fi
      for f in $fs; do
        [ -f "$f" ] || continue
        nfile=$((nfile+1))
        out=$(awk -F';' -v n="$n" -v m="$m" -v F="$f" '
          NR==1 { if ($0 !~ /^Run;Solution;Objective Value/) print "cabecera"; next }
          { if (NF != 3) { print "campos"; next }
            nsol++
            if ($1+0 != nsol) print "numeracion"
            c=split($2, a, " ")
            if (c != n*m) { print "longitud"; next }
            delete job; delete seen; bad=0
            for (k=1; k<=c; k++) { v=a[k]+0
              if (v<0 || v>=n*m) bad=1
              if (seen[v]++) bad=2
              job[int(v/m)]++ }
            if (bad==1) print "id_rango"; else if (bad==2) print "id_repetido"
            for (j=0; j<n; j++) if (job[j]+0 != m) { print "trabajo_desbalanceado"; break }
            v=$3; gsub(/[() ]/,"",v); split(v, ab, ",")
            if (ab[1]=="" || ab[2]=="") print "objetivo_malformado"
            else if (ab[1]+0 > ab[2]+0) print "intervalo_invertido"
            else if (ab[1]+0 <= 0) print "objetivo_no_positivo"
            sol[$2]++ }
          END { d=0; for (s in sol) if (sol[s]>1) d += sol[s]-1
                print "TOTALSOL", nsol; print "DUPS", d }' "$f")
        while read -r tag val; do
          case "$tag" in
            TOTALSOL) nsol=$((nsol + val)) ;;
            DUPS) [ "${val:-0}" -gt 0 ] && { dupfiles=$((dupfiles+1)); duptot=$((duptot+val)); } ;;
            "") ;;
            *) add "$tag"; [ "${ERR[$tag]}" -le 2 ] && echo "  [$tag] $f" ;;
          esac
        done <<< "$out"
      done
    done
  done
done
echo ""
echo "ficheros auditados: $nfile   soluciones: $nsol"
if [ ${#ERR[@]} -eq 0 ]; then echo "estructura: SIN ERRORES"
else for k in "${!ERR[@]}"; do echo "  $k: ${ERR[$k]} ocurrencias"; done; fi
echo "ficheros con soluciones repetidas entre ejecuciones: $dupfiles (repeticiones: $duptot)"
