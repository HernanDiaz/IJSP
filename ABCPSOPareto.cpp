/*
 * ABCPSOPareto.cpp
 *
 *  Created on: Jul 5, 2026
 *      Author: hdiaz
 */

#include "ABCPSOPareto.h"

namespace FuzzyFW {

//-----  prepareToRun: attach the archive to the NSGA-II replacement  ---------
void ABCPSOPareto::prepareToRun(ParameterDB *params) {
	ArtificialBeeColonyPSO::prepareToRun(params);

	// If the NSGA-II replacement is configured (GA/MA-style engines call
	// it every generation), attach the archive there too; the ABC engine
	// itself feeds the archive through applyLocalSearch below.
	ReplacementNSGA2 *nsga2 =
		dynamic_cast<ReplacementNSGA2 *>(this->replacement);
	if (nsga2 != NULL)
		nsga2->setArchive(&this->archive);
}


//-----  applyLocalSearch: feed the archive (ABC engine choke point)  ----------
void ABCPSOPareto::applyLocalSearch(Population *population,
	const unsigned int individualIdx) {

	ArtificialBeeColonyPSO::applyLocalSearch(population, individualIdx);

	Individual *target = population->getIndividual(individualIdx);
	const FitnessLexicographic *fitness =
		dynamic_cast<const FitnessLexicographic *>(target->getFitness());
	if (fitness == NULL || !target->isPhenotypeUpdated())
		return;

	// Offer REAL (unclamped) values to the archive: under a goal cap the
	// individual fitness carries the clamped primary
	IJSP::EvaluationIJSP_MakespanEnergy *ev = dynamic_cast<
		IJSP::EvaluationIJSP_MakespanEnergy *>(this->evaluator);
	IJSP::ScheduleIJSP *sch = dynamic_cast<IJSP::ScheduleIJSP *>(
		target->getPhenotype());
	IJSP::ProblemIJSP *prob = dynamic_cast<IJSP::ProblemIJSP *>(
		this->sharedVariables->problem);
	if (ev != NULL && sch != NULL && prob != NULL) {
		FitnessLexicographic *real = dynamic_cast<FitnessLexicographic *>(
			ev->evaluateSchedule(sch, prob, true));
		this->archive.offer(sch->toString(), real);
		delete real;
	}
	else
		this->archive.offer(target->getPhenotype()->toString(), fitness);
}


//-----  run: clean archive, run, dump the front  ------------------------------
std::pair<Solution *, Objective *> ABCPSOPareto::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	this->archive.clear();

	std::pair<Solution *, Objective *> best =
		ArtificialBeeColonyPSO::run(problem, signature, logFolder, rngSeed);

	if (logFolder.length() > 0)
		this->archive.dump(logFolder + signature + "_Front.csv");

	return best;
}

}
