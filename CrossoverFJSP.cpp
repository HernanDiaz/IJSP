/*
 * Crossover.cpp
 *
 *  Created on: Jul 14, 2017
 */

#include "./CrossoverFJSP.h"

namespace FJSP {

//=============================================================================
//
//	Class Crossover_JOX
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Individual)  ==================================================
void Crossover_JOX::apply(FuzzyFW::Individual *ind1, FuzzyFW::Individual *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;

	if (ind1->size() != ind2->size()) {
		throw new FJSPException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	if (dynamic_cast<EncoderFJSP_Order *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyPermutation(idv1, idv2, svars);
	}
	if (dynamic_cast<EncoderFJSP_JobOrder *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyJobPermutation(idv1, idv2, svars);
	}
	std::string errorMsg = "Individuals must be coded as permutation";
	errorMsg += " to use this crossover operator";
	throw new FJSPException("Crossover", errorMsg);
}



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
	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Creation", errorMsg);
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
	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Creation", errorMsg);
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





//=============================================================================
//
//	Class Crossover_GOXBierwirth
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Individual)  ==================================================
void Crossover_GOXBierwirth::apply(FuzzyFW::Individual *ind1,
	FuzzyFW::Individual *ind2, 
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;

	if (ind1->size() != ind2->size()) {
		throw new FJSPException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	if (dynamic_cast<EncoderFJSP_Order *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyPermutation(idv1, idv2, svars);
	}
	if (dynamic_cast<EncoderFJSP_JobOrder *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyJobPermutation(idv1, idv2, svars);
	}
	std::string errorMsg = "Individuals must be coded as permutation in";
	errorMsg += " to use this crossover operator";
	throw new FJSPException("Crossover", errorMsg);
}



//=====  Apply (Permutation)  =================================================
void Crossover_GOXBierwirth::applyPermutation(FuzzyFW::IndividualArrayInt *ind1,
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
			if (pos2 >= nGenes || ind2->getGene(pos2) == ind1->getGene(pos1))
				for (unsigned int i = 0; i < length; i++)
					offs1.push_back(ind1->getGene(pos1 + i));
			else if (!content1[ind2->getGene(pos2)])
				offs1.push_back(ind2->getGene(pos2));
			pos2++;
		}
		// Build the second offspring
		pos2 = 0;
		while (offs2.size() < nGenes) {
			if (pos2 >= nGenes || ind1->getGene(pos2) == ind2->getGene(pos1))
				for (unsigned int i = 0; i < length; i++)
					offs2.push_back(ind2->getGene(pos1 + i));
			else if (!content2[ind1->getGene(pos2)])
				offs2.push_back(ind1->getGene(pos2));
			pos2++;
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
void Crossover_GOXBierwirth::applyJobPermutation(FuzzyFW::IndividualArrayInt *ind1,
	FuzzyFW::IndividualArrayInt *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	std::vector<int> offs1, offs2;
	unsigned int nGenes, pos1, pos2, length, job;
	std::vector<char> content1, content2;
	std::vector<int> counter, genotype1, genotype2;

	nGenes = ind1->size();

	// Convert the problem type
	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Creation", errorMsg);
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
			if (pos2 >= nGenes || genotype2[pos2] == genotype1[pos1])
				for (unsigned int i = 0; i < length; i++)
					offs1.push_back(ind1->getGene(pos1 + i));
			else if (!content1[genotype2[pos2]])
				offs1.push_back(ind2->getGene(pos2));
			pos2++;
		}
		// Build the second offspring
		pos2 = 0;
		while (offs2.size() < nGenes) {
			if (pos2 >= nGenes || genotype1[pos2] == genotype2[pos1])
				for (unsigned int i = 0; i < length; i++)
					offs2.push_back(ind2->getGene(pos1 + i));
			else if (!content2[genotype1[pos2]])
				offs2.push_back(ind1->getGene(pos2));
			pos2++;
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





//=============================================================================
//
//	Class Crossover_GPMXBierwirth
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Individual)  ==================================================
void Crossover_GPMXBierwirth::apply(FuzzyFW::Individual *ind1,
	FuzzyFW::Individual *ind2, 
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;

	if (ind1->size() != ind2->size()) {
		throw new FJSPException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	if (dynamic_cast<EncoderFJSP_Order *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyPermutation(idv1, idv2, svars);
	}
	if (dynamic_cast<EncoderFJSP_JobOrder *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyJobPermutation(idv1, idv2, svars);
	}
	std::string errorMsg = "Individuals must be coded as permutation in";
	errorMsg += " to use this crossover operator";
	throw new FJSPException("Crossover", errorMsg);
}



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
	unsigned int nGenes, pos1, pos2, length, job;
	std::vector<char> content1, content2;
	std::vector<int> counter, genotype1, genotype2;

	nGenes = ind1->size();

	// Convert the problem type
	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Creation", errorMsg);
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
	if (offs1.size() > nGenes || offs2.size() > nGenes)
		std::cout << "Stop";
	ind1->setGenotype(offs1);
	ind2->setGenotype(offs2);
}





//=============================================================================
//
//	Class Crossover_PPXBierwirth
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Apply (Individual)  ==================================================
void Crossover_PPXBierwirth::apply(FuzzyFW::Individual *ind1,
	FuzzyFW::Individual *ind2,
	const FuzzyFW::SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;

	if (ind1->size() != ind2->size()) {
		throw new FJSPException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	if (dynamic_cast<EncoderFJSP_Order *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyPermutation(idv1, idv2, svars);
	}
	if (dynamic_cast<EncoderFJSP_JobOrder *>(svars->encoder) != NULL) {
		idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
		idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
		return this->applyJobPermutation(idv1, idv2, svars);
	}
	std::string errorMsg = "Individuals must be coded as permutation in";
	errorMsg += " to use this crossover operator";
	throw new FJSPException("Crossover", errorMsg);
}



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
	ProblemFJSP * fuzzyProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "fuzzy problems.";
		throw new FJSPException("Creation", errorMsg);
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


}
