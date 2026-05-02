/*
 * Selection_CellC13.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_CellC9.h"
#include "Selection_CellL9.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionCellC13
//
//=============================================================================
/**
 * This class implements cellular selection, returns 12 cells
 *
 * @author hdiaz
 *
 */
class SelectionCellC13 : public SelectionCellC9 {
public:
	explicit SelectionCellC13(ParameterDB *parameters = NULL)
		: SelectionCellC9(parameters) { }

	virtual ~SelectionCellC13() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual std::vector<int> getCellsIndex(const unsigned int size, const unsigned int n) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SelectionCellC13");
		return setup;
	}
};

} // namespace FuzzyFW
