#!/usr/bin/env bash
# Homogeneiza las curvas anytime: extrae la traza (tiempo, mejor E[Cmax]) de cada
# configuracion —sea de resolucion fina (por generacion) o de 5 s— y la remuestrea
# a una REJILLA TEMPORAL COMUN por algoritmo, con interpolacion escalonada
# (el mejor valor conocido hasta ese instante). Asi las curvas son comparables
# aunque las trazas se hayan grabado con distinta granularidad.
#
# Rejilla por algoritmo (elegida segun su escala temporal, medida en Fase 1):
#   abce3   -> 0.2 s (converge en 0.2-3 s)
#   ga      -> 2 s   (presupuestos 60-321 s)
#   feabcls -> 5 s   (presupuestos 60-900 s)
#   tsn2    -> 5 s   (presupuestos 60-900 s)
# Uso: resample_anytime.sh <algo>  -> final/phase2/anytime_<algo>.csv
cd "$(dirname "$0")/.."
algo=${1:?uso: resample_anytime.sh <algo>}
case "$algo" in
  abce3) STEP=0.2 ;; ga) STEP=2 ;; *) STEP=5 ;;
esac
out="final/phase2/anytime_${algo}.csv"
echo "algo,inst,arm,t,bestcmax" > "$out"

clase() { case "$1" in
  ft10) echo 10x10 ;; tai15_15_*) echo 15x15 ;; tai20_20_*) echo 20x20 ;; tai30_15_*) echo 30x15 ;;
  tai30_20_*) echo 30x20 ;; tai50_15_*) echo 50x15 ;; tai50_20_*) echo 50x20 ;;
esac; }
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done

# traza (runtime, mejor-historico) de un CSV de estadisticas -> stdout
# OJO: se usa la columna 3 ("Best" = best-so-far, monotona), NO la 5
# ("Best Cmax" = mejor de la poblacion actual, que SI puede empeorar cuando el
# algoritmo descarta soluciones por diversidad: verificado 63 subidas de 286
# filas en abce3, con ~3% de diferencia en el valor final).
trace_of() {
  awk -F';' '/^Evolution/{f=1} f && $1 ~ /^[0-9]+$/ { printf "%.3f %.3f\n", $2, $3 }' "$1"
}

for inst in $INSTS; do
  for arm in A0 V2H V2 MOR GT GP MIX; do
    base="final/phase2/$algo/$inst/$arm"
    # recolecta la traza: formato monolitico o por trozos (se toma la de cada trozo
    # y se promedia por instante de rejilla, ya que cada trozo cubre 5 de los 30 runs)
    tmp=$(mktemp)
    mono=$(ls "$base"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    # PRIORIDAD A LOS CHUNKS: las corridas monoliticas abortadas del 26-jul dejaron
    # trazas incompletas al nivel del brazo que ocultaban la corrida buena.
    [ -n "$(ls -d "$base"/c* 2>/dev/null)" ] && mono=""
    if [ -n "$mono" ]; then
      trace_of "$mono" > "$tmp"
    else
      for d in $(ls -d "$base"/c* 2>/dev/null | sort -V); do
        c=$(ls "$d"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
        [ -n "$c" ] && trace_of "$c" >> "$tmp"
      done
    fi
    [ -s "$tmp" ] || { rm -f "$tmp"; continue; }
    # remuestreo a la rejilla: para cada t de la rejilla, media de los valores
    # "ultimo conocido <= t" de cada traza aportada
    sort -n "$tmp" | awk -v st="$STEP" -v a="$algo" -v i="$inst" -v ar="$arm" '
      { t[++n]=$1; v[n]=$2; if($1>tmax) tmax=$1 }
      END{
        if(n==0) exit;
        k=1; last="";
        for (g=0; g<=tmax; g+=st) {
          while (k<=n && t[k]<=g) { last=v[k]; k++ }
          if (last!="") printf "%s,%s,%s,%.1f,%.2f\n", a, i, ar, g, last
        }
      }' >> "$out"
    rm -f "$tmp"
  done
done
echo "$out : $(( $(wc -l < "$out") - 1 )) filas  (rejilla ${STEP}s)"
