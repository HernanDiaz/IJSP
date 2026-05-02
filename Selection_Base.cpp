/*
 * Selection_Base.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_Base.h"

namespace FuzzyFW {

//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
Selection::Selection(ParameterDB * parameters) {
	if(parameters != NULL)
		this->setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Population)  --------------------------------------------------
Population * Selection::apply(Population *population, const unsigned int n,
		const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	for(unsigned int i=0; i < n; i++) {
		selected = this->select(population, svars);
		newPopulation->addIndividual(selected->clone());
	}
	return newPopulation;
}





//=============================================================================
//

} // namespace FuzzyFW
