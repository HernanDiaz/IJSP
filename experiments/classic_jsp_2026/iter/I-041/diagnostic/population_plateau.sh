#!/bin/bash
set -eu
cd /tmp/diag/IJSP
cp /opt/ijsp/crisp/IJSP/ArtificialBeeColonyPSO.cpp ArtificialBeeColonyPSO.cpp
cp /opt/ijsp/crisp/IJSP/ArtificialBeeColonyPSO.h ArtificialBeeColonyPSO.h
python3 - <<'PY'
import io
P = "ArtificialBeeColonyPSO.cpp"
s = io.open(P, encoding="utf-8").read()
s = "#include <cstdio>\n#include <set>\n#include <vector>\n" + s
anchor = "\t\t\tthis->generation++;\n"
assert s.count(anchor) == 1
add = anchor + """\t\t\t{
\t\t\t\tstd::set<std::vector<int> > genos; std::set<long> spans;
\t\t\t\tdouble bestv = 1e18; int atBest = 0;
\t\t\t\tfor (unsigned int k = 0; k < currentPopulation->size(); k++) {
\t\t\t\t\tIndividualArrayInt *a = dynamic_cast<IndividualArrayInt *>(currentPopulation->getIndividual(k));
\t\t\t\t\tstd::vector<int> g(a->size()); for (unsigned int q = 0; q < a->size(); q++) g[q] = a->getGene(q);
\t\t\t\t\tgenos.insert(g);
\t\t\t\t\tdouble f = a->getFitness()->toDouble(); spans.insert((long)f);
\t\t\t\t\tif (f < bestv) { bestv = f; atBest = 0; } if (f == bestv) atBest++;
\t\t\t\t}
\t\t\t\tstatic FILE *o = NULL; if (!o) o = std::fopen("dups.tsv", "w");
\t\t\t\tstd::fprintf(o, "%d\t%lu\t%lu\t%lu\t%.0f\t%d%c", this->generation, (unsigned long)currentPopulation->size(), (unsigned long)genos.size(), (unsigned long)spans.size(), bestv, atBest, 10);
\t\t\t\tstd::fflush(o);
\t\t\t}
"""
s = s.replace(anchor, add)
io.open(P, "w", encoding="utf-8").write(s)
print("instrumented")
PY
make -j14 2>&1 | grep -E ' error|Error [0-9]' | head || true
E=/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
for inst in ta23 ta45; do
  case $inst in ta23) t=40 ;; *) t=150 ;; esac
  D=/tmp/diag/dup_$inst; rm -rf $D; mkdir -p $D
  sed -e "s/^runs = .*/runs = 1/" -e "s/^seed = .*/seed = 1001/" -e "s/^timelimit = .*/timelimit = $t/" $E/setup/ref_I-021.txt > $D/cfg.txt
  (cd $D && ../FuzzyFW cfg.txt /opt/ijsp/crisp/IJSP/TaillardJSP/$inst.txt out > /dev/null 2>&1) &
done
wait
for inst in ta23 ta45; do echo "== $inst: gen popsize unique_genotypes unique_makespans best at_best"; awk 'NR%15==1' /tmp/diag/dup_$inst/dups.tsv; tail -1 /tmp/diag/dup_$inst/dups.tsv; done
