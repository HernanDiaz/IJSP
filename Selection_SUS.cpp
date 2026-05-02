/*
 * Selection_SUS.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_SUS.h"

namespace FuzzyFW {

//	Class SUSSelection
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  apply  --------------------------------------------------------------
Population * SelectionSUS::apply(Population *population, const unsigned int n,
		const SharedVars *svars) const {
	
	double sumValues, fitness;
	double sum, ptr;

	Population * newPopulation = new Population();
	std::vector<Individual *> selectedIndividuals;

	// Array of expected picks per individual
	std::vector<double> expected(population->size());

	if (n <= 0)
		return NULL;

	if (this->interpolate) {
		sumValues = (n*(n + 1) / 2);
		for (unsigned int i = 1; i <= population->size(); i++) {
			expected[population->whoIsBest(svars, population->size() - i)] = n * i / sumValues;
		}
	}
	else if (population->getIndividual(0)->getFitness()->mustMaximize()) {
		sumValues = population->getAverageFitness() * population->size();
		for (unsigned int i = 0; i < population->size(); i++) {
			fitness = population->getIndividual(i)->getFitness()->toDouble();
			expected[population->whoIsBest(svars, population->size() - i)] =
				n * fitness / sumValues;
		}
	}
	else {
		sumValues = 0.0;
		for (unsigned int i = 0; i < population->size(); i++) {
			sumValues += 1 /
				population->getIndividual(i)->getFitness()->toDouble();
		}

		for (unsigned int i = 0; i < population->size(); i++) {
			fitness = 1 / population->getIndividual(i)->getFitness()->toDouble();
			expected[population->whoIsBest(svars, population->size() - i)] =
				n * fitness / sumValues;
		}
	}
	
	// Residual value
	ptr = svars->rng->getProbability();

	// SUS algorithm
	sum = 0.0;
	for(unsigned int i=0; i < population->size(); i++) {
		sum += expected[i];
		while(sum > ptr) {
			selectedIndividuals.push_back(population->getIndividual(i));
			ptr += 1.0;
		}
	}
	svars->rng->shuffle(selectedIndividuals);
	for (unsigned int i = 0; i < selectedIndividuals.size(); i++) {
		newPopulation->addIndividual(selectedIndividuals[i]->clone());
	}
	return newPopulation;
}





//=============================================================================
//

} // namespace FuzzyFW
