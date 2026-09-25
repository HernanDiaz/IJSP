#!/bin/bash
set -eu
cd /tmp/diag/IJSP
cp /opt/ijsp/crisp/IJSP/ArtificialBeeColonyPSO.cpp ArtificialBeeColonyPSO.cpp
cp /opt/ijsp/crisp/IJSP/ArtificialBeeColonyPSO.h ArtificialBeeColonyPSO.h
python3 - <<'PY'
import io
P = "ArtificialBeeColonyPSO.cpp"
s = io.open(P, encoding="utf-8").read()
def sub(s, old, new):
    assert s.count(old) == 1, old[:50]
    return s.replace(old, new)
s = "#include <cstdio>\nstatic double g_afterFirst = 0; static unsigned long g_calls = 0; static FILE *g_out = NULL;\n" + s
# after the first call on the best child in MALS_SOME
s = sub(s, """			best = chosen = population->whoIsBest(this->sharedVariables);
			this->applyLocalSearch(population, chosen);
""", """			best = chosen = population->whoIsBest(this->sharedVariables);
			this->applyLocalSearch(population, chosen);
			g_afterFirst = population->getIndividual(chosen)->getFitness()->toDouble();
""")
s = sub(s, """		optimised = this->localSearch->apply(""", """		g_calls++;
		optimised = this->localSearch->apply(""")
s = sub(s, """				timePoint = clock();

				if (this->lsFrequency == LS_Frequency::MALS_PERIOD""", """				timePoint = clock();
				double rawBest = currentFoodSources.getBest(this->sharedVariables)->getFitness()->toDouble();
				unsigned long callsBefore = g_calls;

				if (this->lsFrequency == LS_Frequency::MALS_PERIOD""")
s = sub(s, "\t\t\t\tIndividual* bestLocal = currentFoodSources.getBest(this->sharedVariables);\n",
"""\t\t\t\tIndividual* bestLocal = currentFoodSources.getBest(this->sharedVariables);
\t\t\t\tif (g_out == NULL) g_out = std::fopen("chains.tsv", "w");
\t\t\t\tstd::fprintf(g_out, "%.0f\t%.0f\t%.0f\t%.0f\t%.0f\t%lu%c", currentFoodSource->getFitness()->toDouble(),
\t\t\t\t\tthis->bestSoFar->getFitness()->toDouble(), rawBest, g_afterFirst, bestLocal->getFitness()->toDouble(), g_calls - callsBefore, 10);
""")
io.open(P, "w", encoding="utf-8").write(s)
print("instrumented")
PY
grep -n 'fprintf(g_out' -A2 ArtificialBeeColonyPSO.cpp | head -3
make -j14 2>&1 | grep -E ' error|Error [0-9]' | head || true
E=/opt/ijsp/crisp/IJSP/experiments/classic_jsp_2026
for inst in ta23 ta45; do
  case $inst in ta23) t=40 ;; *) t=150 ;; esac
  D=/tmp/diag/run_$inst; rm -rf $D; mkdir -p $D
  sed -e "s/^runs = .*/runs = 1/" -e "s/^seed = .*/seed = 1001/" -e "s/^timelimit = .*/timelimit = $t/" $E/setup/ref_I-021.txt > $D/cfg.txt
  (cd $D && ../FuzzyFW cfg.txt /opt/ijsp/crisp/IJSP/TaillardJSP/$inst.txt out > /dev/null 2>&1) &
done
wait
wc -l /tmp/diag/run_*/chains.tsv
