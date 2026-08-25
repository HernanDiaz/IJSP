#!/usr/bin/env bash
# ¿Es el BINARIO la causa de que fEABCLS y TSN2 empeoren en TODOS los brazos,
# incluido el control que no usa semillas?
# Se ejecuta la MISMA configuracion, misma instancia y misma semilla, con el
# binario de la corrida antigua (FuzzyFW.phase2) y con el actual, y se comparan
# los E[Cmax]. Si difieren de forma sistematica, el binario cambio de conducta.
cd "$(dirname "$0")/.."
INST=${1:-tai30_20_01}; ALGO=${2:-tsn2}; BUD=${3:-60}; RUNS=${4:-5}
T=/tmp/bincmp; rm -rf $T; mkdir -p $T/old $T/new
mk() {
  sed -e "s/^runs = .*/runs = $RUNS/" -e "s/^seed = .*/seed = 1/" "repro/setup_${ALGO}.txt" > "$1/setup.txt"
  sed -i "s/^noimprovement = .*/generations = 100000/" "$1/setup.txt"
  grep -q '^timelimit' "$1/setup.txt" && sed -i "s/^timelimit = .*/timelimit = $BUD/" "$1/setup.txt" \
                                      || echo "timelimit = $BUD" >> "$1/setup.txt"
}
mk $T/old; mk $T/new
f="SelectosYTaillardIntervalos/$INST.F.15_01.txt"
echo "=== $ALGO / $INST / brazo A0 (sin semillas) / $BUD s por run / $RUNS runs ==="
./FuzzyFW.phase2 $T/old/setup.txt "$f" $T/old > $T/old/log 2>&1
./FuzzyFW        $T/new/setup.txt "$f" $T/new > $T/new/log 2>&1
ec() { awk -F';' 'NR>1{v=$3; gsub(/[() ]/,"",v); split(v,ab,","); printf "%.1f\n", (ab[1]+ab[2])/2}' \
        "$(ls $1/*_Sols.csv 2>/dev/null | head -1)"; }
paste <(ec $T/old) <(ec $T/new) | awk '
  BEGIN{ printf "%-5s %12s %12s %10s\n", "run", "binario viejo", "binario nuevo", "dif" }
  { d=$2-$1; s+=d; n++; printf "%-5d %12.1f %12.1f %+10.1f\n", NR, $1, $2, d }
  END{ if(n) printf "\nmedia de la diferencia: %+.2f  (positivo = el nuevo es PEOR)\n", s/n }'
echo ""
echo "=== generaciones alcanzadas en el mismo presupuesto ==="
for v in old new; do
  c=$(ls $T/$v/*.csv 2>/dev/null | grep -v Sols | head -1)
  g=$(awk -F';' '/^Step;|^Generation;/{h=1;next} h && $1 ~ /^[0-9]/ {last=$1} END{print last}' "$c")
  echo "  binario $v: ultimo paso registrado = $g"
done
