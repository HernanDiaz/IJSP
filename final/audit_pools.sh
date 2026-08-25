#!/usr/bin/env bash
# AUDITORIA DE POOLS: ¿son permutaciones con repeticion validas y de LA instancia
# que dice el nombre? Un pool con la codificacion mal formada, o con las
# dimensiones de otra instancia, sembraria basura sin que nada lo avisara.
# Se comprueba, para cada pool: numero de genes = n*m, cada trabajo exactamente
# m veces, identificadores en 1..n, y ausencia de lineas mal formadas.
cd "$(dirname "$0")/.."
P=pools_test/corrected
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
dims() {  # n y m de la instancia
  if [ "$1" = ft10 ]; then f="SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt"
  else f="SelectosYTaillardIntervalos/$1.F.15_01.txt"; fi
  tr -d '\r' < "$f" | awk '/NUMERO DE TRABAJOS/{g=1;next} g==1{n=$1;g=0}
    /NUMERO DE RECURSOS/{g=2;next} g==2{m=$1;g=0} END{print n, m}'
}
falt=0; malo=0; ok=0
for inst in $INSTS; do
  read -r n m <<< "$(dims "$inst")"
  for g in v2 graspmor gtmwkr gp mix; do
    f="$P/int__${inst}_${g}_repo_pool.csv"
    if [ ! -f "$f" ]; then echo "  FALTA $(basename $f)"; falt=$((falt+1)); continue; fi
    bad=$(awk -F';' -v n="$n" -v m="$m" '
      { split($1, a, " "); c=0; delete cnt
        for (k in a) { v=a[k]+0; if (a[k] ~ /^[0-9]+$/) { c++; cnt[v]++ } }
        if (c != n*m) { print NR": genes="c" (esperado "n*m")"; next }
        for (j=1; j<=n; j++) if (cnt[j] != m) { print NR": trabajo "j" aparece "cnt[j]+0" veces (esperado "m")"; next }
        # OJO: los indices de array en awk son CADENAS; sin +0 esto compara texto
        # ("6" > "50" es cierto) y da falsos positivos en todas las instancias.
        for (v in cnt) if (v+0 < 1 || v+0 > n) { print NR": id fuera de rango "v; next }
      }' "$f" | head -3)
    if [ -n "$bad" ]; then
      echo "  MAL FORMADO $(basename $f):"; echo "$bad" | sed 's/^/      /'; malo=$((malo+1))
    else ok=$((ok+1)); fi
  done
done
echo ""
echo "pools correctos: $ok | mal formados: $malo | ausentes: $falt"
