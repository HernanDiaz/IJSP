#!/usr/bin/env bash
# CENSO DEL POOL MIX: ¿cuantas semillas de cada tipo hay, cuantas son DISTINTAS,
# y de donde salen? Importa saber si las 7500 entradas son 7500 soluciones
# distintas o una remuestra de los pools de 1024 de cada generador.
cd "$(dirname "$0")/.."
inst=${1:-tai30_20_01}; P=pools_test/corrected
key() { cut -d';' -f1 "$1" | sed 's/[[:space:]]*$//'; }
mixf="$P/int__${inst}_mix_repo_pool.csv"
key "$mixf" > /tmp/mx.txt
for g in v2 gtmwkr gp graspmor; do key "$P/int__${inst}_${g}_repo_pool.csv" | sort -u > "/tmp/g_$g.txt"; done
sort -u /tmp/mx.txt > /tmp/mx_u.txt

echo "=== instancia $inst ==="
printf "  entradas del pool MIX:            %6d\n" "$(wc -l < /tmp/mx.txt)"
printf "  soluciones DISTINTAS en el MIX:   %6d\n" "$(wc -l < /tmp/mx_u.txt)"
echo ""
printf "%-10s %12s %12s %14s %12s\n" generador "en su pool" "distintas" "en el MIX" "% aportado"
tot=0
for g in v2 gtmwkr gp graspmor; do
  disp=$(wc -l < "/tmp/g_$g.txt")
  # distintas del generador presentes en el MIX
  pres=$(comm -12 /tmp/mx_u.txt "/tmp/g_$g.txt" | wc -l)
  # apariciones totales (con repeticion) en el MIX
  ap=$(grep -Fxc -f "/tmp/g_$g.txt" /tmp/mx.txt 2>/dev/null || \
       awk 'NR==FNR{s[$0];next} ($0 in s){n++} END{print n+0}' "/tmp/g_$g.txt" /tmp/mx.txt)
  tot=$((tot+ap))
  printf "%-10s %12d %12d %14d %11.1f%%\n" "$g" "$disp" "$pres" "$ap" "$(awk -v a=$ap 'BEGIN{printf "%.1f", 100*a/7500}')"
done
printf "%-10s %12s %12s %14d %11.1f%%\n" "TOTAL" "" "" "$tot" "$(awk -v a=$tot 'BEGIN{printf "%.1f", 100*a/7500}')"
echo ""
echo "  (si 'en el MIX' supera a 'distintas', el MIX repite soluciones del mismo"
echo "   generador; si TOTAL < 7500, hay entradas que no vienen de ningun pool)"
