/*
 * Selection.cpp
 *
 *  Created on: Aug 2, 2017
 */

#include "Selection.h"

namespace FuzzyFW {

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
Population * Selection::apply(Population *population, const unsigned int n,
		const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	for(unsigned int i=0; i < n; i++) {
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
		throw new FuzzyFWException("Selection", errMsg);
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
Population * SelectionShuffle::apply(Population *population, const unsigned int n,
		const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	std::vector<int> randomOrder;
	
	for (unsigned int i = 0; i < population->size(); i++) {
		randomOrder.push_back(i);
	} 
		
	while (newPopulation->size() < n) {
		svars->rng->shuffle(randomOrder);
		for (unsigned int i = 0; i < randomOrder.size() && newPopulation->size() < n; i++) {
			selected = population->getIndividual(randomOrder[i]);
			newPopulation->addIndividual(selected->clone());
		}
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


//=============================================================================
//
//	Class EliteSelection
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionElite::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}



//-----  apply  --------------------------------------------------------------
Population * SelectionElite::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population * newPopulation = new Population();
	Individual * selected;
	
	while (newPopulation->size() < n) {
		for (unsigned int i = 0;  newPopulation->size() < n; i++) {
			selected = select(population, svars);
			newPopulation->addIndividual(selected->clone());
		}
	}

	return newPopulation;
}

//=============================================================================
//
//	Class SelectionCellR5
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellR5::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellR5::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	std::vector<int> coords;
	coords.push_back(n);
	Population* grid = new Population();
	for (int i = 0; i < 5; i++) {
		int index = svars->rng->getInteger(0, population->size()-1);
		while  (std::find(coords.begin(), coords.end(), index) != coords.end()) {
			index = svars->rng->getInteger(0, population->size()-1);
		}
		coords.push_back(index);
		grid->addIndividual(population->getIndividual(index)->clone());
	}
	return grid;
}



//=============================================================================
//
//	Class SelectionCellR13
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellR13::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellR13::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	std::vector<int> coords;
	coords.push_back(n);
	Population* grid = new Population();
	for (int i = 0; i < 13; i++) {
		int index = svars->rng->getInteger(0, population->size() - 1);
		while (std::find(coords.begin(), coords.end(), index) != coords.end()) {
			index = svars->rng->getInteger(0, population->size() - 1);
		}
		coords.push_back(index);
		grid->addIndividual(population->getIndividual(index)->clone());
	}
	return grid;
}

//=============================================================================
//
//	Class SelectionCellL5
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellL5::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellL5::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coords(4);
	int square = sqrt(size);
	//coords[0] = n;
	//E
	/*
	if (n > 0) {
		coords[0] = n - 1;
	}
	else coords[0] = size - 1;
	//W
	if (n < size - 1) {
		coords[1] = n + 1;
	}
	else coords[1] = 0;
	*/
	//E
	if (n == 0 || n % square == 0) {
		coords[0] = n + square - 1;
	}
	else coords[0] = n - 1;

	//W
	if ((n + 1) % square == 0 || n == size - 1) {
		coords[1] = n - square + 1;
	}
	else coords[1] = n + 1;
	
	//N
	
	if (n < square) {
		coords[2] = size - square + n;
	}
	else coords[2] = n - square;
	//S
	if (n < size - square) {
		coords[3] = n + square;
	}
	else coords[3] = square - size + n;
	return coords;
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellL5::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population* grid = new Population();
	std::vector<int> coords = getCellsIndex(population->size(), n);
	for (int i = 0; i < coords.size(); i++) {
		if (coords[i] >= 0 && coords[i] < population->size()) {
			grid->addIndividual(population->getIndividual(coords[i])->clone());
		}
	}
	return grid;
}

//=============================================================================
//
//	Class SelectionCellL9
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellL9::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellL9::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coords = SelectionCellL5::getCellsIndex(size, n);
	int square = sqrt(size);
	/*
	//East +1
	if (n > 1) {
		coords.push_back(n - 2);
	}
	else coords.push_back(size - 2 - n);
	//West +1 
	if (n < size - 2) {
		coords.push_back(n + 2);
	}
	else coords.push_back(n - size + 2);
	*/

	//East +1
	 //E
	if (n == 0 || n == 1 || n % square == 0 || (n - 1) % square == 0) {
		coords.push_back(n + square - 2);
	}
	else coords.push_back(n - 2);

	//West +1 
	if ((n + 1) % square == 0 || n == size - 1 || n == size - 2 || (n + 2) % square == 0) {
		coords.push_back(n - square + 2);
	}
	else coords.push_back(n + 2);
	   	 
	//North +1
	
    if (n < 2 * square) {
		coords.push_back(size - square - square + n);
	}
	else coords.push_back(n - square - square);
	//South +1 
	if (n < size - square - square) {
		coords.push_back(n + square + square);
	}
	else coords.push_back(square + square - size + n);
	return coords;
}

