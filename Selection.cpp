/*
 * Selection.cpp
 *
 *  Created on: Aug 2, 2017
 *      Author: jjpalacios
 */

#include "Selection.h"

namespace FJSP {

//=============================================================================
//
//	Abstract class Selection
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
Selection::Selection(ParameterDB * parameters) {
	if(parameters != NULL)
		this->setup(parameters);
}



//=============================================================================
//		METHODS
//=============================================================================
//-----  apply (Population)  --------------------------------------------------
Population * Selection::apply(Population *population,
		const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	for(unsigned int i=0; i < population->size(); i++) {
		selected = this->select(population, svars);
		newPopulation->addIndividual(selected->clone());
	}
	return newPopulation;
}





//=============================================================================
//
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
		throw new FJSPException("Selection", errMsg);
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

	if (Fitness::FitnessMaximize) {
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
//	Class SUSSelection
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  apply  --------------------------------------------------------------
Population * SelectionSUS::apply(Population *population,
		const SharedVars *svars) const {
	
	int n = population->size();
	double sumValues, fitness;
	double sum, ptr;

	Population * newPopulation = new Population();
	std::vector<Individual *> selectedIndividuals;

	// Array of expected picks per individual
	std::vector<double> expected(n);

	if (this->interpolate) {
		sumValues = (n*(n + 1) / 2);
		for (unsigned int i = 1; i <= population->size(); i++) {
			expected[population->whoIsBest(svars, n - i)] = n * i / sumValues;
		}
	}
	else if(Fitness::FitnessMaximize) {
		sumValues = population->getAverageFitness() * population->size();
		for (unsigned int i = 0; i < population->size(); i++) {
			fitness = population->getIndividual(i)->getFitness()->toDouble();
			expected[population->whoIsBest(svars, n - i)] =
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
			expected[population->whoIsBest(svars, n - i)] =
				n * fitness / sumValues;
		}
	}
	
	// Residual value
	ptr = svars->rng->getProbability();

	// SUS algorithm
	sum = 0.0;
	for(int i=0; i < n; i++) {
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
//	Class ShuffleSelection
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionShuffle::select(Population *population,
		const SharedVars *svars) const {
	int randomValue = svars->rng->getInteger(0, population->size()-1);
	return population->getIndividual(randomValue);
}



//-----  apply  --------------------------------------------------------------
Population * SelectionShuffle::apply(Population *population,
		const SharedVars *svars) const {
	std::vector<int> randomOrder =
			svars->rng->getRandomVector(0, population->size()-1);

	Population * newPopulation = new Population();
	Individual * selected;
	for(unsigned int i=0; i < population->size(); i++) {
		selected = population->getIndividual(randomOrder[i]);
		newPopulation->addIndividual(selected->clone());
	}

	return newPopulation;
}





//=============================================================================
//
//	Class SelectionRandom
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionRandom::select(Population *population,
		const SharedVars *svars) const {

	int randomValue = svars->rng->getInteger(0, population->size()-1);
	return population->getIndividual(randomValue);
}


}
