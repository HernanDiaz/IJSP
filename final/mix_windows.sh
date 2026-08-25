#!/usr/bin/env bash
# ¿Se mantiene la composicion 84/83/83 de MIX en CUALQUIER ventana de 250, o solo
# en las alineadas a multiplo de 250? Importa porque los bloques que se usan
# empiezan en 250r mod L: si L no es multiplo de 250, las ventanas que dan la
# vuelta quedan descompensadas y el brazo MIX dejaria de ser mixto.
cd "$(dirname "$0")/.."
inst=${1:-tai30_20_01}; P=pools_test/corrected
for g in v2 gtmwkr gp; do
  cut -d';' -f1 "$P/int__${inst}_${g}_repo_pool.csv" | sed 's/[[:space:]]*$//' | sort -u > "/tmp/w_$g.txt"
done
mixf="$P/int__${inst}_mix_repo_pool.csv"
comp() {  # $1=inicio (0-based) $2=L efectivo
  local s=$1 L=$2
  { for ((i=0;i<250;i++)); do echo $(( (s+i) % L + 1 )); done; } | sort -n | \
    while read -r ln; do sed -n "${ln}p" "$mixf"; done | cut -d';' -f1 | sed 's/[[:space:]]*$//' | sort -u > /tmp/w_blk.txt
  local out=""
  for g in v2 gtmwkr gp; do out="$out $(comm -12 /tmp/w_blk.txt "/tmp/w_$g.txt" | wc -l)"; done
  echo "$out"
}
echo "=== composicion (v2 / gtmwkr / gp) de ventanas de 250 ==="
echo ""
echo "--- pool COMPLETO (L=7500, multiplo de 250): todos los inicios son 250r ---"
for s in 0 250 1750 7250; do printf "  inicio %5d ->%s\n" "$s" "$(comp $s 7500)"; done
echo ""
echo "--- si truncasemos a L=1024 (NO multiplo de 250) ---"
for s in 0 250 750 1000; do
  printf "  inicio %5d ->%s%s\n" "$s" "$(comp $s 1024)" \
    "$( [ "$s" -eq 1000 ] && echo '   <- esta da la vuelta' )"
done
