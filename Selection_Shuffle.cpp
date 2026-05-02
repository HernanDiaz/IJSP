/*
 * Selection_Shuffle.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_Shuffle.h"

namespace FuzzyFW {

//	Class ShuffleSelection
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionShuffle::select(Population *population,
		const SharedVars *svars) const {
	int randomValue = svars->rng->getInteger(0, population->size()-1);
	return population->getIndividual(randomValue);
}



//-----  apply  --------------------------------------------------------------
Population * SelectionShuffle::apply(Population *population, const unsigned int n,
		const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	std::vector<int> randomOrder;
	
	for (unsigned int i = 0; i < population->size(); i++) {
		randomOrder.push_back(i);
	} 
		
	while (newPopulation->size() < n) {
		svars->rng->shuffle(randomOrder);
		for (unsigned int i = 0; i < randomOrder.size() && newPopulation->size() < n; i++) {
			selected = population->getIndividual(randomOrder[i]);
			newPopulation->addIndividual(selected->clone());
		}
	}

	return newPopulation;
}





//=============================================================================
//

} // namespace FuzzyFW
