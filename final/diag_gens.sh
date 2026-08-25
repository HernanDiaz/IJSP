#!/usr/bin/env bash
cd "$(dirname "$0")/.."
show() { # $1 etiqueta, $2 dir
  c=$(ls "$2"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
  if [ -z "$c" ]; then echo "  $1: sin CSV"; return; fi
  g=$(grep -i 'Number of Generations' "$c" | awk -F';' '{print $2}')
  ev=$(grep -i 'Number of Generations' "$c" | awk -F';' '{print $2}')
  # tamano de la seccion Evolution (lineas de traza) = proxy del consumo de memoria
  nl=$(awk '/^Evolution/{f=1} f&&/^[0-9]/{n++} END{print n+0}' "$c")
  sz=$(du -h "$c" 2>/dev/null | awk '{print $1}')
  echo "  $1: generaciones_medias=$g | lineas de traza=$nl | CSV=$sz"
}
echo "=== FASE 2 (mi diseno: generations=100000 + timelimit) ==="
show "ga/tai50_15_01/A0" final/phase2/ga/tai50_15_01/A0
show "ga/tai30_15_01/A0" final/phase2/ga/tai30_15_01/A0
show "abce3/tai50_20_01/A0" final/phase2/abce3/tai50_20_01/A0
echo ""
echo "=== PILOTO con noimprovement=25 (protocolo publicado) ==="
show "ga/TA44/A0" pilot/matrix/ga/TA44/A0
show "abce3/TA44/A0" pilot/matrix/abce3/TA44/A0
echo ""
echo "=== PILOTO con 200 generaciones fijas ==="
show "ga/TA44/A0" pilot/matrix_fixed/ga/TA44/A0
