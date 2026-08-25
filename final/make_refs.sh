#!/usr/bin/env bash
# Extrae la referencia best_known por instancia DESDE LOS DATOS PUBLICADOS del
# grupo (Papers/COR_Tabu/data/phaseA/all_results_full.csv, columna best_known;
# verificado: 82 instancias, valor constante por instancia).
# NO se inventa ningun valor. Salida: final/refs.csv  (inst,best_known)
cd "$(dirname "$0")/.."
SRC="/mnt/c/Users/diazhernan/CLionProjects/IJSP/Papers/COR_Tabu/data/phaseA/all_results_full.csv"
[ -f "$SRC" ] || { echo "FALTA la fuente: $SRC"; exit 1; }
out=final/refs.csv
echo "inst,best_known" > "$out"
awk -F',' 'NR>1{
  gsub(/"/,"");
  inst=$1; bk=$13;
  # normaliza el nombre al que usamos: tai50_20_10.F.15_01 -> tai50_20_10
  #                                    F0.15.0.ft10_10     -> ft10
  sub(/\.F\.15_01$/,"",inst);
  if (inst ~ /^F0\.15\.0\.ft10/) inst="ft10";
  if (!(inst in seen)) { seen[inst]=1; print inst "," bk }
}' "$SRC" >> "$out"
echo "$out: $(( $(wc -l < "$out") - 1 )) instancias"
echo "--- cobertura de nuestras 61 ---"
miss=0
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do
    grep -q "^${cls}_${i}," "$out" || { echo "  FALTA ${cls}_${i}"; miss=$((miss+1)); }
  done
done
grep -q '^ft10,' "$out" || { echo "  FALTA ft10"; miss=$((miss+1)); }
echo "  faltan: $miss de 61"
