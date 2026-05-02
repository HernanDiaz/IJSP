/*
 * CrossoverIJSP_PPX.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CrossoverIJSP_PPX.h"

namespace IJSP {

//=============================================================================
//
//	Class Crossover_PPXBierwirth
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Permutation)  =================================================
void Crossover_PPXBierwirth::applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> offs1, offs2;
	unsigned int nGenes, pos1, pos2, offsSize;
	std::vector<char> passed, mask;

	nGenes = ind1->size();
	mask.resize(nGenes);

	for (unsigned int i = 0; i < nGenes; i++)
		mask[i] = svars->rng->getInteger(1, 2);

	// Create the first offsrping
	pos1 = pos2 = offsSize = 0;
	passed.resize(nGenes, false);
	while (offsSize < nGenes) {
		if (mask[offsSize] == 1) {
			while (pos1 < nGenes && passed[ind1->getGene(pos1)])
				pos1++;
			if (pos1 < nGenes) {
				offs1.push_back(ind1->getGene(pos1));
				passed[ind1->getGene(pos1)] = true;
			}
		}
		else {
			while (pos2 < nGenes && passed[ind2->getGene(pos2)])
				pos2++;
			if (pos2 < nGenes) {
				offs1.push_back(ind2->getGene(pos2));
				passed[ind2->getGene(pos2)] = true;
			}
		}
		offsSize = offs1.size();
	}

	// Create the second offsrping
	pos1 = pos2 = offsSize = 0;
	passed.clear();
	passed.resize(nGenes, false);
	while (offsSize < nGenes) {
		if (mask[offsSize] == 1) {
			while (pos1 < nGenes && passed[ind1->getGene(pos1)])
				pos1++;
			if (pos1 < nGenes) {
				offs2.push_back(ind1->getGene(pos1));
				passed[ind1->getGene(pos1)] = true;
			}
		}
		else {
			while (pos2 < nGenes && passed[ind2->getGene(pos2)])
				pos2++;
			if (pos2 < nGenes) {
				offs2.push_back(ind2->getGene(pos2));
				passed[ind2->getGene(pos2)] = true;
			}
		}
		offsSize = offs2.size();
	}

	ind1->setGenotype(offs1);
	ind2->setGenotype(offs2);
}



//=====  Apply (Permutation with Repetitions)  ================================
void Crossover_PPXBierwirth::applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> offs1, offs2;
	unsigned int nGenes, pos1, pos2, job, offsSize;
	std::vector<char> content1, content2;
	std::vector<int> counter, genotype1, genotype2;
	std::vector<char> passed, mask;

	nGenes = ind1->size();

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Generate genotypes without repetition
	counter.resize(fuzzyProb->getNumberJobs(), 0);
	genotype1.resize(nGenes);
	for (unsigned int i = 0; i < nGenes; i++) {
		job = ind1->getGene(i);
		genotype1[i] = fuzzyProb->getTaskId(job, counter[job]);
		counter[job]++;
	}

	counter.clear();
	counter.resize(fuzzyProb->getNumberJobs(), 0);
	genotype2.resize(nGenes);
	for (unsigned int i = 0; i < nGenes; i++) {
		job = ind2->getGene(i);
		genotype2[i] = fuzzyProb->getTaskId(job, counter[job]);
		counter[job]++;
	}

	mask.resize(nGenes);

	for (unsigned int i = 0; i < nGenes; i++)
		mask[i] = svars->rng->getInteger(1, 2);

	// Create the first offsrping
	pos1 = pos2 = offsSize = 0;
	passed.resize(nGenes, false);
	while (offsSize < nGenes) {
		if (mask[offsSize] == 1) {
			while (pos1 < nGenes && passed[genotype1[pos1]])
				pos1++;
			if (pos1 < nGenes) {
				offs1.push_back(ind1->getGene(pos1));
				passed[genotype1[pos1]] = true;
			}
		}
		else {
			while (pos2 < nGenes && passed[genotype2[pos2]])
				pos2++;
			if (pos2 < nGenes) {
				offs1.push_back(ind2->getGene(pos2));
				passed[genotype2[pos2]] = true;
			}
		}
		offsSize = offs1.size();
	}

	// Create the second offsrping
	pos1 = pos2 = offsSize = 0;
	passed.clear();
	passed.resize(nGenes, false);
	while (offsSize < nGenes) {
		if (mask[offsSize] == 1) {
			while (pos1 < nGenes && passed[genotype1[pos1]])
				pos1++;
			if (pos1 < nGenes) {
				offs2.push_back(ind1->getGene(pos1));
				passed[genotype1[pos1]] = true;
			}
		}
		else {
			while (pos2 < nGenes && passed[genotype2[pos2]])
				pos2++;
			if (pos2 < nGenes) {
				offs2.push_back(ind2->getGene(pos2));
				passed[genotype2[pos2]] = true;
			}
		}
		offsSize = offs2.size();
	}

	ind1->setGenotype(offs1);
	ind2->setGenotype(offs2);
}

} // namespace IJSP
