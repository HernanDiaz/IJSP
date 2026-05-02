/*
 * Selection_Roulette.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_Roulette.h"

namespace FuzzyFW {

//	Class RouletteSelection
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void SelectionRoulette::setup(ParameterDB * parameters) {
	// Load the parameters of your parent
	Selection::setup(parameters);

	// Load own parameters
	this->interpolate = parameters->getBoolean(this->interpolateLabel, true);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionRoulette::select(Population *population,
		const SharedVars *svars) const {

	if (population->size() == 0)
		return NULL;
	if (population->size() == 1)
		return population->getIndividual(0);

	if (this->interpolate)
		return this->selectInterpolate(population, svars);
	return this->selectClassic(population, svars);
}



//-----  select with interpolation  -------------------------------------------
Individual * SelectionRoulette::selectInterpolate(Population *population,
	const SharedVars *svars) const {

	long int n = population->size();
	long int sumValues = (n*(n + 1)) / 2;	// God bless Gauss

	// Generates a random value
	int randomValue = svars->rng->getInteger(0, sumValues - 1);

	// Looks for the individual to pick
	int acum = n;
	int cont = 0;
	while (acum < randomValue) {
		cont++;
		acum += (n - cont);
	}

	return population->getBest(svars, cont);
}



//-----  select with classical method  ----------------------------------------
Individual * SelectionRoulette::selectClassic(Population *population,
	const SharedVars *svars) const {

	int n = population->size();
	double sumValues, randomValue, acum;
	int cont = 0;

	if (n <= 0)
		return NULL;

	if (population->getIndividual(0)->getFitness()->mustMaximize()) {
		sumValues = population->getAverageFitness() * population->size();
		randomValue = svars->rng->getDouble(0, sumValues);

		// Looks for the individual to pick
		acum = population->getIndividual(0)->getFitness()->toDouble();
		while (acum < randomValue) {
			cont++;
			acum += population->getIndividual(cont)->getFitness()->toDouble();
		}
	}
	else {
		sumValues = 0.0;
		for (unsigned int i = 0; i < population->size(); i++) {
			sumValues += 1 /
				population->getIndividual(i)->getFitness()->toDouble();
		}
		randomValue = svars->rng->getDouble(0, sumValues);
		
		// Looks for the individual to pick
		acum = 1 /
			population->getIndividual(0)->getFitness()->toDouble();
		while (acum < randomValue) {
			cont++;
			acum += 1 /
				population->getIndividual(cont)->getFitness()->toDouble();
		}
	}
	return population->getIndividual(cont);
}





//=============================================================================
//

} // namespace FuzzyFW