//-----  apply  --------------------------------------------------------------
Population * SelectionCellL9::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population* grid = new Population();
	std::vector<int> coords = getCellsIndex(population->size(), n);
	for (int i = 0; i < coords.size(); i++) {
		if (coords[i] >= 0 && coords[i] < population->size()) {
			grid->addIndividual(population->getIndividual(coords[i])->clone());
		}
	}
	return grid;
}



//=============================================================================
//
//	Class Selection9Cell
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellC9::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellC9::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coords = SelectionCellL5::getCellsIndex(size, n);
	int square = sqrt(size);

	//NorthW
	if (n < square - 1) {
		coords.push_back(size - square + n + 1);
	}
	else if (n == square - 1) {
		coords.push_back(size - square);
	}
	else if ((n + 1) % square == 0 || n == size - 1) {
		coords.push_back(n - square - square + 1);
	}
	else coords.push_back(n - square + 1);
	//SouthW

	if (n == size - 1) {
		coords.push_back(0);
	}
	else if (n > size - square - 1) {
		coords.push_back(n - size + square + 1);
	}
	else if ((n + 1) % square == 0) {
		coords.push_back(n + 1);
	}
	else coords.push_back(n + square + 1);

	//NorthE

	if (n == 0) {
		coords.push_back(size - 1);
	}
	else if (n < square) {
		coords.push_back(size - (square - n) - 1);
	}
	else if (n%square == 0) {
		coords.push_back(n - 1);
	}
	else coords.push_back(n - square - 1);

	//SoutheE
	if (n == 0 || n % square == 0) {
		coords.push_back(n + square + square - 1);
	}
	else if (n > size - square) {
		coords.push_back(n + square - size - 1);
	}
	else coords.push_back(n + square - 1);

	/*
	//NorthW
	if (n < square - 1) {
		coords.push_back(size - square + n + 1);
	}
	else if (n == square - 1) {
		coords.push_back(0);
	}
	else {
		coords.push_back(n - square + 1);
	}
	//SouthW
	if (n < size - square - 1) {
		coords.push_back(n + square + 1);
	}
	else if (n < size - square) {
		coords.push_back(0);
	}
	else {
		coords.push_back(square - size + n + 1);
	}
	//NorthE
	if (n <= square) {
		coords.push_back(size - (square - n) - 1);
	}
	else coords.push_back(n - square - 1);
	//SoutheE
	if (n <= size - square) {
		coords.push_back(n + square - 1);
	}
	else coords.push_back(square - size + n - 1);
	*/
	return coords;
}


//-----  apply  --------------------------------------------------------------
Population * SelectionCellC9::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population* grid = new Population();
	std::vector<int> coords = getCellsIndex(population->size(), n);
	for (int i = 0; i < coords.size(); i++) {
		if (coords[i] >= 0 && coords[i] < population->size()) {
			grid->addIndividual(population->getIndividual(coords[i])->clone());
		}
	}
	return grid;
}

//=============================================================================
//
//	Class Selection13Cell
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//-----  select  --------------------------------------------------------------
Individual * SelectionCellC13::select(Population *population,
	const SharedVars *svars) const {
	int elite_size = svars->parameters->getInteger(ELITE_SIZE);
	int randomValue = svars->rng->getInteger(0, elite_size);
	return population->getIndividual(randomValue);
}

std::vector<int> SelectionCellC13::getCellsIndex(const unsigned int size, const unsigned int n) const {
	std::vector<int> coord1 = SelectionCellC9::getCellsIndex(size, n);
	SelectionCellL9 selL9;
	std::vector<int> coord2 = selL9.getCellsIndex(size, n);
	unsigned int coord1Size = coord1.size();
	for (int i = 0; i < coord2.size(); i++) {
		bool repeated = false;
		for (int j = 0; j < coord1Size; j++) {
			if (coord2[i] == coord1[j]) repeated = true;
		}
		if (!repeated) coord1.push_back(coord2[i]);
	}
	std::cout << coord1.size() << std::endl;
	return coord1;
}


//-----  apply  --------------------------------------------------------------
Population * SelectionCellC13::apply(Population *population, const unsigned int n,
	const SharedVars *svars) const {
	Population* grid = new Population();
	std::vector<int> coords = getCellsIndex(population->size(), n);
	for (int i = 0; i < coords.size(); i++) {
		if (coords[i] >= 0 && coords[i] < population->size()) {
			grid->addIndividual(population->getIndividual(coords[i])->clone());
		}
	}
	return grid;
}

}