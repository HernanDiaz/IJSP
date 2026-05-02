/*
 * Selection_CellC9.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_CellL5.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionCellC9
//
//=============================================================================
/**
 * This class implements cellular selection, returns 8 cells, n, s, e, w, nw, sw, ne, se
 *
 * @author hdiaz
 *
 */
class SelectionCellC9 : public SelectionCellL5 {
public:
	explicit SelectionCellC9(ParameterDB *parameters = NULL)
		: SelectionCellL5(parameters) { }

	virtual ~SelectionCellC9() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual std::vector<int> getCellsIndex(const unsigned int size, const unsigned int n) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SelectionCellC9");
		return setup;
	}
};

} // namespace FuzzyFW
