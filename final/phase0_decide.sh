#!/usr/bin/env bash
# Decision p* de la Fase 0 (regla PRE-REGISTRADA):
#   criterio 1: rango de avg E[Cmax] final en abce3 (menor mejor)
#   criterio 2: rango de TTT a la calidad final de A0 (tol +0.5%) en tsn2 (menor mejor;
#               no alcanza -> peor rango)
#   p* = mejor rango medio combinado sobre las 8 instancias; empate -> p menor.
cd "$(dirname "$0")/.."
INSTS="tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05"
ARMS="F00 F10 F25 F50 F75 F100"

avgof() { s=$(ls "final/phase0/$1/$2/$3"/*_Sols.csv 2>/dev/null | head -1); [ -z "$s" ] && { echo ""; return; }
  awk -F';' 'NR>1&&$3~/\(/{gsub(/[()]/,"",$3);split($3,ab,",");e=(ab[1]+ab[2])/2;n++;s+=e} END{if(n)printf "%.2f",s/n}' "$s"; }
tracecsv() { ls "final/phase0/$1/$2/$3"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1; }
# TTT: primer runtime con bestCmax(media) <= objetivo
ttt() { awk -F';' -v t="$2" 'BEGIN{f=0;r=-1} /^Evolution/{f=1} f&&$1~/^[0-9]+$/{ if(r<0 && $5+0<=t) r=$2 } END{print r}' "$1"; }

echo "=========== CRITERIO 1: abce3 calidad final (avg E[Cmax]) ==========="
declare -A R1SUM
for inst in $INSTS; do
  vals=""; for a in $ARMS; do vals="$vals $(avgof abce3 $inst $a)"; done
  i=0; for a in $ARMS; do i=$((i+1)); v=$(echo $vals | awk -v k=$i '{print $k}')
    r=$(echo $vals | tr ' ' '\n' | awk -v v="$v" '$1<v{c++} END{print c+1}')
    R1SUM[$a]=$(awk -v x="${R1SUM[$a]:-0}" -v r=$r 'BEGIN{print x+r}')
  done
done
for a in $ARMS; do printf "  %-5s rango_medio=%.2f\n" "$a" "$(awk -v x="${R1SUM[$a]}" 'BEGIN{print x/8}')"; done

echo ""
echo "=========== CRITERIO 2: tsn2 TTT a calidad final de A0 (tol +0.5%) ==========="
declare -A R2SUM
for inst in $INSTS; do
  c0=$(tracecsv tsn2 $inst F00)
  a0f=$(awk -F';' '/^Evolution/{f=1} f&&$1~/^[0-9]+$/{v=$5} END{print v}' "$c0")
  tgt=$(awk -v x=$a0f 'BEGIN{printf "%.3f", x*1.005}')
  vals=""
  for a in $ARMS; do
    c=$(tracecsv tsn2 $inst $a); t=$(ttt "$c" "$tgt")
    [ "$t" = "-1" ] && t=999999
    vals="$vals $t"
  done
  printf "  %-14s A0final=%.0f  TTT(s):" "$inst" "$a0f"
  i=0; for a in $ARMS; do i=$((i+1)); v=$(echo $vals | awk -v k=$i '{print $k}')
    printf " %s=%s" "$a" "$(awk -v v=$v 'BEGIN{print (v>=999999?"NO":sprintf("%.0f",v))}')"
    r=$(echo $vals | tr ' ' '\n' | awk -v v="$v" '$1<v{c++} END{print c+1}')
    R2SUM[$a]=$(awk -v x="${R2SUM[$a]:-0}" -v r=$r 'BEGIN{print x+r}')
  done; echo ""
done
for a in $ARMS; do printf "  %-5s rango_medio=%.2f\n" "$a" "$(awk -v x="${R2SUM[$a]}" 'BEGIN{print x/8}')"; done

echo ""
echo "=========== COMBINADO (media de los dos rangos medios) ==========="
best=""; bestv=999
for a in $ARMS; do
  c=$(awk -v x="${R1SUM[$a]}" -v y="${R2SUM[$a]}" 'BEGIN{printf "%.3f",(x/8+y/8)/2}')
  printf "  %-5s combinado=%.2f\n" "$a" "$c"
  cmp=$(awk -v c=$c -v b=$bestv 'BEGIN{print (c<b)?1:0}')
  [ "$cmp" = "1" ] && { best=$a; bestv=$c; }
done
echo ""
echo ">>> p* = $best (regla: mejor combinado; empate resuelto a p menor por orden de recorrido)"