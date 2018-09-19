/*
* EvolutionaryAlgorithm.cpp
*
*  Created on: Sep 15, 2017
*      Author: jjpalacios
*/

#include "EvolutiveAlgorithm.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class EvolutionaryAlgorithm
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
EvolutiveAlgorithm::EvolutiveAlgorithm(ParameterDB *params) {
	EvaluationClassRegister::registerClasses();
	StatisticsClassRegister::registerClasses();

	this->sharedVariables = new SharedVarsEvolutionary();
	this->sharedVariables->parameters = params;
#if defined(_WIN32)
	this->sharedVariables->rng = new RandomMT();
#else
	this->sharedVariables->rng = new Random();
#endif
	finished = false;
	evaluator = NULL;
	ready = false;
}


//-----  ClearAll  ------------------------------------------------------------
void EvolutiveAlgorithm::clearAll() {
	delete this->evaluator;
	this->finished = false;
	this->ready = false;
}



//=========================================================================
//		METHODS
//=========================================================================
//-----  prepareToRun  ----------------------------------------------------
void EvolutiveAlgorithm::prepareToRun(ParameterDB *params) {
	std::string paramName;
	std::string paramValue;

	if (this->ready)
		clearAll();

	if (params != NULL) {
		if (this->sharedVariables->parameters != params)
			delete this->sharedVariables->parameters;
		this->sharedVariables->parameters = params;
	}

	// Loads the evaluation function to use
	paramName = OBJECTIVE_FUNCTION;
	std::string evalName = this->sharedVariables->parameters
		->getString(paramName);

	if (paramName.length() < 1) {
		std::string errorMsg = "Objective function not found";
		throw new FuzzyFWException("Loading", errorMsg);
	}

	this->evaluator = EvaluationClassRegister::getEvaluationObject(evalName);

	if (this->evaluator == NULL) {
		std::string errorMsg = "Invalid evaluation function. Incorrect name ";
		errorMsg += "or missing parameter";
		throw new FuzzyFWException("Loading", errorMsg);
	}
	this->evaluator->setup(this->sharedVariables->parameters);

	// Load the different statistics to use
	Statistics * stats;
	statistics.clear();
	int cont = 1;
	Statistics::STAT_TYPE type;

	paramName = valueToString(STATISTICS_ROOT);
	paramName += ".1." + valueToString(STATISTICS_VALUE);
	paramValue = this->sharedVariables->parameters->getString(paramName);

	while (paramValue.size() > 1) {
		stats = StatisticsClassRegister::getStatsObject(paramValue);

		paramName = valueToString(STATISTICS_ROOT) + ".";
		paramName += valueToString(cont) + ".";
		paramName += valueToString(STATISTICS_METRIC);
		paramValue = this->sharedVariables->parameters->getString(paramName);
		
		if(paramValue.compare(STATISTICS_BEST) == 0)
			type = Statistics::STAT_BEST;
		else if (paramValue.compare(STATISTICS_WORST) == 0)
			type = Statistics::STAT_WORST;
		else if (paramValue.compare(STATISTICS_AVG) == 0)
			type = Statistics::STAT_AVG;
		else if (paramValue.compare(STATISTICS_SDEV) == 0)
			type = Statistics::STAT_SDEV;
		else 
			type = Statistics::STAT_BEST;

		stats->setStat(type);
		statistics.push_back(stats);

		cont++;

		paramName = valueToString(STATISTICS_ROOT) + ".";
		paramName += valueToString(cont) + ".";
		paramName += valueToString(STATISTICS_VALUE);
		paramValue = this->sharedVariables->parameters->getString(paramName);
	}

	this->ready = true;
	this->finished = false;
}

}

