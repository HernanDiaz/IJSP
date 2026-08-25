#!/usr/bin/env bash
# Compila el test de consistencia en WSL (g++ nativo Linux, misma rama que el paper).
set -e
cd "$(dirname "$0")"

# Todos los .cpp salvo los que traen main/scaffolding de test y backups con espacios
ls *.cpp | grep -v -E '^(main|mainTest|IntervalTest|seed_consistency_test)\.cpp$' \
         | grep -v -E ' |copia|copy' > srclist_wsl.txt
echo "seed_consistency_test.cpp" >> srclist_wsl.txt
echo "Fuentes: $(wc -l < srclist_wsl.txt)"

g++ -O2 -std=c++11 -fpermissive -I. -o seed_consistency_test $(cat srclist_wsl.txt) 2> build_errors_wsl.txt
echo "compile exit=$?"
