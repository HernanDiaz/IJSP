#!/usr/bin/env bash
# Extrae, POR EJECUCION, los dos numeros que necesita la regla de meseta sobre
# el control A0:
#
#   T_run  = tiempo de CPU final de esa ejecucion
#   v90    = mejor-hasta-ahora (minimo acumulado) en la ultima muestra con
#            t <= 0.9 * T_run
#   v_fin  = mejor-hasta-ahora en la ULTIMA muestra de la traza. Permite separar
#            la mejora observada en el ultimo 10% (v90 - v_fin) del desfase
#            entre la instantanea de poblacion y la solucion devuelta
#            (v_fin - devuelta), que son cosas distintas.
#
# Por que por ejecucion y no sobre la curva promediada: promediar 30 trazas
# antes de aplicar la regla puede ocultar que un subconjunto de ejecuciones
# sigue mejorando al final. La regla se evalua ejecucion a ejecucion y se
# agrega despues con un criterio declarado.
#
# El valor FINAL de cada ejecucion NO se toma de la traza: se toma del
# resultado devuelto (results_<algo>.csv), porque la ultima mejora puede caer
# entre la ultima muestra y la terminacion, y porque el solver retiene una
# estructura elite que la instantanea de poblacion no contiene. Verificado
# sobre abce3/tai30_20_01: en 4 de 30 ejecuciones el resultado devuelto es
# mejor que la ultima muestra, y en ninguna es peor.
#
# Salida: final/phase2/conv_runs_<algo>.csv con algo,inst,run,t_fin,v90,v_fin
# La numeracion de ejecucion es 1..30, para casar con results_<algo>.csv.
cd "$(dirname "$0")/.."
algo=${1:?uso: convergence_runs.sh <algo>}
BASEDIR=${OUT_OVERRIDE:-final/phase2}
out="$BASEDIR/conv_runs_${algo}.csv"

if [ -n "$INSTS_OVERRIDE" ]; then INSTS="$INSTS_OVERRIDE"; else
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
fi

# "run_global tiempo mejor_acumulado", identico a resample_anytime.sh
trace_runs() {
  awk -F';' -v off="$2" '
    # ver nota en resample_anytime.sh: Step es decimal y la ultima fila la tiene
    # vacia, asi que el bloque se delimita por cabeceras, no por el formato de $1
    /^Evolution/ { e=1; next }
    e && /^Step;/ { h=1; next }
    h && NF > 6 {
      for (r=0; r<40; r++) {
        ct = 7 + r*6 + 1
        cv = 7 + r*6 + 2
        if (cv > NF || $cv == "" || $ct == "") continue
        v = $cv + 0
        if (v <= 0) continue
        key = off + r
        if (!(key in mn) || v < mn[key]) mn[key] = v
        printf "%d %.3f %.3f\n", key, $ct, mn[key]
      }
    }' "$1"
}

echo "algo,inst,run,t_fin,v90,v_fin" > "$out"
for inst in $INSTS; do
  base="$BASEDIR/$algo/$inst/A0"
  tmp=$(mktemp)
  # los trozos tienen PRIORIDAD sobre un monolitico que haya quedado del
  # mismo cell: mezclarlos duplicaria ejecuciones
  if [ -n "$(ls -d "$base"/c* 2>/dev/null)" ]; then
    off=0
    for d in $(ls -d "$base"/c* 2>/dev/null | sort -V); do
      c=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
      [ -n "$c" ] && { trace_runs "$c" "$off" >> "$tmp"; off=$((off+5)); }
    done
  else
    c=$(ls "$base"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    [ -n "$c" ] && trace_runs "$c" 0 >> "$tmp"
  fi
  [ -s "$tmp" ] || { rm -f "$tmp"; continue; }
  sort -k1,1n -k2,2n "$tmp" | awk -v a="$algo" -v i="$inst" '
    { n=++cnt[$1]; T[$1,n]=$2; V[$1,n]=$3; run[$1]=1 }
    END {
      for (r in run) {
        m = cnt[r]; tf = T[r,m]
        lim = 0.9 * tf; v90 = ""
        for (j=1; j<=m; j++) { if (T[r,j] <= lim) v90 = V[r,j]; else break }
        if (v90 == "") v90 = V[r,1]
        printf "%s,%s,%d,%.3f,%.3f,%.3f\n", a, i, r+1, tf, v90, V[r,m]
      }
    }' >> "$out"
  rm -f "$tmp"
done
echo "escrito: $out ($(($(wc -l < "$out")-1)) filas)"
