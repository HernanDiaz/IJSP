/*
 * Selection_Elite.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionElite
//
//=============================================================================
/**
 * This class implements an elite selection. It just chooses one individual
 * at random between the best N individuals in the population
 *
 * @author hdiaz
 *
 */
class SelectionElite : public Selection {
public:
	explicit SelectionElite(ParameterDB *parameters = NULL)
		: Selection(parameters) { }

	virtual ~SelectionElite() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
		const SharedVars *svars) const;

	virtual Population * apply(Population *population, const unsigned int n,
		const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Elite");
		return setup;
	}
};

} // namespace FuzzyFW
