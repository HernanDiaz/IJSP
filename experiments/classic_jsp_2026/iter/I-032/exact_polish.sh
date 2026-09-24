#!/bin/bash
# Apply the three exact B-12 operators to I-032's best ta23 finals: every run
# at 1570 or below. ta23 has not reached its best known, so here an exact step
# could close the last units.
set -u
cd /opt/scratch
E=/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
D=/tmp/ta23_best
rm -rf $D && mkdir -p $D
python3 - <<'PY'
import glob, os, sys
E = "/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026"
sys.path.insert(0, os.path.join(E, "scripts"))
from verify_certificate import check_run, load_certificate, load_orlib
routes, nj, nm = load_orlib(os.path.join(E, "reference", "taillard_orlib.txt"), "ta23")
n = 0
for path in sorted(glob.glob(os.path.join(E, "results", "I-032_c*", "ta23_*_Certificate.csv"))):
    lines = open(path).read().splitlines()
    for run, rows in sorted(load_certificate(path).items()):
        ms, err = check_run(rows, routes, nj, nm)
        if not err and ms <= 1570:
            keep = [l for l in lines[1:] if l.split(";")[0] == str(run)]
            n += 1
            open("/tmp/ta23_best/ta23_%d_%03d.csv" % (ms, n), "w").write("\n".join([lines[0]] + keep) + "\n")
print("%d ta23 finals at 1570 or below" % n)
PY
for probe in b12_probe.py b12_two_all.py b12_window.py b12_window60.py; do
    echo "=== $probe ==="
    python3 $probe "$D/ta23_*.csv" 2>&1 | tail -4
done
