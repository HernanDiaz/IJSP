#!/usr/bin/env bash
# Calidad de los pools de siembra por instancia y generador: media, mejor y
# desviacion del E[Cmax] de las 250 lineas que se inyectan (las mismas que usa
# el brazo @100%). Salida: final/pool_quality.csv
# NOTA: los intervalos almacenados provienen del decodificador SEMIACTIVO; los
# algoritmos decodifican con INSERCION (schedules activos), que da makespans
# iguales o mejores. Es por tanto un proxy PESIMISTA de lo que ve el algoritmo,
# consistente entre instancias (que es lo que importa para el predictor).
cd "$(dirname "$0")/.."
out=final/pool_quality.csv
echo "inst,gen,pool_mean,pool_best,pool_sd" > "$out"
INSTS="ft10"
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
for inst in $INSTS; do
  for g in v2 graspmor gtmwkr gp mix; do
    p="pools_test/corrected/int__${inst}_${g}_repo_pool.csv"
    [ -f "$p" ] || continue
    head -250 "$p" | awk -F';' -v i="$inst" -v g="$g" '
      { s=$2; gsub(/[\[\] ]/,"",s); split(s,ab,","); e=(ab[1]+ab[2])/2;
        n++; sum+=e; sq+=e*e; if(b==""||e<b) b=e }
      END{ if(n){ m=sum/n; sd=sqrt(sq/n - m*m); printf "%s,%s,%.2f,%.2f,%.2f\n", i, g, m, b, sd } }' >> "$out"
  done
done
echo "$out: $(( $(wc -l < "$out") - 1 )) filas"
