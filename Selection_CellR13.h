/*
 * Selection_CellR13.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionCellR13
//
//=============================================================================
class SelectionCellR13 : public Selection {
public:
	explicit SelectionCellR13(ParameterDB *parameters = NULL)
		: Selection(parameters) { }

	virtual ~SelectionCellR13() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("SelectionCellR13");
		return setup;
	}
};

} // namespace FuzzyFW
