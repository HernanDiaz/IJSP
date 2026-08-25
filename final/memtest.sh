#!/usr/bin/env bash
# Mide el peor caso de memoria: 1 worker TSN2 en 50x20 (1000 operaciones).
cd "$(dirname "$0")/.."
d=final/memtest; rm -rf "$d"; mkdir -p "$d"
sed -e 's/^runs = .*/runs = 2/' -e 's/^noimprovement = .*/generations = 100000/' repro/setup_tsn2.txt > "$d/setup.txt"
sed -i 's/^timelimit = .*/timelimit = 120/' "$d/setup.txt"
# sembrado (V2, k=250: el caso con mas estructuras vivas)
sed -i 's/^creation = ijsp.random/creation = ijsp.seeded/' "$d/setup.txt"
printf '\ncreation.seed.pool = pools_test/corrected/int__tai50_20_01_v2_repo_pool.csv\ncreation.seed.count = 250\n' >> "$d/setup.txt"
nohup ./FuzzyFW "$d/setup.txt" SelectosYTaillardIntervalos/tai50_20_01.F.15_01.txt "$d" > "$d/log.txt" 2>&1 &
echo "lanzado pid=$!"
