/*
 * Selection_Random.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionRandom
//
//=============================================================================
/**
 * This class implements a random selection. It just chooses one individual
 * at random
 *
 *
 */
class SelectionRandom : public Selection {
public:
	explicit SelectionRandom(ParameterDB *parameters=NULL)
		: Selection(parameters) { }

	virtual ~SelectionRandom() { }; 	// Nothing to destroy here

	virtual Individual * select(Population *population,
			const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Random");
		return setup;
	}
};

} // namespace FuzzyFW
