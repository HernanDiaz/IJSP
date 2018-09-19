/*
* Evaluation.cpp
*
*  Created on: July 10, 2017
*      Author: Juan Jose Palacios
*/

#include "Evaluation.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract Class Evaluation
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS
//=============================================================================
//-----  Default constructor  -------------------------------------------------
Evaluation::Evaluation(ParameterDB *parameters)
	: lamarckLabel(FUZZYFW_EVALUATION_LAMARCK), lamarckism(false) {
	if (parameters != NULL)
		this->setup(parameters);
}

//-----  Copy constructor  ----------------------------------------------------
Evaluation::Evaluation(const Evaluation & source)
	: lamarckLabel(source.lamarckLabel), lamarckism(source.lamarckism) { }


//-----  Setup method  --------------------------------------------------------
void Evaluation::setup(ParameterDB *parameters) {
	// Checks is lamarckism is desired
	this->lamarckism = parameters->getBoolean(this->lamarckLabel);
}


	
//=============================================================================
//		METHODS
//=============================================================================
//-----  Evaluate Population  -------------------------------------------------
void Evaluation::evaluatePopulation(const SharedVarsEvolutionary * const svars,
	Population *population, bool reEvaluate) const {
	Fitness *fitness;
	for (unsigned int i = 0; i < population->size(); i++) {
		if (!population->getIndividual(i)->isEvaluated()
			|| reEvaluate) {
			fitness = this->evaluate(svars, population->getIndividual(i));
			population->getIndividual(i)->updateFitness(fitness);
		}
	}
}




}
