#!/usr/bin/env bash
# VERIFICACION DEL MAPEO instancia_repo -> instancia ta de Taillard.
#
# No damos por supuesto que tai{n}_{m}_{k} sea la ta correspondiente: lo
# comprobamos. Para cada instancia CRISP del repo calculamos la cota trivial
#   LBtriv = max( max_j sum_k p_jk , max_m sum_{(j,k): maq=m} p_jk )
# y la comparamos con la LB publicada. Condiciones:
#   (a) LBtriv <= LB_pub   SIEMPRE (si falla, el mapeo es imposible)
#   (b) en las instancias grandes cerradas (ta51-ta70) la LB publicada ES la
#       cota de maquina, asi que LBtriv == LB_pub  -> identificacion exacta.
CRISP=${CRISP:-/mnt/c/Users/diazhernan/CLionProjects/IJSP/SelectosYTaillardCrisp}
PUB=${1:?uso: check_ta_mapping.sh <ta_lb.csv>}

bounds() {  # imprime "jobbound machinebound"
  tr -d '\r' < "$1" | awk '
    /NUMERO DE TRABAJOS/{g="n";next} /NUMERO DE RECURSOS/{g="m";next}
    /SECUENCIA DE MAQUINAS/{g="seq";r=0;next} /DURACIONES/{g="dur";r=0;next}
    /TIEMPOS MAXIMOS/{g="";next}
    g=="n"{n=$1+0;g="";next} g=="m"{m=$1+0;g="";next}
    g=="seq" && NF>0 {r++; for(k=1;k<=NF;k++) S[r,k]=$k+0; next}
    g=="dur" && NF>0 {
      r++; gsub(/[(),]/," "); nf=split($0,a," ");
      # cada duracion viene como "(p, p)" -> tras limpiar quedan pares repetidos
      for(k=1;k<=nf/2;k++){ p=a[2*k-1]+0; D[r,k]=p; job[r]+=p; mac[S[r,k]]+=p }
      next
    }
    END{ jb=0; for(j=1;j<=n;j++) if(job[j]>jb) jb=job[j];
         mb=0; for(i=0;i<m;i++) if(mac[i]>mb) mb=mac[i];
         print jb, mb }'
}

printf "%-14s %6s %8s %8s %8s   %s\n" inst ta LBtriv LB_pub UB_pub veredicto
ok=0; bad=0; exact=0
while IFS=, read -r inst ta lb ub; do
  [ "$inst" = inst ] && continue
  f=$(ls "$CRISP"/${inst}.*.txt 2>/dev/null | head -1)
  [ -z "$f" ] && { printf "%-14s %6s %8s %8s %8s   SIN FICHERO CRISP\n" "$inst" "$ta" - "$lb" "$ub"; continue; }
  read -r jb mb <<< "$(bounds "$f")"
  t=$jb; [ "$mb" -gt "$t" ] && t=$mb
  if   [ "$t" -eq "$lb" ]; then v="EXACTO (LBtriv = LB publicada)"; exact=$((exact+1)); ok=$((ok+1))
  elif [ "$t" -le "$lb" ]; then v="coherente (LBtriv <= LB)";        ok=$((ok+1))
  else v="*** INCOMPATIBLE ***";                                    bad=$((bad+1)); fi
  printf "%-14s %6s %8d %8s %8s   %s\n" "$inst" "$ta" "$t" "$lb" "$ub" "$v"
done < "$PUB"
echo
echo "coherentes: $ok   (de ellos con identificacion EXACTA: $exact)   incompatibles: $bad"
