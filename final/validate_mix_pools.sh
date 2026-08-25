#!/usr/bin/env bash
# Valida los 61 pools MIX reconstruidos:
#   1. 1024 entradas y codificacion valida (cada trabajo m veces, ids en rango)
#   2. los 30 bloques que usa el experimento (250r mod 1024) tienen composicion
#      equilibrada de v2/GT/gp, incluidos los que dan la vuelta
#   3. no quedan entradas de graspmor (el MIX no incluye MOR, por diseno)
cd "$(dirname "$0")/.."
P=pools_test/corrected
INSTS="ft10"
for c in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${c}_${i}"; done
done
dims() { if [ "$1" = ft10 ]; then f="SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt"
  else f="SelectosYTaillardIntervalos/$1.F.15_01.txt"; fi
  tr -d '\r' < "$f" | awk '/NUMERO DE TRABAJOS/{g=1;next} g==1{n=$1;g=0}
    /NUMERO DE RECURSOS/{g=2;next} g==2{m=$1;g=0} END{print n, m}'; }
badenc=0; badcomp=0; badmor=0; peor=0; okn=0
for inst in $INSTS; do
  mix="$P/int__${inst}_mix_repo_pool.csv"; [ -f "$mix" ] || continue
  read -r n m <<< "$(dims "$inst")"
  # 1) codificacion
  e=$(awk -F';' -v n="$n" -v m="$m" '{ c=split($1,a," "); if (c!=n*m){print "len";exit}
        delete cnt; for(k=1;k<=c;k++){v=a[k]+0; if(v<1||v>n){print "rango";exit} cnt[v]++}
        for(j=1;j<=n;j++) if(cnt[j]+0!=m){print "balance";exit} }' "$mix")
  [ -n "$e" ] && { echo "  [$inst] codificacion: $e"; badenc=$((badenc+1)); continue; }
  # 2) composicion de los 30 bloques  (posicion i -> generador i%3: 0=v2,1=GT,2=gp)
  d=$(awk -v L=1024 'BEGIN{ peor=0
      for (r=0;r<30;r++){ s=(r*250)%L; delete c
        for (i=0;i<250;i++) c[((s+i)%L)%3]++
        mx=0; mn=999; for(t=0;t<3;t++){ v=c[t]+0; if(v>mx)mx=v; if(v<mn)mn=v }
        if (mx-mn>peor) peor=mx-mn }
      print peor }')
  [ "$d" -gt "$peor" ] && peor=$d
  [ "$d" -gt 2 ] && { echo "  [$inst] desequilibrio de composicion: $d"; badcomp=$((badcomp+1)); continue; }
  # 3) sin MOR
  cut -d';' -f1 "$mix" | sed 's/[[:space:]]*$//' | sort -u > /tmp/vm.txt
  cut -d';' -f1 "$P/int__${inst}_graspmor_repo_pool.csv" | sed 's/[[:space:]]*$//' | sort -u > /tmp/vg.txt
  k=$(comm -12 /tmp/vm.txt /tmp/vg.txt | wc -l)
  [ "$k" -gt 0 ] && { echo "  [$inst] contiene $k soluciones de graspmor"; badmor=$((badmor+1)); continue; }
  okn=$((okn+1))
done
echo ""
echo "pools validados: $okn de 61"
echo "  codificacion invalida: $badenc | composicion desequilibrada: $badcomp | con MOR: $badmor"
echo "  desequilibrio maximo entre generadores en cualquiera de los 30 bloques: $peor"
