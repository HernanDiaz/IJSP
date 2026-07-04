#!/bin/bash
# Broadened regression smokes after the Phase-1 C++ changes (ProblemIJSP
# loadFile touched). verify_refactor.sh no longer exists in the repo, so this
# is the pragmatic gate: diverse loading paths + both objectives, runs=1 tl=10.
set -u
REPO=/mnt/c/Users/diazhernan/CLionProjects/Fable/IJSP
EXE=/mnt/c/Users/diazhernan/CLionProjects/Fable/FuzzyFW
cd "$REPO"
OUT=/tmp/regr2; rm -rf "$OUT"; mkdir -p "$OUT"

# makespan-only setup (paper N2 config, 1 run, 10 s)
sed -e 's/^runs = .*/runs = 1/' -e 's/^timelimit = .*/timelimit = 10/' \
  experiments/cor_tabu_2026/setup/setup_N2_tuned.txt > "$OUT/mk.txt"
# lexicographic makespan-energy setup
cp experiments/mo_green_2026/setup/setup_LexME_smoke.txt "$OUT/lex.txt"

run_case () {  # label setup instdir instance
  local label="$1" setup="$2" dir="$3" inst="$4"
  local sub="$OUT/$label"; mkdir -p "$sub"
  if timeout 120 "$EXE" "$setup" "$dir/$inst.txt" "$sub/" > "$sub/run.log" 2>&1; then
    local best
    best=$(grep -h "Best solution" "$sub"/*.csv 2>/dev/null | head -1 | cut -d';' -f2)
    echo "$label ($inst): best=$best  OK"
  else
    echo "$label ($inst): FAILED  (see $sub/run.log)"
    tail -2 "$sub/run.log"
  fi
}

run_case mk_formatB   "$OUT/mk.txt"  SelectosYTaillardIntervalos        F0.15.0.la29_03
run_case mk_taillard  "$OUT/mk.txt"  SelectosYTaillardIntervalos        tai15_15_01.F.15_01
run_case mk_energyinst "$OUT/mk.txt" SelectosYTaillardIntervalosEnergia F0.15.0.ft10_10
run_case lex_formatB  "$OUT/lex.txt" SelectosYTaillardIntervalosEnergia F0.15.0.la29_03
run_case lex_taillard "$OUT/lex.txt" SelectosYTaillardIntervalosEnergia tai15_15_01.F.15_01
