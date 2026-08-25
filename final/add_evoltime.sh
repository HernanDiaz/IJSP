#!/usr/bin/env bash
# Anade muestreo de traza POR TIEMPO a los setups base (en vez de por generacion).
# Motivo: con generations=100000+timelimit el GA hace ~3000 gens/run y el framework
# guarda una entrada de traza POR GENERACION de los 30 runs -> memoria descontrolada.
# Con unit=time span=5s: ~47 entradas en un run de 233s (60x menos) y ademas es la
# granularidad correcta para nuestras curvas anytime-vs-tiempo.
cd "$(dirname "$0")/.."
for f in repro/setup_ga.txt repro/setup_abce3.txt repro/setup_feabcls.txt repro/setup_tsn2.txt; do
  grep -q '^evolution.unit' "$f" || printf '\n# Traza de evolucion muestreada por tiempo (memoria acotada)\nevolution.unit = time\nevolution.span = 5\n' >> "$f"
  echo "== $f =="
  grep -E '^evolution\.' "$f"
done
