#!/usr/bin/env bash
set -e
cd "$(dirname "$0")"
ls *.cpp | grep -v -E '^(main|mainTest|IntervalTest|seed_consistency_test|verify_solution)\.cpp$' \
         | grep -v -E ' |copia|copy' > srclist_ver.txt
echo "verify_solution.cpp" >> srclist_ver.txt
echo "Fuentes: $(wc -l < srclist_ver.txt)"
g++ -O2 -std=c++11 -fpermissive -I. -o verify_solution $(cat srclist_ver.txt) 2> build_verify_err.txt
echo "compile exit=$?"
ls -la verify_solution
