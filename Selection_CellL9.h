/*
 * Selection_CellL9.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_CellL5.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionCellL9
//
//=============================================================================
class SelectionCellL9 : public SelectionCellL5 {
public:
	explicit SelectionCellL9(ParameterDB *parameters = NULL)
		: SelectionCellL5(parameters) { }

	virtual ~SelectionCellL9() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual std::vector<int> getCellsIndex(const unsigned int size, const unsigned int n) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SelectionCellL9");
		return setup;
	}
};

} // namespace FuzzyFW
