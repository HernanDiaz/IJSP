/*
 * Selection_CellR13.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_CellR13.h"

namespace FuzzyFW {

//	Class SelectionCellR13
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellR13::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellR13::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	std::vector<int> coords;
	coords.push_back(n);
	Population* grid = new Population();
	for (int i = 0; i < 13; i++) {
		int index = svars->rng->getInteger(0, population->size() - 1);
		while (std::find(coords.begin(), coords.end(), index) != coords.end()) {
			index = svars->rng->getInteger(0, population->size() - 1);
		}
		coords.push_back(index);
		grid->addIndividual(population->getIndividual(index)->clone());
	}
	return grid;
}

//=============================================================================
//

} // namespace FuzzyFW
