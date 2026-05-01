/*
 * Crossover.cpp
 *
 *  Created on: Jul 14, 2017
 */

#include "Crossover.h"

namespace FuzzyFW {

//=============================================================================
//
//	Abstract class Crossover
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
Crossover::Crossover(ParameterDB *parameters) {
	if(parameters != NULL)
		this->setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Population)  --------------------------------------------------
void Crossover::apply(Population *population, const double probability,
	const SharedVarsEvolutionary *svars) const {
	for(unsigned int i=0; i < population->size(); i+=2) {
		// If there are two parents, generate two offspring
		if((i+1) < population->size()) {
			if (svars->rng->getProbability() < probability) {
				// Cross the individuals to generate two offspring
				this->apply(population->getIndividual(i),
					population->getIndividual(i + 1), svars);
			}
		}

		// If there is only one parent, keep it
	}
}






//=============================================================================
//
//	Abstract class Crossover_OBC
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Pair)  --------------------------------------------------------
void Crossover_OBC::apply(Individual *ind1, Individual *ind2,
	const SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;
	std::vector<int> offs1, offs2;
	unsigned int nGenes, insertPos, parentPos, pos1, pos2;
	std::vector<char> remove1, remove2;
	char found;

	if (ind1->size() != ind2->size()) {
		throw new FuzzyFWException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;
	
	idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
	idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
	if (idv1 == NULL || idv2 == NULL) {
		std::string errorMsg = "This crossover operator can be applied only";
		errorMsg += " to permutations";
		throw new FuzzyFWException("Crossover", errorMsg);
	}


	// Find crossover points
	nGenes = ind1->size();
	pos1 = pos2 = svars->rng->getInteger(0, nGenes - 1);
	while(pos1 == pos2)
		pos2 = svars->rng->getInteger(0, nGenes - 1);
	if (pos1 > pos2)
		std::swap(pos1,pos2);

	// Check the genes contained in the defined string
	remove1.resize(nGenes, false);
	remove2.resize(nGenes, false);
	offs1.resize(nGenes, -1);
	offs2.resize(nGenes, -1);

	for (unsigned int i = pos1; i <= pos2; i++) {
		found = false;
		for (unsigned int j = 0; j < nGenes && !found; j++) {
			if (idv2->getGene(j) == idv1->getGene(i)) {
				remove1[j] = true;
				found = true;
			}
		}
		found = false;
		for (unsigned int j = 0; j < nGenes && !found; j++) {
			if (idv1->getGene(j) == idv2->getGene(i)) {
				remove2[j] = true;
				found = true;
			}
		}
		offs1[i] = idv1->getGene(i);
		offs2[i] = idv2->getGene(i);
	}


	// Build the first offspring
	insertPos = parentPos = 0;
	while (insertPos < nGenes) {
		while (remove1[parentPos])
			parentPos++;
		offs1[insertPos] = idv2->getGene(parentPos);
		insertPos++;
		parentPos++;
		if (insertPos == pos1)
			insertPos = pos2 + 1;
	}

	// Build the second offspring
	insertPos = parentPos = 0;
	while (insertPos < nGenes) {
		while (remove2[parentPos])
			parentPos++;
		offs2[insertPos] = idv1->getGene(parentPos);
		insertPos++;
		parentPos++;
		if (insertPos == pos1)
			insertPos = pos2 + 1;
	}

	idv1->setGenotype(offs1);
	idv2->setGenotype(offs2);
}





//=============================================================================
//
//	Abstract class Crossover_CBC
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Pair)  --------------------------------------------------------
void Crossover_CBC::apply(Individual *ind1, Individual *ind2,
	const SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;
	std::vector<int> offs1, offs2, cycleIds;
	unsigned int nGenes, index, cont, cycle;
	int initialValue, currentValue;

	if (ind1->size() != ind2->size()) {
		throw new FuzzyFWException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
	idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
	if (idv1 == NULL || idv2 == NULL) {
		std::string errorMsg = "This crossover operator can be applied only";
		errorMsg += " to permutations";
		throw new FuzzyFWException("Crossover", errorMsg);
	}

	nGenes = ind1->size();

	// Identify the cycles
	cycle = 1;
	cycleIds.resize(nGenes, -1);
	cont = 0;
	while (cont < nGenes) {
		// Find the initial value for the cycle
		index = cycle - 1;
		while (cycleIds[index] >= 0)
			index++;

		initialValue = idv1->getGene(index);
		currentValue = idv2->getGene(index);
		cycleIds[index] = cycle;
		cont++;

		// Look for the current value
		while (currentValue != initialValue) {
			index = cycle;
			while (idv1->getGene(index) != currentValue)
				index++;
			cycleIds[index] = cycle;
			cont++;
			currentValue = idv2->getGene(index);
		}
		cycle++;
	}

	for (unsigned int i = 0; i < nGenes; i++) {
		if (cycleIds[i] % 2 == 0) {
			offs1.push_back(idv2->getGene(i));
			offs2.push_back(idv1->getGene(i));
		}
		else {
			offs1.push_back(idv1->getGene(i));
			offs2.push_back(idv2->getGene(i));
		}
	}

	idv1->setGenotype(offs1);
	idv2->setGenotype(offs2);
}





//=============================================================================
//
//	Abstract class Crossover_PMX
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Pair)  --------------------------------------------------------
void Crossover_PMX::apply(Individual *ind1, Individual *ind2,
	const SharedVarsEvolutionary *svars) const {

	FuzzyFW::IndividualArrayInt *idv1, *idv2;
	std::vector<int> offs1, offs2;
	std::vector<int> origin1, origin2;
	unsigned int nGenes, pos, pos1, pos2, cont;

	if (ind1->size() != ind2->size()) {
		throw new FuzzyFWException("Crossover", "Individual sizes must match.");
	}

	// Too small for crossover
	if (ind1->size() < 2 || ind2->size() < 2)
		return;

	idv1 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind1);
	idv2 = dynamic_cast<FuzzyFW::IndividualArrayInt *>(ind2);
	if (idv1 == NULL || idv2 == NULL) {
		std::string errorMsg = "This crossover operator can be applied only";
		errorMsg += " to permutations";
		throw new FuzzyFWException("Crossover", errorMsg);
	}

	nGenes = ind1->size();

	// Find crossover points
	nGenes = ind1->size();
	pos1 = pos2 = svars->rng->getInteger(0, nGenes - 1);
	while (pos1 == pos2)
		pos2 = svars->rng->getInteger(0, nGenes - 1);
	if (pos1 > pos2)
		std::swap(pos1, pos2);

	for (unsigned int i = 0; i < nGenes; i++) {
		// Look for the gene on the other parent
		cont = pos1;
		while (cont <= pos2 && idv1->getGene(cont) != idv2->getGene(i))
			cont++;
		if (cont <= pos2)
			origin1.push_back(cont);
		else
			origin1.push_back(-1);

		cont = pos1;
		while (cont <= pos2 && idv2->getGene(cont) != idv1->getGene(i))
			cont++;
		if (cont <= pos2)
			origin2.push_back(cont);
		else
			origin2.push_back(-1);
	}


	for (unsigned int i = 0; i < nGenes; i++) {
		if (i >= pos1 && i <= pos2) {
			offs1.push_back(idv1->getGene(i));
			offs2.push_back(idv2->getGene(i));
		}

		pos = i;
		while (origin1[pos] >= 0)
			pos = origin1[pos];
		offs1.push_back(idv2->getGene(pos));

		pos = i;
		while (origin2[pos] >= 0)
			pos = origin2[pos];
		offs2.push_back(idv1->getGene(pos));
	}

	idv1->setGenotype(offs1);
	idv2->setGenotype(offs2);
}









}
