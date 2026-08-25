#!/usr/bin/env bash
# Encadena: espera a PHASE0 DONE -> lanza la extension FTOP.
cd "$(dirname "$0")/.."
{
  echo "auto_phase0_top esperando PHASE0 DONE... $(date)"
  for i in $(seq 1 3000); do
    grep -q 'PHASE0 DONE' final/phase0/run.log 2>/dev/null && break
    pgrep -f 'run_phase0.sh' >/dev/null 2>&1 || break
    sleep 60
  done
  echo "lanzando FTOP $(date)"
  bash final/run_phase0_top.sh
} >> final/phase0_top_auto.log 2>&1
