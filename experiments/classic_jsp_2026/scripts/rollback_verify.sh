#!/bin/bash
# The rolled-back solver must behave exactly as the one before the rollback
# on every configuration still in use. Runs are made deterministic by a fixed
# number of generations and a time limit that never binds; the final
# makespans (recomputed from the certificates) and the per-generation best of
# every run must be identical.
set -u
cd /opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
D=/tmp/rollback_verify
rm -rf $D && mkdir -p $D
declare -A BIN=( [before]=/opt/scratch/FuzzyFW_before_rollback [after]=/opt/ijsp/crisp/FuzzyFW )
for cfg in prereg2_abc_300s ref_I-018 ref_I-021; do
  for inst in ta23 ta45; do
    sed -e "s/^runs = .*/runs = 2/" -e "s/^seed = .*/seed = 7001/" \
        -e "s/^timelimit = .*/timelimit = 100000/" setup/$cfg.txt > $D/${cfg}_${inst}.txt
    printf '\ngenerations = 12\n' >> $D/${cfg}_${inst}.txt
    for b in before after; do
      ${BIN[$b]} $D/${cfg}_${inst}.txt /opt/ijsp/crisp/IJSP/TaillardJSP/$inst.txt \
          $D/${b}_${cfg}_${inst} > $D/${b}_${cfg}_${inst}.out 2>&1
    done
  done
done
python3 - <<'PY'
import glob, os, sys
E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(E, "scripts"))
from compare import makespans
ORLIB = os.path.join(E, "reference", "taillard_orlib.txt")
D = "/tmp/rollback_verify"

def trace(d, inst):
    f = [p for p in glob.glob(os.path.join(d, inst + "_*.csv"))
         if not p.endswith(("_Certificate.csv", "_Sols.csv"))][0]
    rows, seen = [], False
    for line in open(f):
        if line.startswith("Step;Runtime;"):
            seen = True
            continue
        if seen:
            x = line.strip().split(";")
            if len(x) > 8:
                # per-run best Cmax columns, runtime excluded
                rows.append(tuple(x[8 * k + 4] for k in range(1, (len(x) // 8))))
    return rows

ok = True
for cfg in ("prereg2_abc_300s", "ref_I-018", "ref_I-021"):
    for inst in ("ta23", "ta45"):
        a = makespans(os.path.join(D, "before_%s_%s" % (cfg, inst)), ORLIB, inst)
        b = makespans(os.path.join(D, "after_%s_%s" % (cfg, inst)), ORLIB, inst)
        ta, tb = trace(os.path.join(D, "before_%s_%s" % (cfg, inst)), inst), \
                 trace(os.path.join(D, "after_%s_%s" % (cfg, inst)), inst)
        same = a == b and ta == tb and len(ta) > 0
        ok &= same
        print("  %-18s %-5s before %-14s after %-14s trace %3d gens  %s"
              % (cfg, inst, a, b, len(ta), "IDENTICAL" if same else "DIFFERENT"))
print("\nALL IDENTICAL" if ok else "\nMISMATCH: the rollback changed behaviour")
PY
grep -l -i 'error\|exception' $D/*.out | head -3
