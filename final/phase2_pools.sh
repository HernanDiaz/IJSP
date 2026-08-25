#!/usr/bin/env bash
# Fase 2 — pools: (1) re-evalua los 244 pools crudos a la convencion componentwise
# (permutaciones intactas) -> pools_test/corrected/int__<inst>_<gen>_repo_pool.csv;
# (2) construye los pools mix (84 v2 + 83 gtmwkr + 83 gp por run, 30 runs = 7500
# lineas) para las 61 instancias. Idempotente. Registra la auditoria en un log.
cd "$(dirname "$0")/.."
mkdir -p pools_test/corrected
LOG=final/phase2_pools_audit.log
: > "$LOG"

instfile() { # nombre de instancia -> fichero
  if [ "$1" = "ft10" ]; then echo "SelectosYTaillardIntervalos/F0.15.0.ft10_10.txt";
  else echo "SelectosYTaillardIntervalos/$1.F.15_01.txt"; fi
}

INSTS=""
for cls in tai15_15 tai20_20 tai30_15 tai30_20 tai50_15 tai50_20; do
  for i in $(seq -w 1 10); do INSTS="$INSTS ${cls}_${i}"; done
done
INSTS="$INSTS ft10"

n=0; done_r=0
for inst in $INSTS; do
  f=$(instfile "$inst")
  [ -f "$f" ] || { echo "FALTA instancia $f" | tee -a "$LOG"; continue; }
  for g in v2 graspmor gtmwkr gp; do
    n=$((n+1))
    p="pools_test/int__${inst}_${g}_pool.csv"
    o="pools_test/corrected/int__${inst}_${g}_repo_pool.csv"
    [ -f "$o" ] && { done_r=$((done_r+1)); continue; }
    [ -f "$p" ] || { echo "FALTA pool $p" | tee -a "$LOG"; continue; }
    ./seed_consistency_test "$f" "$p" --rewrite "$o" 2>/dev/null | grep -E 'lineas=|coincide' >> "$LOG"
    echo "== $inst $g ==" >> "$LOG"
    done_r=$((done_r+1))
  done
done
echo "rewrite: $done_r/$n pools corregidos" | tee -a "$LOG"

# --- mix: 84 v2 + 83 gtmwkr + 83 gp por run (k=250, p*=100%) ---
mixok=0
for inst in $INSTS; do
  out="pools_test/corrected/int__${inst}_mix_repo_pool.csv"
  [ -f "$out" ] && { mixok=$((mixok+1)); continue; }
  ok=1
  for g in v2 gtmwkr gp; do [ -f "pools_test/corrected/int__${inst}_${g}_repo_pool.csv" ] || ok=0; done
  [ "$ok" = "1" ] || { echo "mix SKIP $inst (falta base)" | tee -a "$LOG"; continue; }
  : > "$out"
  for r in $(seq 0 29); do
    for spec in "v2 84" "gtmwkr 83" "gp 83"; do
      set -- $spec; g=$1; k=$2
      awk -v r=$r -v k=$k '{a[NR]=$0} END{L=NR; s=(r*k)%L; for(i=0;i<k;i++){print a[((s+i)%L)+1]}}' \
        "pools_test/corrected/int__${inst}_${g}_repo_pool.csv" >> "$out"
    done
  done
  mixok=$((mixok+1))
done
echo "mix: $mixok/61 construidos" | tee -a "$LOG"
echo "PHASE2 POOLS DONE: $(date)" | tee -a "$LOG"
