#!/usr/bin/env bash
# AUDITORIA DE CONFIGURACIONES: dentro de un solver y una instancia, los siete
# brazos deben ser IDENTICOS salvo en las lineas de siembra. Cualquier otra
# diferencia (presupuesto, poblacion, operadores, criterio de parada) sesgaria
# la comparacion entre brazos, que es la inferencia central del paper.
cd "$(dirname "$0")/.."
ARMS="A0 V2H V2 MOR GT GP MIX"
IGN='^(creation|creation\.seed\.pool|creation\.seed\.count|runs|seed) '

echo "=== 1) los 7 brazos coinciden salvo en la siembra? ==="
difs=0; comp=0
for algo in ga abce3 feabcls tsn2; do
  for inst in ft10 tai15_15_03 tai20_20_05 tai30_15_07 tai30_20_01 tai50_15_04 tai50_20_09; do
    ref=""
    for arm in $ARMS; do
      d="final/phase2/$algo/$inst/$arm"
      s="$d/setup.txt"; [ -f "$s" ] || s="$d/c1/setup.txt"
      [ -f "$s" ] || continue
      cur=$(grep -vE "$IGN" "$s" | grep -v '^$' | sort)
      if [ -z "$ref" ]; then ref="$cur"; refarm=$arm
      else
        comp=$((comp+1))
        if [ "$cur" != "$ref" ]; then
          difs=$((difs+1))
          echo "  DIFERENCIA $algo/$inst: $arm vs $refarm"
          diff <(echo "$ref") <(echo "$cur") | head -6 | sed 's/^/      /'
        fi
      fi
    done
  done
done
echo "  comparaciones: $comp | con diferencias: $difs"

echo ""
echo "=== 2) el presupuesto es el previsto para cada solver y clase? ==="
declare -A EXP=(
 [ga:15x15]=60 [ga:20x20]=60 [ga:30x15]=109 [ga:30x20]=154 [ga:50x15]=233 [ga:50x20]=321
 [abce3:15x15]=60 [abce3:20x20]=60 [abce3:30x15]=60 [abce3:30x20]=60 [abce3:50x15]=60 [abce3:50x20]=60
 [feabcls:15x15]=60 [feabcls:20x20]=67 [feabcls:30x15]=371 [feabcls:30x20]=900 [feabcls:50x15]=900 [feabcls:50x20]=900
 [tsn2:15x15]=60 [tsn2:20x20]=486 [tsn2:30x15]=630 [tsn2:30x20]=900 [tsn2:50x15]=900 [tsn2:50x20]=900 )
clase() { case "$1" in ft10|tai15_15_*) echo 15x15;; tai20_20_*) echo 20x20;; tai30_15_*) echo 30x15;;
  tai30_20_*) echo 30x20;; tai50_15_*) echo 50x15;; tai50_20_*) echo 50x20;; esac; }
malb=0; okb=0
for algo in ga abce3 feabcls tsn2; do
  for inst in ft10 tai15_15_03 tai20_20_05 tai30_15_07 tai30_20_01 tai50_15_04 tai50_20_09; do
    cl=$(clase "$inst"); e=${EXP[$algo:$cl]}
    for arm in $ARMS; do
      d="final/phase2/$algo/$inst/$arm"; s="$d/setup.txt"; [ -f "$s" ] || s="$d/c1/setup.txt"
      [ -f "$s" ] || continue
      t=$(grep -E '^timelimit' "$s" | tr -dc '0-9')
      if [ "$t" != "$e" ]; then echo "  PRESUPUESTO $algo/$inst/$arm: $t (esperado $e)"; malb=$((malb+1)); else okb=$((okb+1)); fi
    done
  done
done
echo "  correctos: $okb | incorrectos: $malb"

echo ""
echo "=== 3) la siembra es la que dice el brazo? (pool y numero de semillas) ==="
mals=0; oks=0
for algo in ga abce3 feabcls tsn2; do
  for inst in tai30_20_01 tai50_20_09; do
    for arm in $ARMS; do
      d="final/phase2/$algo/$inst/$arm"; s="$d/setup.txt"; [ -f "$s" ] || s="$d/c1/setup.txt"
      [ -f "$s" ] || continue
      p=$(grep -E '^creation.seed.pool' "$s" | sed 's/.*int__//;s/_repo_pool.*//;s/^[a-z0-9_]*_//')
      k=$(grep -E '^creation.seed.count' "$s" | tr -dc '0-9')
      cr=$(grep -E '^creation ' "$s" | awk '{print $3}')
      case "$arm" in
        A0)  exp_p=""; exp_k=""; exp_c="ijsp.random" ;;
        V2H) exp_p="v2"; exp_k=125; exp_c="ijsp.seeded" ;;
        V2)  exp_p="v2"; exp_k=250; exp_c="ijsp.seeded" ;;
        MOR) exp_p="graspmor"; exp_k=250; exp_c="ijsp.seeded" ;;
        GT)  exp_p="gtmwkr"; exp_k=250; exp_c="ijsp.seeded" ;;
        GP)  exp_p="gp"; exp_k=250; exp_c="ijsp.seeded" ;;
        MIX) exp_p="mix"; exp_k=250; exp_c="ijsp.seeded" ;;
      esac
      if [ "$p" != "$exp_p" ] || [ "$k" != "$exp_k" ] || [ "$cr" != "$exp_c" ]; then
        echo "  SIEMBRA $algo/$inst/$arm: creation=$cr pool=$p k=$k (esperado $exp_c/$exp_p/$exp_k)"; mals=$((mals+1))
      else oks=$((oks+1)); fi
    done
  done
done
echo "  correctos: $oks | incorrectos: $mals"
