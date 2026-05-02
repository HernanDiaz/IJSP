/*
 * Selection_Random.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_Random.h"

namespace FuzzyFW {

//	Class SelectionRandom
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionRandom::select(Population *population,
		const SharedVars *svars) const {

	int randomValue = svars->rng->getInteger(0, population->size()-1);
	return population->getIndividual(randomValue);
}


//=============================================================================
//

} // namespace FuzzyFW
