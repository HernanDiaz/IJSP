/*
 * Selection_Shuffle.h
 *
 *  Created on: Aug 2, 2017
 */
#pragma once

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class SelectionShuffle
//
//=============================================================================
/**
 * This class implements the shuffle selection. In this method all the
 * individuals from the original population are selected, but changing their
 * order so they are randomly sorted in the following population
 *
 *
 */
class SelectionShuffle : public Selection {
	//=========================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=========================================================================
public:
	explicit SelectionShuffle(ParameterDB *parameters=NULL)
		: Selection(parameters) { }

	virtual void setup(ParameterDB *parameters) {
		Selection::setup(parameters);
	}

	virtual ~SelectionShuffle() { }; 	// Nothing to destroy here



	//=========================================================================
	//		METHODS
	//=========================================================================
public:
	virtual Individual * select(Population *population,
			const SharedVars *svars) const;

	virtual Population * apply(Population *population, const unsigned int n,
			const SharedVars *svars) const;

	virtual std::vector<std::string> getName() const {
		std::vector<std::string> setup;
		setup.push_back("Shuffle");
		return setup;
	}
};

} // namespace FuzzyFW
