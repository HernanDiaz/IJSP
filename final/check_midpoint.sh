#!/usr/bin/env bash
# ¿Es el punto medio del intervalo EXACTAMENTE la duracion crisp?
# Si lo es, la LB crisp publicada es una cota inferior VALIDA de E[Cmax] del
# problema con intervalos, porque:
#   mid(A+B) = mid(A)+mid(B)   y   mid(max(A,B)) >= max(mid A, mid B)
# (con el maximo componentwise), luego mid(Cmax_int(x)) >= Cmax_crisp(x) para
# TODA planificacion x, y por tanto min_x E[Cmax_int] >= optimo crisp >= LB.
# Comprobamos ademas que la secuencia de maquinas es identica en ambas versiones.
CR=${CR:-/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardCrisp}
IN=${IN:-/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardIntervalos}

dur()  { tr -d '\r' < "$1" | awk '/DURACIONES/{f=1;next} /TIEMPOS MAXIMOS/{f=0} f&&NF{gsub(/[(),]/," ");print}'; }
seqm() { tr -d '\r' < "$1" | awk '/SECUENCIA DE MAQUINAS/{f=1;next} /DURACIONES/{f=0} f&&NF{print}'; }

printf "%-14s %8s %10s %10s %8s   %s\n" inst ops "mid!=crisp" "asimetr." "seq" veredicto
tot_bad=0
for c in "$CR"/*.txt; do
  b=$(basename "$c"); inst=${b%%.*}
  i="$IN/$b"; [ -f "$i" ] || { echo "$inst : sin pareja"; continue; }
  read -r ops nmid nasym <<< "$(paste <(dur "$c") <(dur "$i") | awk '
    { nc=NF/4;                       # crisp aporta 2 col/op, intervalo otras 2
      # tras el paste, la fila es: crisp(2*n valores) intervalo(2*n valores)
      n=NF/4;
      for(k=1;k<=n;k++){
        cp=$(2*k-1)+0;                       # p crisp (a==b)
        lo=$(2*n + 2*k-1)+0; hi=$(2*n + 2*k)+0;
        ops++;
        if (lo+hi != 2*cp) mid++;            # el medio no es el crisp
        if (cp-lo != hi-cp) asym++;          # intervalo no simetrico
      } }
    END{ printf "%d %d %d\n", ops, mid+0, asym+0 }')"
  sd="OK"; cmp -s <(seqm "$c") <(seqm "$i") || sd="DISTINTA"
  v="OK"; [ "$nmid" -ne 0 ] && v="*** medio != crisp ***"
  [ "$sd" = DISTINTA ] && v="*** rutas distintas ***"
  [ "$v" != OK ] && tot_bad=$((tot_bad+1))
  printf "%-14s %8d %10d %10d %8s   %s\n" "$inst" "$ops" "$nmid" "$nasym" "$sd" "$v"
done
echo
echo "instancias con problema: $tot_bad"
