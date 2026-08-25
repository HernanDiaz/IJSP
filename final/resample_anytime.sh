#!/usr/bin/env bash
# Curvas anytime: para cada (instancia, brazo) produce E[Cmax] MEJOR-HASTA-AHORA
# frente al tiempo de CPU, sobre una rejilla comun por algoritmo.
#
# DOS CUIDADOS QUE LA VERSION ANTERIOR NO TENIA:
#
# 1) Se usan las trazas POR EJECUCION (columnas 8+6r = Runtime, 9+6r = Best),
#    no el bloque "Average Evolution" de las columnas 1-6. Ese bloque promedia
#    sobre las ejecuciones que han alcanzado cada paso, asi que su composicion
#    cambia a lo largo de la traza.
#
# 2) Se aplica MINIMO ACUMULADO a cada ejecucion. La columna "Best" es el mejor
#    de la POBLACION en esa generacion, no el mejor encontrado hasta el momento:
#    medido sobre TSN2, sube entre 4 y 9 veces de 85 puntos por ejecucion. Una
#    curva anytime debe ser monotona, de modo que se acumula el minimo.
#
# Cada ejecucion se extiende con su ultimo valor hasta el final de la rejilla,
# para que la media no cambie de composicion (sesgo de supervivencia).
#
# NOTA: el ultimo punto de la traza puede ser peor que la solucion finalmente
# reportada, porque el muestreo es cada evolution.span y la ultima mejora puede
# caer entre la ultima muestra y la terminacion. Por eso las curvas al 100% del
# presupuesto no coinciden exactamente con las tablas de calidad final.
cd "$(dirname "$0")/.."
algo=${1:?uso: resample_anytime.sh <algo>}
BASEDIR=${OUT_OVERRIDE:-final/phase2}
out="$BASEDIR/anytime_${algo}.csv"
case "$algo" in
  ga)      STEP=2   ;;
  abce3)   STEP=0.2 ;;
  feabcls) STEP=5   ;;
  tsn2)    STEP=5   ;;
  *)       STEP=5   ;;
esac
if [ -n "$INSTS_OVERRIDE" ]; then INSTS="$INSTS_OVERRIDE"; else
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
fi

# Extrae "run_global tiempo mejor_acumulado" de un fichero de trazas.
# $2 = desplazamiento de numeracion de ejecucion (para no mezclar trozos).
trace_runs() {
  awk -F';' -v off="$2" '
    /^Evolution/{f=1}
    f && $1 ~ /^[0-9]+$/ {
      for (r=0; r<40; r++) {
        ct = 7 + r*6 + 1        # Runtime de la ejecucion r
        cv = 7 + r*6 + 2        # Best de la ejecucion r
        if (cv > NF || $cv == "") continue
        v = $cv + 0
        if (v <= 0) continue
        key = off + r
        if (!(key in mn) || v < mn[key]) mn[key] = v    # minimo acumulado
        printf "%d %.3f %.3f\n", key, $ct, mn[key]
      }
    }' "$1"
}

echo "algo,inst,arm,t,bestcmax" > "$out"
for inst in $INSTS; do
  for arm in A0 V2H V2 MOR GT GP MIX; do
    base="$BASEDIR/$algo/$inst/$arm"
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
    # Rejilla comun: para cada t, media sobre ejecuciones del ultimo valor <= t,
    # extendiendo con el ultimo conocido las que ya terminaron.
    sort -k1,1n -k2,2n "$tmp" | awk -v st="$STEP" -v a="$algo" -v i="$inst" -v ar="$arm" '
      { run[$1]=1; n=++cnt[$1]; T[$1,n]=$2; V[$1,n]=$3; if ($2>tmax) tmax=$2 }
      END {
        nrun=0; for (r in run) nrun++
        for (t=st; t<=tmax+st/2; t+=st) {
          s=0; k=0
          for (r in run) {
            m=cnt[r]; cur=""
            for (j=1; j<=m; j++) { if (T[r,j] <= t) cur=V[r,j]; else break }
            if (cur=="") continue            # esa ejecucion aun no ha reportado
            s+=cur; k++
          }
          # COMPOSICION ESTABLE: solo se emite el punto si TODAS las ejecuciones
          # han reportado ya. Si no, la media cambia de composicion al ir
          # incorporandose ejecuciones y la curva puede subir aunque cada
          # ejecucion sea monotona.
          if (k == nrun) printf "%s,%s,%s,%.2f,%.2f\n", a, i, ar, t, s/k
        }
      }' >> "$out"
    rm -f "$tmp"
  done
done
echo "$out : $(( $(wc -l < "$out") - 1 )) filas  (rejilla ${STEP}s, minimo acumulado por ejecucion)"
