/*
 * Selection_CellL5.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionCellL5
//
//=============================================================================
/**
 * This class implements cellular selection, returns 4 cells, n, s, e, w
 *
 * @author hdiaz
 *
 */
class SelectionCellL5 : public Selection {
public:
	explicit SelectionCellL5(ParameterDB *parameters = NULL)
		: Selection(parameters) { }

	virtual ~SelectionCellL5() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual std::vector<int> getCellsIndex(const unsigned int size, const unsigned int n) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SelectionCellL5");
		return setup;
	}
};

} // namespace FuzzyFW
