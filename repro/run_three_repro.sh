#!/usr/bin/env bash
# Replicacion ABCE3, fEABC, fEABCLS: 30 runs x 12 instancias clasicas, parsea best/avg RE.
cd "$(dirname "$0")/.."

read -r -d '' ROWS <<'EOF'
F0.15.0.ft10_10.txt   930  FT10
F0.15.0.ft20_05.txt  1165  FT20
F0.15.0.la21_04.txt  1046  LA21
F0.15.0.la24_03.txt   935  LA24
F0.15.0.la25_04.txt   977  LA25
F0.15.0.la27_09.txt  1235  LA27
F0.15.0.la29_03.txt  1152  LA29
F0.15.0.la38_06.txt  1196  LA38
F0.15.0.la40_05.txt  1222  LA40
F0.15.0.abz7_06.txt   656  ABZ7
F0.15.0.abz8_05.txt   645  ABZ8
F0.15.0.abz9_10.txt   661  ABZ9
EOF

for algo in abce3 feabc feabcls; do
  SET="repro/setup_$algo.txt"
  OUT="repro/out_$algo"
  mkdir -p "$OUT"
  echo "############################################################"
  echo "# $algo"
  echo "############################################################"
  printf "%-6s %5s  %-16s %-18s\n" "Inst" "LB" "best(RE%)" "avg(RE%)"
  echo "-------------------------------------------------------"
  while read -r file lb label; do
    [ -z "$file" ] && continue
    d="$OUT/$label"; rm -rf "$d"; mkdir -p "$d"
    ./FuzzyFW "$SET" "SelectosYTaillardIntervalos/$file" "$d" > "$d/stdout.log" 2>&1
    sols=$(ls "$d"/*_Sols.csv 2>/dev/null | head -1)
    if [ -z "$sols" ]; then printf "%-6s %5s  SIN SALIDA\n" "$label" "$lb"; continue; fi
    awk -F';' -v lb="$lb" -v label="$label" '
      NR>1 && $3 ~ /\(/ { s=$3; gsub(/[()]/,"",s); split(s,ab,","); a=ab[1]+0; b=ab[2]+0;
        ec=(a+b)/2.0; n++; sum+=ec; if(best==""||ec<best) best=ec; }
      END { if(n==0){printf "%-6s %5s  sin datos\n",label,lb}
            else { br=100*(best-lb)/lb; ar=100*(sum/n-lb)/lb;
              printf "%-6s %5s  %6.1f (%4.1f%%)   %7.2f (%4.1f%%)  [n=%d]\n",label,lb,best,br,sum/n,ar,n; } }' "$sols"
  done <<< "$ROWS"
  echo ""
done
