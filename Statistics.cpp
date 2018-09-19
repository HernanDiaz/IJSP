/*
* Statistics.cpp
*
*  Created on: Oct 4, 2017
*      Author: jjpalacios
*/

#include "Statistics.h"

namespace FJSP {

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
	ParameterDB *newParams = new ParameterDB();
	newParams->setParameter(EVALUATION_COMPARE, "EV");
	newParams->setParameter(EVALUATION_MAXIMUM, "COMPONENT");
	newParams->setParameter(EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_Makespan(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsMakespan::getValue(const SharedVars * svars,
	Population *population) {

	Fitness *fitness, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		fitness = 
			this->evaluation->evaluate(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = fitness;
			sum = fitness->toDouble();
		}
		else {
			sum += fitness->toDouble();
			if (fitness->isBetterThan(best)) {
				if (best != worst)
					delete best;
				best = fitness;
			}
			else if (fitness->isWorseThan(worst)) {
				if (best != worst)
					delete worst;
				worst = fitness;
			}
			else
				delete fitness;
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





//=============================================================================
//
//	Class StatisticsAIavg
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsAIavg::StatisticsAIavg(STAT_TYPE flag)
	: Statistics(flag) {
	ParameterDB *newParams = new ParameterDB();
	newParams->setParameter(EVALUATION_AI, EVAL_AI_EXACT);
	newParams->setParameter(EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_AIavg(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsAIavg::getValue(const SharedVars * svars,
	Population *population) {

	Fitness *fitness, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		fitness =
			this->evaluation->evaluate(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = fitness;
			sum = fitness->toDouble();
		}
		else {
			sum += fitness->toDouble();
			if (fitness->isBetterThan(best)) {
				if (best != worst)
					delete best;
				best = fitness;
			}
			else if (fitness->isWorseThan(worst)) {
				if (best != worst)
					delete worst;
				worst = fitness;
			}
			else
				delete fitness;
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





//=============================================================================
//
//	Class StatisticsAImin
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsAImin::StatisticsAImin(STAT_TYPE flag)
	: Statistics(flag) {
	ParameterDB *newParams = new ParameterDB();
	newParams->setParameter(EVALUATION_AI, EVAL_AI_EXACT);
	newParams->setParameter(EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_AImin(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsAImin::getValue(const SharedVars * svars,
	Population *population) {

	Fitness *fitness, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		fitness =
			this->evaluation->evaluate(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = fitness;
			sum = fitness->toDouble();
		}
		else {
			sum += fitness->toDouble();
			if (fitness->isBetterThan(best)) {
				if (best != worst)
					delete best;
				best = fitness;
			}
			else if (fitness->isWorseThan(worst)) {
				if (best != worst)
					delete worst;
				worst = fitness;
			}
			else
				delete fitness;
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
