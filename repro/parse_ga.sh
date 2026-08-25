#!/usr/bin/env bash
cd "$(dirname "$0")/.."
OUT=repro/out_ga
read -r -d '' ROWS <<'EOF'
FT10 930
FT20 1165
LA21 1046
LA24 935
LA25 977
LA27 1235
LA29 1152
LA38 1196
LA40 1222
ABZ7 656
ABZ8 645
ABZ9 661
EOF
printf "%-6s %5s  %-16s %-16s\n" "Inst" "LB" "GA_best(RE%)" "GA_avg(RE%)"
echo "-------------------------------------------------------"
while read -r label lb; do
  [ -z "$label" ] && continue
  sols=$(ls "$OUT/$label"/*_Sols.csv 2>/dev/null | head -1)
  if [ -z "$sols" ]; then printf "%-6s %5s  SIN SALIDA\n" "$label" "$lb"; continue; fi
  awk -F';' -v lb="$lb" -v label="$label" '
    NR>1 && $3 ~ /\(/ {
      s=$3; gsub(/[()]/,"",s); split(s,ab,","); a=ab[1]+0; b=ab[2]+0;
      ec=(a+b)/2.0; n++; sum+=ec; if(best==""||ec<best) best=ec;
    }
    END {
      if(n==0){ printf "%-6s %5s  sin datos\n", label, lb }
      else {
        breRE=100*(best-lb)/lb; areRE=100*(sum/n-lb)/lb;
        printf "%-6s %5s  %6.1f (%4.1f%%)   %7.2f (%4.1f%%)   [n=%d]\n",
               label, lb, best, breRE, sum/n, areRE, n;
      }
    }' "$sols"
done <<< "$ROWS"
