#!/usr/bin/env bash
# AUDITORIA DE PROCEDENCIA de los datos de la Fase 2.
#
# Problema detectado: las corridas MONOLITICAS abortadas del 26-jul dejaron
# <arm>/*_Sols.csv, y extract_phase2c.sh los PREFIERE sobre los chunks c*/.
# Hay que saber, por algoritmo: cuantos brazos tienen chunks completos, cuantos
# quedaron servidos por el monolitico, y si las CONFIGURACIONES difieren.
cd "$(dirname "$0")/.."
CH=( [ga]=5 [abce3]=5 [feabcls]=3 [tsn2]=3 )
declare -A CHUNK=( [ga]=6 [abce3]=6 [feabcls]=10 [tsn2]=10 )

for a in ga abce3 feabcls tsn2; do
  nmono=0; nchunk_ok=0; nchunk_bad=0; nboth=0; tot=0
  for d in final/phase2/$a/*/*/; do
    tot=$((tot+1))
    m=$(ls "$d"*_Sols.csv 2>/dev/null | head -1)
    runs_ch=0
    for c in "$d"c*/; do
      [ -d "$c" ] || continue
      s=$(ls "$c"*_Sols.csv 2>/dev/null | head -1)
      [ -n "$s" ] && runs_ch=$(( runs_ch + $(wc -l < "$s") - 1 ))
    done
    [ -n "$m" ] && nmono=$((nmono+1))
    [ -n "$m" ] && [ "$runs_ch" -gt 0 ] && nboth=$((nboth+1))
    if [ "$runs_ch" -eq 30 ]; then nchunk_ok=$((nchunk_ok+1))
    elif [ "$runs_ch" -gt 0 ]; then nchunk_bad=$((nchunk_bad+1)); fi
  done
  printf "%-8s brazos=%3d | con monolitico=%3d | chunks completos(30)=%3d | chunks incompletos=%3d | AMBOS=%3d\n" \
    "$a" "$tot" "$nmono" "$nchunk_ok" "$nchunk_bad" "$nboth"
done

echo ""
echo "=== diferencias de CONFIGURACION entre la corrida monolitica y la chunked ==="
for a in ga abce3; do
  d=$(ls -d final/phase2/$a/*/A0/ 2>/dev/null | head -1)
  sm="${d}setup.txt"; sc="${d}c1/setup.txt"
  echo "--- $a ($d) ---"
  if [ -f "$sm" ] && [ -f "$sc" ]; then
    diff <(grep -vE '^(runs|seed) ' "$sm" | sort) <(grep -vE '^(runs|seed) ' "$sc" | sort) \
      && echo "  (identicas salvo runs/seed)"
  else
    echo "  falta setup: monolitico=$([ -f "$sm" ] && echo si || echo NO) chunk=$([ -f "$sc" ] && echo si || echo NO)"
  fi
done
