/*
 * Replacement.cpp
 *
 *  Created on: Aug 2, 2017
 *      Author: jjpalacios
 */


#include "Replacement.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class Replacement
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
Replacement::Replacement(ParameterDB * parameters)
	: labelRepeat(REPLACE_REPEAT), allowRepeated(true) {
	if(parameters != NULL)
		this->setup(parameters);
}


//-----  Setup method  --------------------------------------------------------
void Replacement::setup(ParameterDB * parameters) {
	// Load own parameters
	this->allowRepeated = parameters->getBoolean(this->labelRepeat, true);
}





//=============================================================================
//
//	Class ReplacementElitist
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void ReplacementElitist::setup(ParameterDB * parameters) {
	// Load the parameters of your parent
	Replacement::setup(parameters);

	// Load own parameters
	this->elite = parameters->getInteger(this->labelElite, -1);
	std::cout << "Warning: Elite parameter not found for the Replacement";
	std::cout << " strategy. No elitism is assumed";
}


//=============================================================================
//		METHODS
//=============================================================================
//-----  apply  ---------------------------------------------------------------
void ReplacementElitist::apply(Population *oldPopulation,
		Population *newPopulation, const SharedVars *svars) const {

	/* Look for the elite worst individuals in the new population.
	 * This will save time sorting the population each time we replace an
	 * individual.
	 */
	unsigned int popSize = newPopulation->size();
	Individual *replaced, *oldInd, *newInd;
	std::vector<int> best, worst;

	for (unsigned int i = 0; i < this->elite; i++)
		worst.push_back(newPopulation->whoIsBest(svars, popSize - i - 1));

	if (this->allowRepeated) {
		for (unsigned int i = 0; i < this->elite; i++)
			best.push_back(i);
	}
	else {
		unsigned int cont = 0, find;
		while (best.size() < this->elite && cont < oldPopulation->size()) {
			oldInd = oldPopulation->getBest(svars, cont);

			// Check if already exists
			find = 1;
			newInd = newPopulation->getBest(svars, 0);
			while (find < newPopulation->size()) {
				newInd = newPopulation->getBest(svars, find);
				if (newInd->getFitness()->isBetterThan(oldInd->getFitness()))
					find++;
				else break;
			}
				
			if (find == newPopulation->size() ||
				!newInd->getFitness()->isEqualTo(oldInd->getFitness()))
				best.push_back(cont);
			cont++;
		}
	}

	// Move the best individuals of the old population to the new one
	for(unsigned int i=0; i < best.size(); i++) {
		replaced = newPopulation->replaceIndividual(
				worst[i],	// Pick worst
				oldPopulation->getBest(svars, best[i])->clone());
		delete replaced;
	}
	delete oldPopulation;
}





//=============================================================================
//
//	Class ReplacementParents
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  apply  ---------------------------------------------------------------
void ReplacementParents::apply(Population *oldPopulation,
	Population *newPopulation, const SharedVars *svars) const {

	if (this->allowRepeated)
		this->applyRepeat(oldPopulation, newPopulation);
	else
		this->applyNoRepeat(oldPopulation, newPopulation);
}



//-----  apply (repated elements allowed)  ------------------------------------
void ReplacementParents::applyRepeat(Population *oldPopulation,
	Population *newPopulation) const {

	std::vector<Individual *> family(4);
	unsigned int best, best2;

	for (unsigned int i = 0; i < newPopulation->size(); i += 2) {
		family[0] = newPopulation->getIndividual(i);
		family[1] = newPopulation->getIndividual(i + 1);
		family[2] = newPopulation->getIndividual(family[0]->id);
		family[3] = newPopulation->getIndividual(family[1]->id);

		if (family[1]->getFitness()->isBetterThan(family[0]->getFitness())) {
			std::swap(family[0], family[1]);
			best = i + 1;
			best2 = i;
		}
		else {
			best = i;
			best2 = i + 1;
		}
		if (family[3]->getFitness()->isBetterThan(family[2]->getFitness()))
			std::swap(family[2], family[3]);

		if (family[2]->getFitness()->isBetterThan(family[1]->getFitness())) {
			newPopulation->replaceIndividual(best2, family[2]->clone());
			if (family[2]->getFitness()->isBetterThan(family[0]->getFitness())) {
				family[1] = family[0];
				family[0] = family[2];
				std::swap(best2, best);

				if (family[3]->getFitness()->isBetterThan(family[1]->getFitness()))
					newPopulation->replaceIndividual(best2, family[3]->clone());
			}
		}
	}

	delete oldPopulation;
}



//-----  apply (repeated elements not allowed)  -------------------------------
void ReplacementParents::applyNoRepeat(Population *oldPopulation,
	Population *newPopulation) const {

	std::vector<Individual *> family(4);
	unsigned int best, best2;
	char isRepeated;

	for (unsigned int i = 0; i < newPopulation->size(); i += 2) {
		family[0] = newPopulation->getIndividual(i);
		family[1] = newPopulation->getIndividual(i + 1);
		family[2] = newPopulation->getIndividual(family[0]->id);
		family[3] = newPopulation->getIndividual(family[1]->id);

		if (family[1]->getFitness()->isBetterThan(family[0]->getFitness())) {
			std::swap(family[0], family[1]);
			best = i + 1;
			best2 = i;
		}
		else {
			best = i;
			best2 = i + 1;
		}
		if (family[3]->getFitness()->isBetterThan(family[2]->getFitness()))
			std::swap(family[2], family[3]);

		if (family[0]->getFitness()->isEqualTo(family[1]->getFitness()))
			isRepeated = true;
		else
			isRepeated = false;

		if (isRepeated ||
			family[2]->getFitness()->isBetterThan(family[1]->getFitness())) {

			if (!family[2]->getFitness()->isEqualTo(family[0]->getFitness())) {
				newPopulation->replaceIndividual(best2, family[2]->clone());
				if (family[2]->getFitness()->isBetterThan(family[0]->getFitness())) {
					family[1] = family[0];
					family[0] = family[2];
					std::swap(best2, best);
				}
				else {
					family[1] = family[2];
				}
				isRepeated = false;
			}

			if (isRepeated ||
				family[3]->getFitness()->isBetterThan(family[1]->getFitness())) {
				if (!family[3]->getFitness()->isEqualTo(family[0]->getFitness()))
					newPopulation->replaceIndividual(best2, family[3]->clone());
			}
		}
	}

	delete oldPopulation;
}


}

