#!/usr/bin/env bash
# Recupera la instancia CRISP subyacente de cada instancia intervalar.
#
# Las instancias intervalares se construyeron como p -> [p-delta, p+delta] con
# delta ~ U[0, 0.15p], de modo que el intervalo es SIMETRICO respecto de p y el
# midpoint devuelve la duracion crisp original EXACTAMENTE. Verificado: en las
# 61 instancias todos los midpoints son enteros (0 excepciones).
#
# Consecuencia practica: la identificacion del mapeo instancia -> ta<NN> no
# tiene por que apoyarse en cotas debiles. Basta comparar la matriz recuperada
# aqui con la matriz publicada de Taillard, elemento a elemento. Este script
# emite esas matrices en formato estandar (una linea por trabajo: pares
# maquina duracion) junto con un hash por instancia, para que la comprobacion
# sea una sola orden para quien disponga de los ficheros originales.
#
# Salida: final/crisp/<inst>.txt  y  final/crisp/SHA256SUMS
cd "$(dirname "$0")/.."
OUT=final/crisp
mkdir -p "$OUT"
: > "$OUT/SHA256SUMS"

# Solo las instancias del conjunto de evaluacion (las que tienen cota publicada
# en ta_lb.csv). Las instancias clasicas de la replicacion (abz*, la*, ft20)
# vienen en un formato sin cabeceras de seccion y no participan del mapeo.
EVAL=$(cut -d, -f1 final/ta_lb.csv | tail -n +2)

n=0; saltadas=0
for f in SelectosYTaillardIntervalos/*.txt; do
  b=$(basename "$f" .txt)
  # tai30_20_04.F.15_01 -> tai30_20_04 ; F0.15.0.ft10_10 -> ft10
  case "$b" in
    F0.*) inst=$(echo "$b" | awk -F. '{print $NF}' | sed 's/_[0-9]*$//') ;;
    *)    inst=${b%%.*} ;;
  esac
  if ! echo "$EVAL" | grep -qx "$inst"; then saltadas=$((saltadas+1)); continue; fi
  awk '
    /NUMERO DE TRABAJOS/ {s=1; next}
    /NUMERO DE RECURSOS/ {s=2; next}
    /SECUENCIA DE MAQUINAS/ {s=3; next}
    /DURACIONES/ {s=4; next}
    /TIEMPOS MAXIMOS/ {s=0; next}
    /^\// || /^[[:space:]]*$/ {next}
    s==1 {n=$1+0; s=0; next}
    s==2 {m=$1+0; s=0; next}
    s==3 {nj++; for(i=1;i<=NF;i++) M[nj,i]=$i+0; next}
    s==4 {
      nd++; line=$0; gsub(/[(),]/," ",line); c=split(line,a," ")
      k=0
      for(i=1;i<c;i+=2) { k++; mid=(a[i]+a[i+1])/2; D[nd,k]=mid
        if (mid != int(mid)) bad++ }
      next
    }
    END {
      if (bad) { printf "AVISO: %d midpoints no enteros\n", bad > "/dev/stderr" }
      printf "%d %d\n", n, m
      for (j=1; j<=n; j++) {
        s=""
        for (k=1; k<=m; k++) s = s sprintf("%d %d ", M[j,k], D[j,k])
        sub(/ $/, "", s); print s
      }
    }' "$f" > "$OUT/$inst.txt"
  h=$(sha256sum "$OUT/$inst.txt" | cut -c1-16)
  printf "%s  %s\n" "$h" "$inst" >> "$OUT/SHA256SUMS"
  n=$((n+1))
done
echo "recuperadas $n instancias crisp en $OUT/ (saltadas $saltadas ajenas al conjunto)"
echo "comparacion exacta contra Taillard: diff <(normalizar ta<NN>) $OUT/<inst>.txt"
