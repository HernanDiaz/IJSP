#!/usr/bin/env bash
# Valida el arreglo: 1 worker GA en tai50_15_01 (la instancia que engordaba),
# runs=5, traza por TIEMPO. Se ejecuta en PRIMER PLANO (los procesos en background
# lanzados desde `wsl -e` mueren al terminar el comando).
# Comparar memoria con el diseno anterior: 437 MB al inicio -> 988 MB tras ~20 runs.
cd "$(dirname "$0")/.."
d=final/validate
rm -rf "$d"; mkdir -p "$d"
sed -e 's/^runs = .*/runs = 5/' -e 's/^seed = .*/seed = 1/' repro/setup_ga.txt > "$d/setup.txt"
sed -i 's/^noimprovement = .*/generations = 100000/' "$d/setup.txt"
if grep -q '^timelimit' "$d/setup.txt"; then
  sed -i 's/^timelimit = .*/timelimit = 233/' "$d/setup.txt"
else
  echo "timelimit = 233" >> "$d/setup.txt"
fi
echo "--- setup efectivo ---"
grep -E '^(runs|seed|generations|timelimit|evolution)' "$d/setup.txt"
echo "--- ejecutando (5 runs x 233s ~ 20 min) ---"
./FuzzyFW "$d/setup.txt" SelectosYTaillardIntervalos/tai50_15_01.F.15_01.txt "$d" > "$d/log.txt" 2>&1
echo "VALIDACION_TERMINADA rc=$?"
bash final/check_validate.sh
