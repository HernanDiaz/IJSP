#!/usr/bin/env bash
# Test de fuga de memoria: UN solo worker (tsn2, instancia grande, 6 runs) y
# muestreo de RSS cada 15s. Si el RSS crece monotonamente run a run -> fuga.
cd "$(dirname "$0")/.."
d=final/leaktest; rm -rf "$d"; mkdir -p "$d"
sed -e 's/^runs = .*/runs = 6/' -e 's/^noimprovement = .*/generations = 100000/' repro/setup_tsn2.txt > "$d/setup.txt"
grep -q '^timelimit' "$d/setup.txt" && sed -i 's/^timelimit = .*/timelimit = 60/' "$d/setup.txt" || echo "timelimit = 60" >> "$d/setup.txt"
./FuzzyFW "$d/setup.txt" SelectosYTaillardIntervalos/tai50_20_01.F.15_01.txt "$d" > "$d/log.txt" 2>&1 &
pid=$!
echo "pid=$pid  (tsn2, tai50_20_01, 6 runs x 60s)"
echo "t(s)  RSS(MB)  run_actual"
for i in $(seq 1 28); do
  sleep 15
  kill -0 $pid 2>/dev/null || { echo "proceso terminado"; break; }
  rss=$(awk '/VmRSS/{print int($2/1024)}' /proc/$pid/status 2>/dev/null)
  cur=$(grep -oE 'Run [0-9]+' "$d/log.txt" 2>/dev/null | tail -1)
  printf "%4d  %7s  %s\n" $((i*15)) "$rss" "$cur"
done
wait $pid 2>/dev/null
echo "--- fin ---"
