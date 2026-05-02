/*
 * Selection_CellC13.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_CellC13.h"

namespace FuzzyFW {

//	Class Selection13Cell
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellC13::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellC13::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coord1 = SelectionCellC9::getCellsIndex(size, n);
	SelectionCellL9 selL9;
	std::vector<int> coord2 = selL9.getCellsIndex(size, n);
	unsigned int coord1Size = coord1.size();
	for (int i = 0; i < coord2.size(); i++) {
		bool repeated = false;
		for (int j = 0; j < coord1Size; j++) {
			if (coord2[i] == coord1[j]) repeated = true;
		}
		if (!repeated) coord1.push_back(coord2[i]);
	}
	std::cout << coord1.size() << std::endl;
	return coord1;
}


//-----  apply  --------------------------------------------------------------
Population * SelectionCellC13::apply(Population *population, const unsigned int n,
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


} // namespace FuzzyFW
