/*
 * CrossoverIJSP_Base.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CrossoverIJSP_Base.h"

namespace IJSP {

//=============================================================================
//
//	Class CrossoverIJSP_Base
//
//=============================================================================
void CrossoverIJSP_Base::apply(FuzzyFW::Individual *ind1,
	FuzzyFW::Individual *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;

	if (ind1->size() != ind2->size())
		throw IJSPException("Crossover", "Individual sizes must match.");

	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	if (dynamic_cast<EncoderIJSP_Order *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyPermutation(idv1, idv2, svars);
	}
	if (dynamic_cast<EncoderIJSP_JobOrder *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyJobPermutation(idv1, idv2, svars);
	}
	throw IJSPException("Crossover",
		"Individuals must be coded as permutation to use this crossover operator");
}

} // namespace IJSP
