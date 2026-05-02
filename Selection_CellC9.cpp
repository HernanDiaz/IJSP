/*
 * Selection_CellC9.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_CellC9.h"

namespace FuzzyFW {

//	Class Selection9Cell
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellC9::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellC9::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coords = SelectionCellL5::getCellsIndex(size, n);
	int square = sqrt(size);

	//NorthW
	if (n < square - 1) {
		coords.push_back(size - square + n + 1);
	}
	else if (n == square - 1) {
		coords.push_back(size - square);
	}
	else if ((n + 1) % square == 0 || n == size - 1) {
		coords.push_back(n - square - square + 1);
	}
	else coords.push_back(n - square + 1);
	//SouthW

	if (n == size - 1) {
		coords.push_back(0);
	}
	else if (n > size - square - 1) {
		coords.push_back(n - size + square + 1);
	}
	else if ((n + 1) % square == 0) {
		coords.push_back(n + 1);
	}
	else coords.push_back(n + square + 1);

	//NorthE

	if (n == 0) {
		coords.push_back(size - 1);
	}
	else if (n < square) {
		coords.push_back(size - (square - n) - 1);
	}
	else if (n%square == 0) {
		coords.push_back(n - 1);
	}
	else coords.push_back(n - square - 1);

	//SoutheE
	if (n == 0 || n % square == 0) {
		coords.push_back(n + square + square - 1);
	}
	else if (n > size - square) {
		coords.push_back(n + square - size - 1);
	}
	else coords.push_back(n + square - 1);

	/*
	//NorthW
	if (n < square - 1) {
		coords.push_back(size - square + n + 1);
	}
	else if (n == square - 1) {
		coords.push_back(0);
	}
	else {
		coords.push_back(n - square + 1);
	}
	//SouthW
	if (n < size - square - 1) {
		coords.push_back(n + square + 1);
	}
	else if (n < size - square) {
		coords.push_back(0);
	}
	else {
		coords.push_back(square - size + n + 1);
	}
	//NorthE
	if (n <= square) {
		coords.push_back(size - (square - n) - 1);
	}
	else coords.push_back(n - square - 1);
	//SoutheE
	if (n <= size - square) {
		coords.push_back(n + square - 1);
	}
	else coords.push_back(square - size + n - 1);
	*/
	return coords;
}


//-----  apply  --------------------------------------------------------------
Population * SelectionCellC9::apply(Population *population, const unsigned int n,
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
