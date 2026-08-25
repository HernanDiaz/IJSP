#!/usr/bin/env bash
# Compila el binario completo FuzzyFW (incluye main.cpp) en WSL.
set -e
cd "$(dirname "$0")"

# Todos los .cpp salvo scaffolding de test y backups con espacios (SI incluye main.cpp)
ls *.cpp | grep -v -E '^(mainTest|IntervalTest|seed_consistency_test)\.cpp$' \
         | grep -v -E ' |copia|copy' > srclist_full.txt
echo "Fuentes: $(wc -l < srclist_full.txt)"

g++ -O2 -std=c++11 -fpermissive -I. -o FuzzyFW $(cat srclist_full.txt) 2> build_full_errors.txt
echo "compile exit=$?"
ls -la FuzzyFW
