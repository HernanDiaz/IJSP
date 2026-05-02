/*
 * CrossoverIJSP_JOX.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CrossoverIJSP_JOX.h"

namespace IJSP {

//=============================================================================
//
//	Class Crossover_JOX
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Permutation)  =================================================
void Crossover_JOX::applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	unsigned int count1, count2;
	int gene, job;
	bool different1, different2;
	std::vector<int> mask;
	std::vector<int> offs1, offs2;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Choose the jobs to keep in position
	mask.resize(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		if (svars->rng->getProbability() < 0.5)
			mask[i] = 0;
		else mask[i] = 1;
	}

	// Build the first offspring
	offs1.resize(ind1->size());

	different1 = false;
	count1 = count2 = 0;
	while (count1 < ind1->size()) {
		gene = ind1->getGene(count1);
		job = fuzzyProb->getTask(gene)->job;

		if (mask[job] == 1)
			offs1[count1] = gene;
		else {
			gene = ind2->getGene(count2);
			job = fuzzyProb->getTask(gene)->job;

			while (count2 < ind2->size() - 1 && mask[job] == 1) {
				count2++;
				gene = ind2->getGene(count2);
				job = fuzzyProb->getTask(gene)->job;
			}
			offs1[count1] = gene;
			count2++;
		}
		if (offs1[count1] != ind1->getGene(count1))
			different1 = true;
		count1++;
	}


	// Build the second offspring
	offs2.resize(ind2->size());

	different2 = false;
	count1 = count2 = 0;
	while (count1 < ind2->size()) {
		gene = ind2->getGene(count1);
		job = fuzzyProb->getTask(gene)->job;

		if (mask[job] == 1)
			offs2[count1] = gene;
		else {
			gene = ind1->getGene(count2);
			job = fuzzyProb->getTask(gene)->job;

			while (count2 < ind1->size() - 1 && mask[job] == 1) {
				count2++;
				gene = ind1->getGene(count2);
				job = fuzzyProb->getTask(gene)->job;
			}
			offs2[count1] = gene;
			count2++;
		}
		if (offs2[count1] != ind2->getGene(count1))
			different2 = true;
		count1++;
	}

	if (different1)
		ind1->setGenotype(offs1);
	if (different2)
		ind2->setGenotype(offs2);
}



//=====  Apply (Permutation with Repetitions)  ================================
void Crossover_JOX::applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	unsigned int count1, count2;
	int gene;
	bool different1, different2;
	std::vector<int> mask;
	std::vector<int> offs1, offs2;


	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Choose the jobs to keep in position
	mask.resize(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		if (svars->rng->getProbability() < 0.5)
			mask[i] = 0;
		else mask[i] = 1;
	}

	// Build the first offspring
	offs1.resize(ind1->size());

	different1 = false;
	count1 = count2 = 0;
	while (count1 < ind1->size()) {
		gene = ind1->getGene(count1);

		if (mask[gene] == 1)
			offs1[count1] = gene;
		else {
			gene = ind2->getGene(count2);

			while (count2 < ind2->size() - 1 && mask[gene] == 1) {
				count2++;
				gene = ind2->getGene(count2);
			}
			offs1[count1] = gene;
			count2++;
		}
		if (offs1[count1] != ind1->getGene(count1))
			different1 = true;
		count1++;
	}


	// Build the second offspring
	offs2.resize(ind2->size());

	different2 = false;
	count1 = count2 = 0;
	while (count1 < ind2->size()) {
		gene = ind2->getGene(count1);

		if (mask[gene] == 1)
			offs2[count1] = gene;
		else {
			gene = ind1->getGene(count2);

			while (count2 < ind1->size() - 1 && mask[gene] == 1) {
				count2++;
				gene = ind1->getGene(count2);
			}
			offs2[count1] = gene;
			count2++;
		}
		if (offs2[count1] != ind2->getGene(count1))
			different2 = true;
		count1++;
	}

	if (different1)
		ind1->setGenotype(offs1);
	if (different2)
		ind2->setGenotype(offs2);
}

} // namespace IJSP
