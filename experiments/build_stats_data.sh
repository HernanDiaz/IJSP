#!/bin/bash
#
# build_stats_data.sh
# Extracts individual run makespans from all _Sols.csv files into a single
# long-format CSV: one row per (config, instance, run).
# Uses only the latest _Sols.csv per instance (avoids duplicates).
#
# Output: statistical_results/runs_data.csv
# Columns: config,neigh,comp,instance,run,lower,upper,midpoint
#

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RESULTS_BASE="${SCRIPT_DIR}/results"
OUT_DIR="${SCRIPT_DIR}/statistical_results"
OUT_FILE="${OUT_DIR}/runs_data.csv"

NEIGHBOURHOODS=("n1" "n2" "n3" "nh" "next")
COMPARATORS=("EV" "LEX1" "LEX2" "YX")

mkdir -p "$OUT_DIR"
echo "config,neigh,comp,instance,run,lower,upper,midpoint" > "$OUT_FILE"

total=0

for nb in "${NEIGHBOURHOODS[@]}"; do
    for comp in "${COMPARATORS[@]}"; do
        config="${nb}_${comp}"
        RESULTS_DIR="${RESULTS_BASE}/${config}"

        # Collect latest _Sols.csv per instance using awk to parse everything at once
        # 1) List all Sols files, pick latest per instance stem
        declare -A latest_sols
        while IFS= read -r fname; do
            stem="${fname%_*_Sols.csv}"   # strip _TIMESTAMP_Sols.csv
            stem="${stem##*/}"            # strip path
            full="${RESULTS_DIR}/${fname}"
            # keep lexicographically latest (timestamp is in filename)
            if [[ -z "${latest_sols[$stem]}" || "$full" > "${latest_sols[$stem]}" ]]; then
                latest_sols["$stem"]="$full"
            fi
        done < <(ls "${RESULTS_DIR}/" 2>/dev/null | grep -E '_[0-9]{14}_Sols\.csv$')

        # 2) Process all latest Sols files with a single awk call per config
        for stem in "${!latest_sols[@]}"; do
            # Skip tai100
            [[ "$stem" == tai100* ]] && continue

            sols="${latest_sols[$stem]}"
            # awk parses "Run;Solution;(lower, upper)" rows
            count=$(awk -F';' -v cfg="$config" -v nb="$nb" -v comp="$comp" \
                        -v inst="$stem" '
                NR == 1 { next }   # skip header
                {
                    run = $1
                    obj = $3        # "(lower, upper)"
                    # extract two integers
                    if (match(obj, /\(([0-9]+),[ ]*([0-9]+)\)/, arr)) {
                        lo = arr[1]; hi = arr[2]
                        mid = (lo + hi) / 2.0
                        printf "%s,%s,%s,%s,%s,%s,%s,%.4f\n", \
                            cfg, nb, comp, inst, run, lo, hi, mid
                    }
                }
            ' "$sols" | tee -a "$OUT_FILE" | wc -l)
            total=$((total + count))
        done
        unset latest_sols

        echo "  ${config}: done"
    done
done

echo ""
echo "Done. Total rows: $total"
echo "Output: $OUT_FILE"
wc -l "$OUT_FILE"
