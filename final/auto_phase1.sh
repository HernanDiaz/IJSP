#!/usr/bin/env bash
# Encadena: espera PHASE0-TOP DONE -> lanza la calibracion de Fase 1.
cd "$(dirname "$0")/.."
{
  echo "auto_phase1 esperando PHASE0-TOP DONE... $(date)"
  for i in $(seq 1 2000); do
    grep -q 'PHASE0-TOP DONE' final/phase0/run.log 2>/dev/null && break
    pgrep -f 'run_phase0_top.sh' >/dev/null 2>&1 || break
    sleep 60
  done
  echo "lanzando PHASE1 $(date)"
  bash final/run_phase1_calib.sh
} >> final/phase1_auto.log 2>&1
