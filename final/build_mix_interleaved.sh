#!/usr/bin/env bash
# PROTOTIPO: pool MIX de 1024 entradas ENTRELAZADO FINO (v2, GT, gp, v2, GT, gp...)
# en lugar de por bloques de 84/83/83.
#
# El problema del MIX actual no es el tamano sino el ORDEN: al estar en bloques
# con periodo 250, las ventanas que dan la vuelta quedan descompensadas
# (108/83/59 en vez de 84/83/83). Con entrelazado fino, CUALQUIER ventana de 250
# contiene ~83 de cada generador, sea cual sea el desplazamiento, y el tamano del
# pool deja de importar para la composicion.
#
# No genera soluciones nuevas: reutiliza las que ya hay en los pools simples.
cd "$(dirname "$0")/.."
inst=${1:-tai30_20_01}; N=${2:-1024}
P=pools_test/corrected
out="/tmp/mix_interleaved_${inst}_${N}.csv"
: > "$out"
per=$(( (N + 2) / 3 ))          # cuantas de cada generador
for ((i=1; i<=per; i++)); do
  for g in v2 gtmwkr gp; do
    [ "$(wc -l < "$out")" -ge "$N" ] && break 2
    sed -n "${i}p" "$P/int__${inst}_${g}_repo_pool.csv" >> "$out"
  done
done
echo "generado: $out  ($(wc -l < "$out") entradas)"

# --- validacion: composicion de las 30 ventanas que realmente se usan ---
for g in v2 gtmwkr gp; do
  cut -d';' -f1 "$P/int__${inst}_${g}_repo_pool.csv" | sed 's/[[:space:]]*$//' | sort -u > "/tmp/v_$g.txt"
done
L=$(wc -l < "$out")
echo ""
echo "=== composicion (v2/GT/gp) de los 30 bloques que usaria el experimento ==="
peor=0
for r in $(seq 0 29); do
  s=$(( (r*250) % L ))
  { for ((i=0;i<250;i++)); do echo $(( (s+i) % L + 1 )); done; } | sort -n | \
    while read -r ln; do sed -n "${ln}p" "$out"; done | cut -d';' -f1 | sed 's/[[:space:]]*$//' | sort -u > /tmp/vb.txt
  c=""
  for g in v2 gtmwkr gp; do c="$c $(comm -12 /tmp/vb.txt "/tmp/v_$g.txt" | wc -l)"; done
  set -- $c
  d=$(( $1 > $2 ? $1 - $2 : $2 - $1 )); e=$(( $1 > $3 ? $1 - $3 : $3 - $1 ))
  [ "$e" -gt "$d" ] && d=$e
  [ "$d" -gt "$peor" ] && peor=$d
  [ "$r" -lt 3 ] || [ "$r" -gt 27 ] && printf "  bloque %2d (inicio %4d) ->%s\n" "$r" "$s" "$c"
done
echo ""
echo "  maximo desequilibrio entre generadores en cualquier bloque: $peor"
echo "  (con el MIX actual truncado a 1024 el desequilibrio llegaba a 49: 108 vs 59)"
