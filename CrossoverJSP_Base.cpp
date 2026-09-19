/*
 * CrossoverJSP_Base.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CrossoverJSP_Base.h"

namespace JSP {

//=============================================================================
//
//	Class CrossoverJSP_Base
//
//=============================================================================
void CrossoverJSP_Base::apply(FuzzyFW::Individual *ind1,
	FuzzyFW::Individual *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;

	if (ind1->size() != ind2->size())
		throw JSPException("Crossover", "Individual sizes must match.");

	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	if (dynamic_cast<EncoderJSP_Order *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyPermutation(idv1, idv2, svars);
	}
	if (dynamic_cast<EncoderJSP_JobOrder *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyJobPermutation(idv1, idv2, svars);
	}
	throw JSPException("Crossover",
		"Individuals must be coded as permutation to use this crossover operator");
}

std::vector<int> CrossoverJSP_Base::buildTaskGenotype(
	FuzzyFW::IndividualArrayInt *ind, ProblemJSP *prob) {

	std::vector<int> counter(prob->getNumberJobs(), 0);
	std::vector<int> genotype(ind->size());
	for (unsigned int i = 0; i < ind->size(); i++) {
		unsigned int job = ind->getGene(i);
		genotype[i] = prob->getTaskId(job, counter[job]);
		counter[job]++;
	}
	return genotype;
}

} // namespace JSP
