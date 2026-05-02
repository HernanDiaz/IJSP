/*
 * Selection_CellR5.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionCellR5
//
//=============================================================================
/**
 * This class implements cellular selection, returns 4 random cells
 *
 * @author hdiaz
 *
 */
class SelectionCellR5 : public Selection {
public:
	explicit SelectionCellR5(ParameterDB *parameters = NULL)
		: Selection(parameters) { }

	virtual ~SelectionCellR5() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SelectionCellR5");
		return setup;
	}
};

} // namespace FuzzyFW
