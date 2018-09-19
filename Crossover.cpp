/*
 * Crossover.cpp
 *
 *  Created on: Jul 14, 2017
 *      Author: jjpalacios
 */

#include "Crossover.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Crossover
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
Crossover::Crossover(ParameterDB *parameters)
: probLabel(CROSSOVER_PROBABILITY), probability(0.0) {
	if(parameters != NULL)
		this->setup(parameters);
}

//-----  Setup method  --------------------------------------------------------
void Crossover::setup(ParameterDB *parameters) {
	this->probability = parameters->getDouble(this->probLabel, -1.0);
	if(compareDouble(this->probability, 0.0) < 0
		|| compareDouble(this->probability, 1.0) > 0) {
		std::string err = "Invalid crossover probability.";
		err += " Incorrect value or missing parameter.";
		throw new FuzzyFWException("Crossover",err);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Population)  --------------------------------------------------
void Crossover::apply(Population *population,
	const SharedVarsEvolutionary *svars) const {
	for(unsigned int i=0; i < population->size(); i+=2) {
		// If there are two parents, generate two offspring
		if((i+1) < population->size()) {
			if (svars->rng->getProbability() < this->probability) {
				// Cross the individuals to generate two offspring
				this->apply(population->getIndividual(i),
					population->getIndividual(i + 1), svars);
			}
		}

		// If there is only one parent, keep it
	}
}

}
