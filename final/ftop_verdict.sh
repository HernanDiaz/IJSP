#!/usr/bin/env bash
# Veredicto FTOP: seleccion elitista (mejores 250 del pool) vs i.i.d.@100% (F100).
# Compara avg E[Cmax] final por (algo, instancia), pareado por semillas.
cd "$(dirname "$0")/.."
INSTS="tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05"
avg() { s=$(ls "final/phase0/$1/$2/$3"/*_Sols.csv 2>/dev/null | head -1); [ -z "$s" ] && { echo ""; return; }
  awk -F';' 'NR>1&&$3~/\(/{gsub(/[()]/,"",$3);split($3,ab,",");e=(ab[1]+ab[2])/2;n++;s+=e} END{if(n)printf "%.1f",s/n}' "$s"; }
for algo in abce3 tsn2; do
  echo "########## $algo ##########"
  wF=0; wT=0
  printf "  %-14s %10s %10s %10s\n" instancia F100 FTOP dFTOP
  for inst in $INSTS; do
    f=$(avg $algo $inst F100); t=$(avg $algo $inst FTOP)
    d=$(awk -v a=$t -v b=$f 'BEGIN{printf "%+.1f",a-b}')
    printf "  %-14s %10s %10s %10s\n" "$inst" "$f" "$t" "$d"
    awk -v a=$t -v b=$f 'BEGIN{exit !(a<b)}' && wT=$((wT+1)) || wF=$((wF+1))
  done
  echo "  gana FTOP en $wT/8   |   gana F100 en $wF/8"
done