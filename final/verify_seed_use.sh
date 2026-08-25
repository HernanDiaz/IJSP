#!/usr/bin/env bash
# ¿SE ESTA LEYENDO EL BLOQUE CORRECTO DEL POOL CORRECTO?
# Para cada ejecucion de una celda ya terminada:
#   - se lee del setup.txt el pool, el numero de semillas y el offset
#   - se calcula el bloque que le corresponde: [(offset+r)*k mod L, +k)
#   - se compara el MEJOR E[Cmax] de ese bloque (segun el makespan que trae el
#     pool) con el "Best Cmax" del paso 0 de esa ejecucion en la traza
# Deben coincidir. Si la traza es algo MEJOR, es el re-decodificado con SGS de
# inserccion (ya documentado). Si la traza fuese PEOR, seria imposible: querria
# decir que se sembro otro bloque, otro pool, o menos semillas de las previstas.
cd "$(dirname "$0")/.."
ok=0; mejor=0; malo=0
for spec in "$@"; do
  IFS=/ read -r algo inst arm <<< "$spec"
  d="final/phase2/$algo/$inst/$arm"
  [ -d "$d" ] || { echo "  (no existe $spec)"; continue; }
  echo "### $spec"
  for j in 1 2 3 4 5 6; do
    s="$d/c$j/setup.txt"; [ -f "$s" ] || continue
    f=$(ls "$d/c$j"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
    [ -z "$f" ] && continue
    pool=$(grep -E '^creation.seed.pool' "$s" | sed 's/.*= *//')
    k=$(grep -E '^creation.seed.count' "$s" | tr -dc '0-9')
    off=$(grep -E '^creation.seed.offset' "$s" | tr -dc '0-9')
    [ -z "$pool" ] && continue
    L=$(wc -l < "$pool")
    [ "$j" -eq 1 ] && echo "    pool=$(basename "$pool")  L=$L  k=$k"
    for r in 0 1 2 3 4; do
      g=$(( off + r )); st=$(( (g*k) % L ))
      # mejor E[Cmax] del bloque, segun el makespan almacenado en el pool
      best=$(awk -v s="$st" -v k="$k" -v L="$L" 'NR>=1{ i=NR-1
          d=(i-s+L)%L; if (d<k) { v=$0; sub(/.*;\[/,"",v); sub(/\].*/,"",v); gsub(/ /,"",v)
            split(v,ab,","); m=(ab[1]+ab[2])/2; if (b=="" || m<b) b=m } }
        END{ printf "%.1f", b }' "$pool")
      obs=$(awk -F';' -v r="$r" '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {c=7+r*6+4; print $c+0; exit}' "$f")
      cmp=$(awk -v a="$best" -v b="$obs" 'BEGIN{ if (b==a) print "igual"; else if (b<a) print "mejor"; else print "PEOR" }')
      case "$cmp" in
        igual) ok=$((ok+1)) ;;
        mejor) mejor=$((mejor+1)) ;;
        PEOR)  malo=$((malo+1)); echo "    run_global=$g bloque=$st  pool=$best  traza=$obs  <-- IMPOSIBLE" ;;
      esac
    done
  done
done
echo ""
echo "ejecuciones comprobadas: $((ok+mejor+malo))"
echo "  coincide exacto con el mejor del bloque: $ok"
echo "  traza algo mejor (re-decodificado activo): $mejor"
echo "  traza PEOR que el bloque (indicaria siembra incorrecta): $malo"
