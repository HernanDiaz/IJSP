/*
 * Selection_CellL9.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_CellL9.h"

namespace FuzzyFW {

//	Class SelectionCellL9
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellL9::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellL9::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coords = SelectionCellL5::getCellsIndex(size, n);
	int square = sqrt(size);
	/*
	//East +1
	if (n > 1) {
		coords.push_back(n - 2);
	}
	else coords.push_back(size - 2 - n);
	//West +1 
	if (n < size - 2) {
		coords.push_back(n + 2);
	}
	else coords.push_back(n - size + 2);
	*/

	//East +1
	 //E
	if (n == 0 || n == 1 || n % square == 0 || (n - 1) % square == 0) {
		coords.push_back(n + square - 2);
	}
	else coords.push_back(n - 2);

	//West +1 
	if ((n + 1) % square == 0 || n == size - 1 || n == size - 2 || (n + 2) % square == 0) {
		coords.push_back(n - square + 2);
	}
	else coords.push_back(n + 2);
	   	 
	//North +1
	
    if (n < 2 * square) {
		coords.push_back(size - square - square + n);
	}
	else coords.push_back(n - square - square);
	//South +1 
	if (n < size - square - square) {
		coords.push_back(n + square + square);
	}
	else coords.push_back(square + square - size + n);
	return coords;
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellL9::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population* grid = new Population();
	std::vector<int> coords = getCellsIndex(population->size(), n);
	for (int i = 0; i < coords.size(); i++) {
		if (coords[i] >= 0 && coords[i] < population->size()) {
			grid->addIndividual(population->getIndividual(coords[i])->clone());
		}
	}
	return grid;
}



//=============================================================================
//

} // namespace FuzzyFW
