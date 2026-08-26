#!/usr/bin/env bash
# BRAZO MIXH: pool mixto sembrando SOLO LA MITAD de la poblacion (125 de 250).
#
# Por que este brazo. El diseno tiene el pool v2 a dos fracciones (V2 al 100%,
# V2H al 50%) pero el pool mixto solo al 100%. Las dos afirmaciones de
# mecanismo del articulo se miden por tanto sobre pools distintos:
#   - la composicion manda sobre la calidad  -> medido a fraccion 100%
#   - la fraccion interactua con el solver   -> medido sobre v2 puro
# Falta saber si son EL MISMO mecanismo o dos. En el GA retener material
# aleatorio ayuda (V2H mejor que V2, -0.39 pp) y componer tambien (MIX no dana,
# los generadores unicos si). Si MIXH mejora sobre MIX, son dos efectos que se
# suman; si no, la composicion ya estaba haciendo el trabajo de la aleatoriedad.
#
# NO HACEN FALTA POOLS NUEVOS: MIXH usa el mismo pool mixto que MIX, igual que
# V2H usa el mismo pool que V2. Solo cambia creation.seed.count.
#
# HOMOGENEIDAD. Este brazo debe correr en condiciones identicas a los otros
# siete o la comparacion no vale. Se reutiliza run_phase2c.sh sin tocar su
# logica, de modo que hereda los mismos presupuestos por (solver, clase), el
# mismo troceado en 6 x 5 ejecuciones, el mismo offset global de semillas y los
# mismos setups de repro/. La lista de brazos se restringe a MIXH: los trozos ya
# hechos se saltarian igualmente, pero restringirla hace imposible tocarlos.
#
# CONSECUENCIA ESTADISTICA que hay que declarar en el articulo: la correccion de
# Holm pasa de 6 a 7 contrastes por solver, asi que TODOS los p-valores se
# mueven. Hay que recalcular las tablas afectadas, no solo anadir una fila.
#
# Uso:   bash final/run_mixh.sh            (lanza)
#        DRYRUN=1 bash final/run_mixh.sh   (solo cuenta trabajos)
cd "$(dirname "$0")/.."

export ARMS_OVERRIDE="MIXH"
export ALGOS="ga abce3 feabcls tsn2"
export PAR=${PAR:-14}

echo "=== MIXH: comprobaciones previas ==="

# 1. binario presente y mas nuevo que las fuentes que le afectan.
# El bit de ejecucion no se comprueba: sobre un montaje de Windows no aparece
# aunque el binario corra sin problemas desde WSL.
[ -s ./FuzzyFW ] || { echo "  FALLO: no existe ./FuzzyFW"; exit 1; }
for src in CreationIJSP.cpp CreationIJSP.h Creation.cpp; do
  [ -f "$src" ] || continue
  [ "$src" -nt ./FuzzyFW ] && { echo "  FALLO: $src es mas reciente que el binario;"; \
    echo "         recompila antes de lanzar o el brazo no sera homogeneo"; exit 1; }
done
echo "  binario: presente y posterior a las fuentes de siembra"

# 2. los 61 pools mixtos existen y tienen 1024 lineas
n=0; bad=0
INSTS="ft10"
for c in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${c}_${i}"; done
done
for inst in $INSTS; do
  p="pools_test/corrected/int__${inst}_mix_repo_pool.csv"
  if [ ! -f "$p" ]; then echo "  FALTA $p"; bad=$((bad+1)); continue; fi
  l=$(wc -l < "$p"); [ "$l" -eq 1024 ] || { echo "  $p tiene $l lineas"; bad=$((bad+1)); }
  n=$((n+1))
done
[ "$bad" -eq 0 ] || { echo "  FALLO: $bad pools defectuosos"; exit 1; }
echo "  pools mixtos: $n de 61, todos con 1024 entradas"

# 3. ninguna celda MIXH previa: esta corrida debe crear, nunca sobrescribir
prev=$(ls -d final/phase2/*/*/MIXH 2>/dev/null | wc -l)
[ "$prev" -eq 0 ] || { echo "  FALLO: ya existen $prev celdas MIXH"; exit 1; }
echo "  celdas MIXH previas: 0"

# 4. los otros brazos estan completos, para que el nuevo no compita por CPU.
# Un solo recorrido del arbol: 1708 celdas con ls una a una tarda minutos sobre
# un montaje de Windows.
comp=$(find final/phase2 -mindepth 3 -maxdepth 3 -type d 2>/dev/null | wc -l)
esp=$((4*61*7))
echo "  celdas originales con datos: $comp de $esp"
[ "$comp" -ge "$esp" ] || echo "  AVISO: faltan $((esp-comp)); MIXH competiria por CPU con ellas"

# 5. cuantos trabajos va a generar
echo ""
echo "=== trabajos a generar ==="
DRYRUN=1 bash final/run_phase2c.sh 2>&1 | tail -2

if [ "${DRYRUN:-0}" = "1" ]; then
  echo ""
  echo "DRYRUN: nada lanzado. Para ejecutar: bash final/run_mixh.sh"
  exit 0
fi

echo ""
echo "=== lanzando MIXH con $PAR workers ==="
echo "  coste estimado: ~50 h de CPU / 14 workers = ~2.1 dias"
bash final/run_phase2c.sh
