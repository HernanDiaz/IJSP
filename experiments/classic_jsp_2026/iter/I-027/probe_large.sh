#!/bin/bash
set -u
cd /opt/scratch
E=/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
for cls in "30x15:ta32 ta33 ta34 ta40" "30x20:ta41 ta42 ta43 ta44 ta45 ta46 ta47 ta48 ta49 ta50"; do
    name=${cls%%:*}; insts=${cls#*:}
    globs=""
    for i in $insts; do globs="$globs $E/results/I-030_w*_s*_current/${i}_*_Certificate.csv"; done
    echo "=== $name, I-030 current finals at 300 s ==="
    python3 b12_probe.py $globs > /tmp/b12_$name.txt 2>&1
    tail -1 /tmp/b12_$name.txt
    grep ' -> ' /tmp/b12_$name.txt | awk '{split($0,a,": "); split(a[2],b," "); d=b[1]-b[3]; s+=d; n++} END {if (n) printf "  improved %d, mean gain %.1f units\n", n, s/n}'
done
