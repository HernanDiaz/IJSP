/*
* Statistics.cpp
*
*  Created on: June 25, 2019
*      Author: hdiaz
*/

#include "StatisticsIJSP.h"

namespace IJSP {

//=============================================================================
//
//	Class StatisticsMakespan
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsMakespan::StatisticsMakespan(STAT_TYPE flag)
	: Statistics(flag) {
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(IJSP_EVALUATION_COMPARE, "COMPONENT");
	newParams->setParameter(IJSP_EVALUATION_MAXIMUM, "COMPONENT");
	newParams->setParameter(FUZZYFW_EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationIJSP_Makespan(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsMakespan::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	FuzzyFW::Objective *objFunction, *best, *worst;
	double sum, sumSq, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		objFunction =
			this->evaluation->getObjectiveFunction(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = objFunction;
			sum = objFunction->toDouble();
			sumSq = objFunction->toDouble() * objFunction->toDouble();
		}
		else {
			sum += objFunction->toDouble();
			if (objFunction->isBetterThan(best)) {
				if (best != worst)
					delete best;
				best = objFunction;
			}
			else if (objFunction->isWorseThan(worst)) {
				if (best != worst)
					delete worst;
				worst = objFunction;
			}
			else
				delete objFunction;
		}
	}
	if (this->type == STAT_BEST)
		solution = best->toDouble();
	else if (this->type == STAT_WORST)
		solution = worst->toDouble();
	else if (this->type == STAT_AVG)
		solution = sum / population->size();
	if (best != worst)
		delete best;
	delete worst;

	return solution;
}

}
