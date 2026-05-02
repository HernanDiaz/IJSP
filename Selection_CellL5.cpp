/*
 * Selection_CellL5.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_CellL5.h"

namespace FuzzyFW {

//	Class SelectionCellL5
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellL5::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellL5::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coords(4);
	int square = sqrt(size);
	//coords[0] = n;
	//E
	/*
	if (n > 0) {
		coords[0] = n - 1;
	}
	else coords[0] = size - 1;
	//W
	if (n < size - 1) {
		coords[1] = n + 1;
	}
	else coords[1] = 0;
	*/
	//E
	if (n == 0 || n % square == 0) {
		coords[0] = n + square - 1;
	}
	else coords[0] = n - 1;

	//W
	if ((n + 1) % square == 0 || n == size - 1) {
		coords[1] = n - square + 1;
	}
	else coords[1] = n + 1;
	
	//N
	
	if (n < square) {
		coords[2] = size - square + n;
	}
	else coords[2] = n - square;
	//S
	if (n < size - square) {
		coords[3] = n + square;
	}
	else coords[3] = square - size + n;
	return coords;
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellL5::apply(Population *population, const unsigned int n,
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
