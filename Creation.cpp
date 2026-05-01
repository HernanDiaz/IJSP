/*
* Creation.cpp
*
*  Created on: Aug 2, 2017
*/

#include "Creation.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Creation
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  createPopulation  ----------------------------------------------------
Population * Creation::createPopulation(const unsigned int popSize,
	const SharedVarsEvolutionary *svars) const {
	Population * population = new Population();
	Individual *indiv;

	for (unsigned int i = 0; i < popSize; i++) {
		indiv = this->createIndividual(svars);
		indiv->id = i;
		population->addIndividual(indiv);
	}
	return population;
}

}
