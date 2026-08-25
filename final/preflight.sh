#!/usr/bin/env bash
# VERIFICACION PREVIA al relanzamiento. Cada comprobacion corresponde a un fallo
# SILENCIOSO que ya nos costo tiempo: no dieron error, simplemente produjeron
# datos equivocados con aspecto correcto.
#
#  1. binario:   el Makefile enlaza en ../FuzzyFW, no en ./FuzzyFW. Se ejecuto
#                durante horas un binario viejo creyendo haber recompilado.
#  2. offset:    runCounter se reinicia por proceso -> los trozos repetian los
#                mismos bloques del pool. Aqui se comprueba que 6 procesos de 5
#                asignan LAS MISMAS semillas que 1 proceso de 30.
#  3. sombra:    ficheros monoliticos sueltos eclipsaban los trozos al extraer.
#  4. guard:     el vigilante de memoria murio en silencio 11 horas.
#  5. entradas:  que existan todas las instancias y todos los pools.
cd "$(dirname "$0")/.."
fail=0
ok()   { echo "  [OK]   $*"; }
bad()  { echo "  [FALLO] $*"; fail=$((fail+1)); }

echo "=== 1) binario en uso ==="
if [ ../FuzzyFW -nt ./FuzzyFW ]; then
  bad "../FuzzyFW es mas reciente que ./FuzzyFW: falta copiarlo (cp ../FuzzyFW ./FuzzyFW)"
else
  ok "./FuzzyFW esta al dia ($(date -r ./FuzzyFW '+%Y-%m-%d %H:%M'))"
fi
./FuzzyFW 2>&1 | head -1 | grep -qi . && ok "el binario arranca" || bad "el binario no responde"

echo ""
echo "=== 2) el offset de semillas funciona (troceado == monolitico) ==="
INST=tai20_20_01; POOL=pools_test/corrected/int__${INST}_mix_repo_pool.csv
T=/tmp/pf; rm -rf $T; mkdir -p $T
mk() {  # $1=dir $2=runs $3=seed $4=offset
  sed -e "s/^runs = .*/runs = $2/" -e "s/^seed = .*/seed = $3/" repro/setup_ga.txt > "$1/setup.txt"
  sed -i "s/^noimprovement = .*/generations = 1/" "$1/setup.txt"
  grep -q '^timelimit' "$1/setup.txt" && sed -i "s/^timelimit = .*/timelimit = 1/" "$1/setup.txt" || echo "timelimit = 1" >> "$1/setup.txt"
  sed -i "s/^creation = ijsp.random/creation = ijsp.seeded/" "$1/setup.txt"
  { echo ""; echo "creation.seed.pool = $POOL"; echo "creation.seed.count = 250";
    echo "creation.seed.offset = $4"; } >> "$1/setup.txt"
}
ini() {  # firma de la poblacion inicial de cada ejecucion: (Best Cmax, Avg Cmax).
  # Solo con Best Cmax hay colisiones entre poblaciones distintas y el recuento
  # de "distintas" sale enganosamente bajo.
  awk -F';' '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {
      for (r=0; r<200; r++) { c=7+r*6+4
        if (c+1<=NF && $c != "") printf "%s|%s\n", $c, $(c+1) } exit }' "$1"
}
mkdir -p $T/mono; mk $T/mono 30 1 0
./FuzzyFW $T/mono/setup.txt SelectosYTaillardIntervalos/$INST.F.15_01.txt $T/mono > $T/mono/log 2>&1
ini "$(ls $T/mono/*.csv | grep -v Sols | head -1)" > $T/mono.txt
: > $T/chunk.txt
for j in 0 1 2 3 4 5; do
  d=$T/c$j; mkdir -p $d; mk $d 5 $((1+j*5)) $((j*5))
  ./FuzzyFW $d/setup.txt SelectosYTaillardIntervalos/$INST.F.15_01.txt $d > $d/log 2>&1
  ini "$(ls $d/*.csv | grep -v Sols | head -1)" >> $T/chunk.txt
done
nm=$(wc -l < $T/mono.txt); nc=$(wc -l < $T/chunk.txt)
if [ "$nm" -eq 30 ] && [ "$nc" -eq 30 ] && diff -q $T/mono.txt $T/chunk.txt >/dev/null; then
  ok "las 30 poblaciones iniciales coinciden exactamente entre ambos regimenes"
  ok "poblaciones iniciales DISTINTAS: $(sort -u $T/mono.txt | wc -l) de 30"
else
  bad "no coinciden (mono=$nm filas, trozos=$nc filas, difs=$(diff $T/mono.txt $T/chunk.txt | grep -c '^[<>]'))"
fi

echo ""
echo "=== 3) sin ficheros que puedan eclipsar los trozos ==="
n=$(find final/phase2 -mindepth 4 -maxdepth 4 -name '*_Sols.csv' 2>/dev/null | wc -l)
[ "$n" -eq 0 ] && ok "no quedan _Sols.csv sueltos a nivel de brazo" \
                || bad "$n ficheros monoliticos sueltos: archivalos antes de lanzar"

echo ""
echo "=== 4) vigilante de memoria ==="
if [ -f final/guard.csv ]; then
  last=$(tail -1 final/guard.csv | cut -d, -f1)
  ok "guard.csv presente (ultima muestra: $last) -- RELANZALO antes de empezar"
else bad "no hay guard.csv"; fi

echo ""
echo "=== 5) instancias y pools completos ==="
INSTS="ft10"; for c in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${c}_${i}"; done; done
falta=0
for i in $INSTS; do
  if [ "$i" = ft10 ]; then f="SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt"
  else f="SelectosYTaillardIntervalos/$i.F.15_01.txt"; fi
  [ -f "$f" ] || { echo "      falta instancia $f"; falta=$((falta+1)); }
  for g in v2 graspmor gtmwkr gp mix; do
    [ -f "pools_test/corrected/int__${i}_${g}_repo_pool.csv" ] || { echo "      falta pool $i/$g"; falta=$((falta+1)); }
  done
done
[ "$falta" -eq 0 ] && ok "61 instancias y 305 pools presentes" || bad "$falta entradas ausentes"

echo ""
echo "=== 6) pools MIX: 1024 entradas y entrelazado fino ==="
# Si se lanzase con los MIX antiguos (7500, ordenados por bloques), los bloques
# que dan la vuelta perderian la composicion. Se comprueba tamano y equilibrio.
malmix=0; nmix=0
for i in $INSTS; do
  f="pools_test/corrected/int__${i}_mix_repo_pool.csv"; [ -f "$f" ] || continue
  nmix=$((nmix+1))
  l=$(wc -l < "$f")
  [ "$l" -ne 1024 ] && { echo "      $i: $l entradas (esperado 1024)"; malmix=$((malmix+1)); }
done
[ "$malmix" -eq 0 ] && ok "$nmix pools MIX con 1024 entradas" \
                    || bad "$malmix pools MIX con tamano incorrecto: ejecuta final/build_mix_pools.sh"

echo ""
[ "$fail" -eq 0 ] && echo "PREVUELO SUPERADO: se puede lanzar." \
                  || echo "PREVUELO CON $fail FALLO(S): NO lanzar hasta resolverlos."
exit $fail
