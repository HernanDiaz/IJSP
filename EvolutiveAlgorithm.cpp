/*
* EvolutionaryAlgorithm.cpp
*
*  Created on: Sep 15, 2017
*      Author: jjpalacios
*/

#include "EvolutiveAlgorithm.h"

namespace FJSP {

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

	this->sharedVariables = new SharedVars();
	this->sharedVariables->parameters = params;
	this->sharedVariables->rng = new RandomMT();
	finished = false;
	evaluator = NULL;
	ready = false;
}


//-----  ClearAll  ------------------------------------------------------------
void EvolutiveAlgorithm::clearAll() {
	delete this->evaluator;
}



//=========================================================================
//		METHODS
//=========================================================================
//-----  prepareToRun  ----------------------------------------------------
void EvolutiveAlgorithm::prepareToRun(ParameterDB *params) {
	std::string paramName;

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
		throw new FJSPException("Loading", errorMsg);
	}

	this->evaluator = EvaluationClassRegister::getEvaluationObject(evalName);

	if (this->evaluator == NULL) {
		std::string errorMsg = "Invalid evaluation function. Incorrect name ";
		errorMsg += "or missing parameter";
		throw new FJSPException("Loading", errorMsg);
	}
	this->evaluator->setup(this->sharedVariables->parameters);

	this->ready = true;
	this->finished = false;
}

}

