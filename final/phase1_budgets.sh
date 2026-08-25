#!/usr/bin/env bash
# Tabla de presupuestos de Fase 2: t_conv(A0) por (algo, instancia calibrada) y
# presupuesto de clase = min(1.5 x mediana_clase(t_conv), 900s).
# t_conv = primer runtime (s, CPU, traza media) con bestCmax <= final*1.001.
# Fuentes A0: ga/feabcls -> final/phase1/<algo>/<inst>; abce3/tsn2 -> phase0 F00.
cd "$(dirname "$0")/.."
tconv() { # $1=trace csv
  awk -F';' 'BEGIN{r=-1} /^Evolution/{f=1} f&&$1~/^[0-9]+$/{g[++m]=$2; v[m]=$5; fin=$5}
    END{ thr=fin*1.001; for(k=1;k<=m;k++){ if(v[k]<=thr){ printf "%.1f", g[k]; exit } } }' "$1"
}
src() { # $1=algo $2=inst -> trace csv
  case "$1" in
    ga|feabcls) ls "final/phase1/$1/$2"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1 ;;
    *) ls "final/phase0/$1/$2/F00"/*.csv 2>/dev/null | grep -v -E 'Sols|Robust|Scenar' | head -1 ;;
  esac
}
declare -A CLS=( [tai15_15_01]=15x15 [tai20_20_02]=20x20 [tai30_15_01]=30x15
                 [tai30_20_04]=30x20 [tai50_15_01]=50x15 [tai50_15_05]=50x15
                 [tai50_20_01]=50x20 [tai50_20_05]=50x20 )
echo "algo,inst,clase,t_conv_s"
for algo in ga abce3 feabc feabcls tsn2; do
  [ "$algo" = "feabc" ] && continue
  for inst in tai15_15_01 tai20_20_02 tai30_15_01 tai30_20_04 tai50_15_01 tai50_15_05 tai50_20_01 tai50_20_05; do
    c=$(src $algo $inst); [ -z "$c" ] && continue
    t=$(tconv "$c")
    echo "$algo,$inst,${CLS[$inst]},$t"
  done
done > final/phase1/tconv.csv
cat final/phase1/tconv.csv
echo ""
echo "=== PRESUPUESTO por (algo, clase) = min(1.5 x mediana t_conv, 900) ==="
awk -F',' 'NR>1{ key=$1"_"$3; vals[key]=vals[key]" "$4 }
  END{ for(k in vals){ n=split(vals[k],a," "); asort_n(a,n);
       med=(n%2)?a[int(n/2)+1]:(a[n/2]+a[n/2+1])/2;
       b=1.5*med; if(b>900)b=900; printf "%-16s tconv_med=%7.1fs  presupuesto=%6.0fs\n", k, med, b } }
  function asort_n(arr,n, i,j,t){ for(i=1;i<n;i++)for(j=i+1;j<=n;j++)if(arr[j]<arr[i]){t=arr[i];arr[i]=arr[j];arr[j]=t} }' final/phase1/tconv.csv | sort