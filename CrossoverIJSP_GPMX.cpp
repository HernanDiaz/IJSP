/*
 * CrossoverIJSP_GPMX.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CrossoverIJSP_GPMX.h"

namespace IJSP {

//=============================================================================
//
//	Class Crossover_GPMXBierwirth
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Permutation)  =================================================
void Crossover_GPMXBierwirth::applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> offs1, offs2;
	unsigned int nGenes, pos1, pos2, length;
	std::vector<char> content1, content2;


	// Find crossover point and string length
	nGenes = ind1->size();
	pos1 = svars->rng->getInteger(0, nGenes - 1);
	length = svars->rng->getInteger(
		roundToCeil(nGenes / 3.0), nGenes / 2);

	// Check the genes contained in the defined string
	content1.resize(nGenes, false);
	content2.resize(nGenes, false);

	for (unsigned int i = 0; i < length; i++) {
		content1[ind1->getGene((pos1 + i) % nGenes)] = true;
		content2[ind2->getGene((pos1 + i) % nGenes)] = true;
	}

	// The crossover makes a clear distinction depending on the selected chain
	// If all the chain is contained between the boundaries of the genotype:
	if (pos1 + length < nGenes) {

		// Build the first offspring
		pos2 = 0;
		while (offs1.size() < nGenes) {
			if (pos2 >= nGenes || offs1.size() == pos1)
				for (unsigned int i = 0; i < length; i++)
					offs1.push_back(ind1->getGene(pos1 + i));
			else {
				if (!content1[ind2->getGene(pos2)])
					offs1.push_back(ind2->getGene(pos2));
				pos2++;
			}
		}
		// Build the second offspring
		pos2 = 0;
		while (offs2.size() < nGenes) {
			if (pos2 >= nGenes || offs2.size() == pos1)
				for (unsigned int i = 0; i < length; i++)
					offs2.push_back(ind2->getGene(pos1 + i));
			else {
				if (!content2[ind1->getGene(pos2)])
					offs2.push_back(ind1->getGene(pos2));
				pos2++;
			}
		}
	}

	// If the chain goes over the boundaries of the genotype:
	else {

		// Build the first offspring
		for (unsigned int i = 0; i < (pos1 + length) % nGenes; i++)
			offs1.push_back(ind1->getGene(i));
		for (unsigned int i = 0; offs1.size() < pos1 && i < nGenes; i++)
			if (!content1[ind2->getGene(i)])
				offs1.push_back(ind2->getGene(i));
		for (unsigned int i = pos1; i < nGenes; i++)
			offs1.push_back(ind1->getGene(i));

		// Build the second offspring
		for (unsigned int i = 0; i < (pos1 + length) % nGenes; i++)
			offs2.push_back(ind2->getGene(i));
		for (unsigned int i = 0; offs2.size() < pos1 && i < nGenes; i++)
			if (!content2[ind1->getGene(i)])
				offs2.push_back(ind1->getGene(i));
		for (unsigned int i = pos1; i < nGenes; i++)
			offs2.push_back(ind2->getGene(i));


	}
	ind1->setGenotype(offs1);
	ind2->setGenotype(offs2);
}



//=====  Apply (Permutation with Repetitions)  ================================
void Crossover_GPMXBierwirth::applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> offs1, offs2;
	unsigned int nGenes, pos1, pos2, length;
	std::vector<char> content1, content2;

	nGenes = ind1->size();

	ProblemIJSP *fuzzyProb = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == nullptr)
		throw IJSPException("Creation", "This encoding function works only with Interval problems.");
	std::vector<int> genotype1 = buildTaskGenotype(ind1, fuzzyProb);
	std::vector<int> genotype2 = buildTaskGenotype(ind2, fuzzyProb);

	// Find crossover point and string length
	pos1 = svars->rng->getInteger(0, nGenes - 1);
	length = svars->rng->getInteger(
		roundToCeil(nGenes / 3.0), nGenes / 2);

	// Check the genes contained in the defined string
	content1.resize(nGenes, false);
	content2.resize(nGenes, false);

	for (unsigned int i = 0; i < length; i++) {
		content1[genotype1[(pos1 + i) % nGenes]] = true;
		content2[genotype2[(pos1 + i) % nGenes]] = true;
	}

	// The crossover makes a clear distinction depending on the selected chain
	// If all the chain is contained between the boundaries of the genotype:
	if (pos1 + length < nGenes) {

		// Build the first offspring
		pos2 = 0;
		while (offs1.size() < nGenes) {
			if (pos2 >= nGenes || offs1.size() == pos1)
				for (unsigned int i = 0; i < length; i++)
					offs1.push_back(ind1->getGene(pos1 + i));
			else {
				if (!content1[genotype2[pos2]])
					offs1.push_back(ind2->getGene(pos2));
				pos2++;
			}
		}
		// Build the second offspring
		pos2 = 0;
		while (offs2.size() < nGenes) {
			if (pos2 >= nGenes || offs2.size() == pos1)
				for (unsigned int i = 0; i < length; i++)
					offs2.push_back(ind2->getGene(pos1 + i));
			else {
				if (!content2[genotype1[pos2]])
					offs2.push_back(ind1->getGene(pos2));
				pos2++;
			}
		}
	}

	// If the chain goes over the boundaries of the genotype:
	else {

		// Build the first offspring
		for (unsigned int i = 0; i < (pos1 + length) % nGenes; i++)
			offs1.push_back(ind1->getGene(i));
		for (unsigned int i = 0; offs1.size() < pos1 && i < nGenes; i++)
			if (!content1[genotype2[i]])
				offs1.push_back(ind2->getGene(i));
		for (unsigned int i = pos1; i < nGenes; i++)
			offs1.push_back(ind1->getGene(i));

		// Build the second offspring
		for (unsigned int i = 0; i < (pos1 + length) % nGenes; i++)
			offs2.push_back(ind2->getGene(i));
		for (unsigned int i = 0; offs2.size() < pos1 && i < nGenes; i++)
			if (!content2[genotype1[i]])
				offs2.push_back(ind1->getGene(i));
		for (unsigned int i = pos1; i < nGenes; i++)
			offs2.push_back(ind2->getGene(i));


	}
	ind1->setGenotype(offs1);
	ind2->setGenotype(offs2);
}

} // namespace IJSP
