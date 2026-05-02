/*
 * Selection_Tournament.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection_Tournament.h"

namespace FuzzyFW {

//	Class TournamentSelection
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Setup method  --------------------------------------------------------
void SelectionTournament::setup(ParameterDB * parameters) {
	// Load the parameters of your parent
	Selection::setup(parameters);

	// Load own parameters
	this->size = parameters->getInteger(this->sizeLabel, -1);
	if(this->size < 0) {
		std::string errMsg = "Invalid tournament size (";
		errMsg += valueToString(this->size) + "). ";
		errMsg += " Incorrect value or missing parameter.";
		throw FuzzyFWException("Selection", errMsg);
	}
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionTournament::select(Population *population,
		const SharedVars *svars) const {

	unsigned int bestPosition, randomPosition;
	Individual *selected;
	Fitness *bestFitness, *currentFitness;

	// Take an individual at random
	bestPosition = svars->rng->getInteger(0, population->size()-1);
	selected = population->getIndividual(bestPosition);
	bestFitness = selected->getFitness();
	
	if (population->size() < 2)
		return selected;

	// Perform the tournament
	for(unsigned int i=0; i < this->size; i++) {
		// Ensure that we choose a different individual for the tournament
		randomPosition = svars->rng->getInteger(0, population->size()-1);
		while(randomPosition == bestPosition)
			randomPosition = svars->rng->getInteger(0, population->size()-1);
		selected = population->getIndividual(randomPosition);
		currentFitness = selected->getFitness();

		// Compare with the current champion
		if(currentFitness->isBetterThan(bestFitness)) {
			bestFitness = currentFitness;
			bestPosition = randomPosition;
		}
	}

	return population->getIndividual(bestPosition);
}





//=============================================================================
//

} // namespace FuzzyFW
