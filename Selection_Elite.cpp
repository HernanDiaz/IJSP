/*
 * Selection_Elite.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_Elite.h"

namespace FuzzyFW {

//	Class EliteSelection
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionElite::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}



//-----  apply  --------------------------------------------------------------
Population * SelectionElite::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	
	while (newPopulation->size() < n) {
		for (unsigned int i = 0;  newPopulation->size() < n; i++) {
			selected = select(population, svars);
			newPopulation->addIndividual(selected->clone());
		}
	}

	return newPopulation;
}

//=============================================================================
//

} // namespace FuzzyFW
