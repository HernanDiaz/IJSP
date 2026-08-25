#!/usr/bin/env bash
# Reconstruye los 61 pools MIX con ENTRELAZADO FINO y 1024 entradas, el mismo
# tamano que los pools de un solo generador.
#
# Por que: el MIX anterior estaba ordenado en BLOQUES de 84 v2 + 83 GT + 83 gp
# con periodo 250. Los bloques que usa el experimento empiezan en 250r mod L, y
# los que dan la vuelta rompian la composicion (108/83/59 en vez de 84/83/83).
# Entrelazando v2, GT, gp uno a uno, CUALQUIER ventana de 250 queda equilibrada
# y el tamano del pool deja de condicionar la composicion.
#
# No se genera ninguna solucion nueva: se reordenan las de los pools existentes.
# Los MIX antiguos se conservan en pools_test/mix_bloques_original/.
set -e
cd "$(dirname "$0")/.."
P=pools_test/corrected
BK=pools_test/mix_bloques_original
mkdir -p "$BK"
INSTS="ft10"
for c in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${c}_${i}"; done
done
N=1024
per=$(( (N + 2) / 3 ))     # 342 de cada uno; se recorta a N al final
n=0
for inst in $INSTS; do
  mix="$P/int__${inst}_mix_repo_pool.csv"
  [ -f "$mix" ] || { echo "  (sin mix: $inst)"; continue; }
  [ -f "$BK/$(basename "$mix")" ] || cp "$mix" "$BK/"
  for g in v2 gtmwkr gp; do
    head -"$per" "$P/int__${inst}_${g}_repo_pool.csv" > "/tmp/mp_$g"
  done
  paste -d'\n' /tmp/mp_v2 /tmp/mp_gtmwkr /tmp/mp_gp | head -"$N" > "$mix"
  l=$(wc -l < "$mix")
  [ "$l" -eq "$N" ] || { echo "  ERROR $inst: $l lineas"; exit 1; }
  n=$((n+1))
done
echo "pools MIX reconstruidos: $n  (1024 entradas, entrelazado v2/GT/gp)"
echo "originales por bloques conservados en: $BK"
