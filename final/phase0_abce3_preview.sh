#!/usr/bin/env bash
# Preview Fase 0 / ABCE3: avg E[Cmax] final por (instancia, fraccion) + rango medio.
cd "$(dirname "$0")/.."
INSTS="tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05"
ARMS="F00 F10 F25 F50 F75 F100"
avg() { s=$(ls "final/phase0/abce3/$1/$2"/*_Sols.csv 2>/dev/null | head -1); [ -z "$s" ] && { echo ""; return; }
  awk -F';' 'NR>1&&$3~/\(/{gsub(/[()]/,"",$3);split($3,ab,",");e=(ab[1]+ab[2])/2;n++;s+=e} END{if(n)printf "%.1f",s/n}' "$s"; }

printf "%-14s" "instancia"; for a in $ARMS; do printf " %9s" "$a"; done; echo "   (mejor)"
declare -A RSUM
for inst in $INSTS; do
  printf "%-14s" "$inst"
  # recoge valores
  vals=""; for a in $ARMS; do v=$(avg $inst $a); vals="$vals $v"; printf " %9s" "$v"; done
  # ranking por instancia (1=mejor)
  best=$(echo $vals | tr ' ' '\n' | sort -n | head -1)
  bestarm=""
  i=0; for a in $ARMS; do i=$((i+1)); v=$(echo $vals | awk -v k=$i '{print $k}')
    r=$(echo $vals | tr ' ' '\n' | sort -n | grep -n -m1 "^$v$" | cut -d: -f1)
    RSUM[$a]=$(awk -v x="${RSUM[$a]:-0}" -v r=$r 'BEGIN{print x+r}')
    [ "$v" = "$best" ] && bestarm=$a
  done
  echo "   <- $bestarm"
done
echo ""
printf "rango medio: "; for a in $ARMS; do printf " %s=%.2f" "$a" "$(awk -v x="${RSUM[$a]}" 'BEGIN{print x/8}')"; done; echo ""