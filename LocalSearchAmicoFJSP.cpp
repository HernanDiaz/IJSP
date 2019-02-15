/*
* LocalSearchAmicoFJSP.cpp
*
*  Created on: Feb 15, 2019
*      Author: jjpalacios
*/

#include "LocalSearchAmicoFJSP.h"

namespace FJSP {

//=============================================================================
//
//	Class LS_Tabu_Amico_FJSP
//
//=============================================================================
//=============================================================================
//		CONTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void LS_Tabu_Amico_FJSP::setup(FuzzyFW::ParameterDB *parameters) {
	// Loads the maximum number of iterations
	this->Tcycle = parameters->getInteger(this->TcycleLabel, 1);
	
	// Loads the Lambda parameter
	this->Lambda = parameters->getInteger(this->LambdaLabel, Infi);

	// Loads the boundaries for the tabu list sizes
	this->minA = 2;
	this->minB = 0;
	this->maxA = 0;
	this->maxB = 0;

	FuzzyFW::LS_Tabu::setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  Method for updating the tabu list size boundaries  -------------------
void LS_Tabu_Amico_FJSP::updateTabuListBounds(const FuzzyFW::SharedVars *svars) {
	unsigned int rand;
	
	if (this->addend < 0) {
		// Convert the problem type
		ProblemFJSP * fuzzyProb = dynamic_cast<ProblemFJSP *>(svars->problem);
		if (fuzzyProb == NULL) {
			std::string errorMsg = "This local search algorithm works only with ";
			errorMsg += "fuzzy problems.";
			throw new FJSPException("Tabu Search (dellAmico)", errorMsg);
		}
		addend = (fuzzyProb->getNumberJobs() + fuzzyProb->getNumberMachines()) / 3.0;
	}

	this->minA = 2;
	this->minB = this->minA + truncateToInteger(addend);
	rand = svars->rng->getInteger(this->minA, this->minB);
	this->tabuList->setMinSize(rand);

	this->maxA = rand + 6;
	this->maxB = this->maxA + truncateToInteger(addend);
	rand = svars->rng->getInteger(this->maxA, this->maxB);
	this->tabuList->setMaxSize(rand);
}


}
