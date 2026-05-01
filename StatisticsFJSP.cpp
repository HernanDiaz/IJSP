/*
* Statistics.cpp
*
*  Created on: Oct 4, 2017
*/

#include "StatisticsFJSP.h"

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
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(FJSP_EVALUATION_COMPARE, "EV");
	newParams->setParameter(FJSP_EVALUATION_MAXIMUM, "COMPONENT");
	newParams->setParameter(FUZZYFW_EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_Makespan(newParams);
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
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(FJSP_EVALUATION_AI, FJSP_EVAL_AI_EXACT);
	newParams->setParameter(FUZZYFW_EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_AIavg(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsAIavg::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	FuzzyFW::Objective *objFunction, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		objFunction =
			this->evaluation->getObjectiveFunction(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = objFunction;
			sum = objFunction->toDouble();
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
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(FJSP_EVALUATION_AI, FJSP_EVAL_AI_EXACT);
	newParams->setParameter(FUZZYFW_EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_AImin(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsAImin::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	FuzzyFW::Objective *objFunction, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		objFunction =
			this->evaluation->getObjectiveFunction(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = objFunction;
			sum = objFunction->toDouble();
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





//=============================================================================
//
//	Class StatisticsESDavg
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsESDavg::StatisticsESDavg(STAT_TYPE flag)
	: Statistics(flag) {
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(FUZZYFW_EVALUATION_LAMARCK, "NO");
	this->evaluation = new EvaluationFJSP_ESDavg(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsESDavg::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	FuzzyFW::Objective *objFunction, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		objFunction =
			this->evaluation->getObjectiveFunction(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = objFunction;
			sum = objFunction->toDouble();
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





//=============================================================================
//
//	Class StatisticsESDmin
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
StatisticsESDmin::StatisticsESDmin(STAT_TYPE flag)
	: Statistics(flag) {
	FuzzyFW::ParameterDB *newParams = new FuzzyFW::ParameterDB();
	newParams->setParameter(FJSP_EVALUATION_AI, FJSP_EVAL_AI_EXACT);
	this->evaluation = new EvaluationFJSP_ESDmin(newParams);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Get value method  ----------------------------------------------------
double StatisticsESDmin::getValue(const FuzzyFW::SharedVarsEvolutionary * svars,
	FuzzyFW::Population *population) {

	FuzzyFW::Objective *objFunction, *best, *worst;
	double sum, solution;
	best = worst = NULL;

	for (unsigned int ind = 0; ind < population->size(); ind++) {
		objFunction =
			this->evaluation->evaluate(svars, population->getIndividual(ind));
		if (ind == 0) {
			best = worst = objFunction;
			sum = objFunction->toDouble();
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
