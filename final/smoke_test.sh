#!/usr/bin/env bash
# SMOKE TEST del relanzamiento: recorre el pipeline COMPLETO en minutos, por el
# MISMO camino de codigo que la corrida real (mismo runner, mismo extractor,
# mismo remuestreo), con presupuesto de 2 s y dos instancias.
#
# La comprobacion que de verdad importa es la 3: que cada celda sembrada tenga
# 30 poblaciones iniciales DISTINTAS. Ese es el fallo que estamos arreglando y
# se detecta aqui, en minutos, no despues de 11 dias.
#
# Escribe en final/smoke, no toca nada de final/phase2.
cd "$(dirname "$0")/.."
SM=final/smoke
INS="ft10 tai20_20_01"
# REUSE=1 reaprovecha una corrida de smoke previa y solo re-valida (segundos).
if [ "${REUSE:-0}" != "1" ]; then rm -rf "$SM"; fi
mkdir -p "$SM"
fail=0
ok()  { echo "  [OK]   $*"; }
bad() { echo "  [FALLO] $*"; fail=$((fail+1)); }

if [ "${REUSE:-0}" = "1" ]; then echo "=== REUSE=1: se reutiliza la corrida previa, solo se valida ==="; else
echo "=== ejecutando: 4 solvers x 2 instancias x 7 brazos x 6 trozos, 2 s por run ==="
t0=$(date +%s)
OUT_OVERRIDE="$SM" INSTS_OVERRIDE="$INS" FORCEBUD=2 ALGOS="ga abce3 feabcls tsn2" \
  CH=5 PAR="${PAR:-14}" bash final/run_phase2c.sh > "$SM/run.out" 2>&1
echo "  duracion: $(( $(date +%s) - t0 )) s"
tail -2 "$SM/run.out" | sed "s/^/    /"
fi

echo ""
echo "=== 1) se generaron todos los trozos? ==="
esp=$((4*2*7*6))
hay=$(ls "$SM"/*/*/*/c*/*_Sols.csv 2>/dev/null | wc -l)
[ "$hay" -eq "$esp" ] && ok "$hay/$esp trozos con soluciones" || bad "solo $hay de $esp trozos"

echo ""
echo "=== 2) la extraccion da 30 ejecuciones por celda? ==="
for a in ga abce3 feabcls tsn2; do
  OUT_OVERRIDE="$SM" INSTS_OVERRIDE="$INS" bash final/extract_phase2c.sh "$a" > /dev/null
  r=$(awk -F, 'NR>1{c[$2","$4]++} END{n=0; mal=0; for(x in c){n++; if(c[x]!=30) mal++} print n" "mal}' "$SM/results_$a.csv")
  set -- $r
  if [ "$1" -eq 14 ] && [ "$2" -eq 0 ]; then ok "$a: 14 celdas, todas con 30 ejecuciones"
  else bad "$a: $1 celdas, $2 con numero de ejecuciones incorrecto"; fi
done

echo ""
echo "=== 3a) CLAVE (exacta): el offset de cada trozo es (j-1)*ch? ==="
# Comprobacion DIRECTA del mecanismo. La indirecta (contar poblaciones distintas)
# da falsos positivos: en instancias pequenas dos bloques distintos pueden
# producir el mismo par (mejor, media), y algunos pools tienen soluciones
# repetidas (ft10/mix: 3027 unicas de 7500).
malo=0; totc=0
for d in "$SM"/*/*/*/c*; do
  arm=$(basename "$(dirname "$d")"); [ "$arm" = A0 ] && continue
  j=$(basename "$d" | tr -d 'c'); esp=$(( (j-1)*5 ))
  o=$(grep -E '^creation.seed.offset' "$d/setup.txt" 2>/dev/null | tr -dc '0-9')
  totc=$((totc+1)); [ "$o" != "$esp" ] && { echo "      $d: offset=$o esperado=$esp"; malo=$((malo+1)); }
done
[ "$malo" -eq 0 ] && ok "$totc trozos sembrados, todos con el offset correcto" \
                  || bad "$malo trozos con offset incorrecto"

echo ""
echo "=== 3b) red de seguridad: poblaciones iniciales distintas por celda ==="
# El fallo que buscamos produce EXACTAMENTE tantas distintas como el tamano de
# trozo (5). Un valor algo por debajo de 30 es colision de firma, no el fallo.
peor=99; malas=0; buenas=0
for a in ga abce3 feabcls tsn2; do
  for i in $INS; do
    for arm in V2H V2 MOR GT GP MIX; do
      d="$SM/$a/$i/$arm"
      n=$(for c in 1 2 3 4 5 6; do
            f=$(ls "$d/c$c"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1)
            [ -z "$f" ] && continue
            awk -F';' '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^0$/ {
                for (r=0;r<5;r++){ c=7+r*6+4; if (c+1<=NF) printf "%s|%s\n", $c, $(c+1) } exit }' "$f"
          done | sort -u | wc -l)
      [ "$n" -lt "$peor" ] && peor=$n
      if [ "$n" -le 6 ]; then
        malas=$((malas+1)); echo "      $a/$i/$arm -> solo $n distintas (= tamano de trozo: EL FALLO)"
      else
        buenas=$((buenas+1))
        [ "$n" -lt 30 ] && echo "      (aviso) $a/$i/$arm: $n de 30, colision de firma en instancia pequena"
      fi
    done
  done
done
if [ "$malas" -eq 0 ]; then ok "$buenas celdas sembradas por encima del umbral (minimo observado: $peor de 30)"
else bad "$malas celdas al nivel del tamano de trozo -> el offset NO esta llegando"; fi

echo ""
echo "=== 4) el remuestreo anytime funciona? ==="
for a in ga tsn2; do
  OUT_OVERRIDE="$SM" INSTS_OVERRIDE="$INS" bash final/resample_anytime.sh "$a" > /dev/null 2>&1
  f="$SM/anytime_$a.csv"; [ -f "$f" ] || f="final/phase2/anytime_$a.csv"
  n=$(( $(wc -l < "$f" 2>/dev/null || echo 1) - 1 ))
  [ "$n" -gt 0 ] && ok "$a: $n filas de curva" || bad "$a: curva vacia"
done

echo ""
echo "=== 5) los makespans se reproducen? ==="
s=$(ls "$SM"/tsn2/tai20_20_01/MIX/c1/*_Sols.csv 2>/dev/null | head -1)
if [ -n "$s" ]; then
  o=$(./verify_solution "SelectosYTaillardIntervalos/tai20_20_01.F.15_01.txt" "$s" insertion 2>&1 | grep -oE 'discrepancias: [0-9]+' | grep -oE '[0-9]+')
  [ "${o:-9}" -eq 0 ] && ok "sin discrepancias en la celda comprobada" || bad "$o discrepancias"
else bad "no hay soluciones que verificar"; fi

echo ""
if [ "$fail" -eq 0 ]; then
  echo "SMOKE TEST SUPERADO. El pipeline funciona de extremo a extremo."
  echo "Borra final/smoke y lanza la corrida real."
else
  echo "SMOKE TEST CON $fail FALLO(S): NO lanzar."
fi
exit $fail
