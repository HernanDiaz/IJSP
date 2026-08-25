#!/usr/bin/env bash
# Vuelca las trazas COMPLETAS por ejecucion del control A0: una fila por muestra.
# Sirve para el estudio de error de tipo I del test de signos (ttt_null.R), que
# necesita comparar ejecucion contra ejecucion sin promediar ni rejilla comun.
#
# Salida: final/phase2/a0_traces_<algo>.csv  con algo,inst,run,t,v
# v es el minimo acumulado (mejor-hasta-ahora) de esa ejecucion.
cd "$(dirname "$0")/.."
algo=${1:?uso: dump_a0_traces.sh <algo>}
BASEDIR=${OUT_OVERRIDE:-final/phase2}
out="$BASEDIR/a0_traces_${algo}.csv"

INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done

# identico a resample_anytime.sh: el bloque se delimita por cabeceras porque la
# columna Step es la generacion promediada (decimal, y vacia en la ultima fila)
trace_runs() {
  awk -F';' -v off="$2" '
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

echo "algo,inst,run,t,v" > "$out"
for inst in $INSTS; do
  base="$BASEDIR/$algo/$inst/A0"
  tmp=$(mktemp)
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
  sort -k1,1n -k2,2n "$tmp" | awk -v a="$algo" -v i="$inst" \
    '{ printf "%s,%s,%d,%.3f,%.3f\n", a, i, $1+1, $2, $3 }' >> "$out"
  rm -f "$tmp"
done
echo "escrito: $out ($(($(wc -l < "$out")-1)) filas)"
