/*
* MemeticAlgorithm.cpp
*
*  Created on: Oct 13, 2015
*      Author: jjpalacios
*/

#include "MemeticAlgorithm.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class MemeticAlgorithm
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
MemeticAlgorithm::MemeticAlgorithm(ParameterDB *params)
	: GeneticAlgorithm(params) {

	LocalSearchClassRegister::registerClasses();

	localSearch = NULL;
	lsFrequency = LS_Frequency::MALS_PERIOD;
	lsPeriod = 1;
	lsTarget = LS_Target::MALS_SOME;
	lsPercentage = 1.0;
	evaluationsLS = 0;
	neighboursLS = 0;
	iterationsLS = 0;
	callsLS = 0;
}


//-----  Destructor  ----------------------------------------------------------
MemeticAlgorithm::~MemeticAlgorithm() {
	delete localSearch;
}


//-----  clearAll  ------------------------------------------------------------
void MemeticAlgorithm::clearAll() {
	GeneticAlgorithm::clearAll();
	delete this->neighbourhood;
	delete this->localSearch;

	lsFrequency = LS_Frequency::MALS_PERIOD;
	lsPeriod = 1;
	lsTarget = LS_Target::MALS_ALL;
	lsPercentage = 1.0;
	evaluationsLS = 0;
	neighboursLS = 0;
	iterationsLS = 0;
	callsLS = 0;
}





//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  printSetupTree  ------------------------------------------------------
void MemeticAlgorithm::printSetupTree(std::ofstream & output) const {
	output << "Memetic Algorithm" << std::endl;

	output << "Problem to solve:;"
		<< this->sharedVariables->problem->getName() << std::endl;

	std::vector< std::string > names;
	names = this->evaluator->getName();
	output << ";Objective Function:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	output << ";Population Size:;" << this->populationSize << std::endl;

	output << ";Stopping criteria:" << std::endl;
	output << ";;Max.Generations:;";
	if (this->maxGenerations < 0) output << "none" << std::endl;
	else output << this->maxGenerations << std::endl;

	output << ";;Max.Generations without improvement:;";
	if (this->maxPlateau < 0) output << "none" << std::endl;
	else output << this->maxPlateau << std::endl;

	output << ";;Max.Evaluations:;";
	if (this->maxEvaluations < 0) output << "none" << std::endl;
	else output << this->maxEvaluations << std::endl;

	output << ";;Time Limit:;";
	if (this->maxRuntime < 0) output << "no" << std::endl;
	else output << this->maxRuntime << std::endl;

	names = this->sharedVariables->encoder->getName();
	output << ";Codification:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->sharedVariables->decoder->getName();
	output << ";Decodification:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->creation->getName();
	output << ";Initial Popul.:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->selection->getName();
	output << ";Selection:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->crossover->getName();
	output << ";Crossover:;" << names[0] << std::endl;
	output << ";;Probability:;" << valueToString(this->crossoverProb) << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->mutation->getName();
	output << ";Mutation:;" << names[0] << std::endl;
	output << ";;Probability:;" << valueToString(this->mutationProb) << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->replacement->getName();
	output << ";Replacement:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->localSearch->getName();
	output << ";Local Search:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->neighbourhood->getName();
	output << ";;Neighbourhood:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";;;" + names[i] << std::endl;

	if (lsFrequency == LS_Frequency::MALS_INITIAL)
		output << ";;Frequency:;" << MA_LS_FREQ_INITIAL << std::endl;
	else if (lsFrequency == LS_Frequency::MALS_FINAL)
		output << ";;Frequency:;" << MA_LS_FREQ_FINAL << std::endl;
	else if (lsFrequency == LS_Frequency::MALS_PERIOD) {
		output << ";;Frequency:; Periodical" << std::endl;
		output << ";;;Iterations:;" << lsPeriod << std::endl;
	}
	else if (lsFrequency == LS_Frequency::MALS_STUCK) {
		output << ";;Frequency:;When stuck";
		output << ";;;Iterations:;" << lsPeriod << std::endl;
	}
	if (lsTarget == LS_Target::MALS_BEST)
		output << ";;Target:;Best solution" << std::endl;
	else if (lsTarget == LS_Target::MALS_WORST)
		output << ";;Target:;Worst solution" << std::endl;
	else if (lsTarget == LS_Target::MALS_ALL)
		output << ";;Target:;All solutions" << std::endl;
	else if (lsTarget == LS_Target::MALS_SOME) {
		output << ";;Target:; Random" << std::endl;
		output << ";;;Percentage:;" << 100 * this->lsPercentage << std::endl;
	}
	if (this->lsLamarckism)
		output << ";;Lamarckism;Yes" << std::endl;
	else
		output << ";;Lamarckism;No" << std::endl;
}



//-----  getStatistics  -------------------------------------------------------
std::vector< std::pair<std::string, double> > MemeticAlgorithm::getStatistics()
	const {
	std::vector< std::pair<std::string, double> > stats;

	stats.push_back(std::pair<std::string, double>
		("Number of Generations", this->generation));
	stats.push_back(std::pair<std::string, double>
		("Evaluations (Genetic)", this->evaluations));
	stats.push_back(std::pair<std::string, double>
		("Evaluations (Local Search)", this->evaluationsLS));
	stats.push_back(std::pair<std::string, double>
		("Evaluations (Total)", this->evaluationsLS + this->evaluations));
	stats.push_back(std::pair<std::string, double>
		("Avg. Neighbours visited per LS",
		(double)this->neighboursLS / this->callsLS));
	stats.push_back(std::pair<std::string, double>
		("Avg. Iterations per LS",
		(double)this->iterationsLS / this->callsLS));
	stats.push_back(std::pair<std::string, double>
		("Best solution", this->bestSoFar->getFitness()->toDouble()));
	return stats;
}



//-----  getRuntime  ----------------------------------------------------------
std::vector< std::pair<std::string, double> > MemeticAlgorithm::getRuntime()
	const {
	std::vector< std::pair<std::string, double> > times;
	double percentage, totalPerc = 0.0;

	times.push_back(std::pair<std::string, double>
		("Total runtime", (1.0*totalRuntime) / CLOCKS_PER_SEC));

	percentage = (double)creationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Creation", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)selectionTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Selection", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)crossoverTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Crossover", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)mutationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Mutation", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)replacementTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Replacement", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)evaluationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Evaluation", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)localSearchTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Local Search", 100 * percentage));
	totalPerc += percentage;

	times.push_back(std::pair<std::string, double>
		("Others", 100 - 100 * totalPerc));
	return times;
}





//=============================================================================
//		METHODS
//=============================================================================
//-----  prepareToRun  --------------------------------------------------------
void MemeticAlgorithm::prepareToRun(ParameterDB *params) {
	// Loads the specific parameters
	std::string value;
	
	// Loads the Local Search strategy to use
	value = params->getStringLower(MA_LOCAL_SEARCH);
	this->localSearch =
		LocalSearchClassRegister::getLocalSearchObject(value);

	// Loads the Neighbourhood structure
	value = params->getStringLower(MA_LOCAL_SEARCH_NEIGHBOURHOOD);
	this->neighbourhood =
		LocalSearchClassRegister::getNeighbourhoodObject(value);
	if (this->neighbourhood == NULL) {
		std::string errorMsg = "Invalid neighbourhood structure";
		errorMsg += " or ommited value.";
		throw new FuzzyFWException("Memetic Algorithm", errorMsg);
	}

	// Loads the frequency to apply local search
	value = params->getStringLower(MA_LOCAL_SEARCH_FREQ);

	this->lsPeriod = 1;
	if (value.compare(MA_LS_FREQ_NONE) == 0)
		this->lsFrequency = LS_Frequency::MALS_NONE;
	else if (value.compare(MA_LS_FREQ_INITIAL) == 0)
		this->lsFrequency = LS_Frequency::MALS_INITIAL;
	else if (value.compare(MA_LS_FREQ_FINAL) == 0)
		this->lsFrequency = LS_Frequency::MALS_FINAL;
	else if (value.compare(MA_LS_FREQ_PERIOD) == 0)
		this->lsFrequency = LS_Frequency::MALS_PERIOD;
	else if (value.compare(MA_LS_FREQ_STUCK) == 0)
		this->lsFrequency = LS_Frequency::MALS_STUCK;
	else {
		this->lsFrequency = LS_Frequency::MALS_NONE;
		this->lsPeriod = -1;
	}

	// Load the period of application, in case of Period or Stuck
	this->lsPeriod = params->getInteger(MA_LOCAL_SEARCH_PERIOD, -2);

	// Loads the target of the local search
	this->lsTarget = LS_Target::MALS_SOME;
	value = params->getStringLower(MA_LOCAL_SEARCH_TARGET);

	if (value.compare(MA_LS_TARGET_BEST) == 0)
		this->lsTarget = LS_Target::MALS_BEST;
	else if (value.compare(MA_LS_TARGET_WORST) == 0)
		this->lsTarget = LS_Target::MALS_WORST;
	else if (value.compare(MA_LS_TARGET_ALL) == 0)
		this->lsTarget = LS_Target::MALS_ALL;
	// It's a numerical value
	else if(value.length() >= 1)
		this->lsPercentage = atof(value.c_str());
	else
		this->lsPercentage = 0.0;


	// Loads the lamarckism flag
	this->lsLamarckism =
		params->getBoolean(MA_LOCAL_SEARCH_LAMARCKISM, true);

	// Loads the common parameters
	GeneticAlgorithm::prepareToRun(params);

	this->neighbourhood->setup(params);
	this->localSearch->setup(params);
	this->localSearch->setNeighbourhood(neighbourhood);
}



//-----  checkSetup  ----------------------------------------------------------
bool MemeticAlgorithm::checkSetup() {
	bool correct = true;
	std::string err = "";

	GeneticAlgorithm::checkSetup();

	if (this->localSearch == NULL) {
		err = "Invalid Local Search algorithm.";
		correct = false;
	}
	if (this->lsPeriod < 0) {
		err = "Invalid Frequency of application of Local Search.";
		correct = false;
	}
	if (this->lsTarget == LS_Target::MALS_SOME
		&& compareDouble(this->lsPercentage, 0.0) <= 0) {
		err = "Invalid target to apply the Local Search to.";
		correct = false;
	}

	if (!correct) {
		err += " Incorrect value or missing parameter";
		throw new FuzzyFWException("Memetic Algorithm", err);
	}

	return true;
}



//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> MemeticAlgorithm::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	Population *currentPopulation;
	Population *offspring;
	clock_t timePoint, algorithmTime;

	// Initialize dynamic variables
	this->creationTime = 0;
	this->selectionTime = 0;
	this->crossoverTime = 0;
	this->mutationTime = 0;
	this->replacementTime = 0;
	this->evaluationTime = 0;
	this->localSearchTime = 0;
	this->iterationsNI = 0;
	evolutionStats.clear();

	this->generation = 0;
	this->evaluations = 0;
	this->nextSplit = 0.0;
	this->evaluationsLS = 0;
	this->neighboursLS = 0;
	this->iterationsLS = 0;
	this->callsLS = 0;
	this->finished = false;

	// Initialize the RNG
	this->sharedVariables->rng->init(rngSeed);

	// Set the problem to solve
	this->sharedVariables->problem = problem;

	this->totalRuntime = clock();

	// Create initial population  -------------------------
	this->creationTime = clock();
	currentPopulation = this->creation->createPopulation(this->populationSize,
		this->sharedVariables);
	this->creationTime = clock() - this->creationTime;


	// Evaluate the initial population  -------------------
	this->evaluationTime = clock();
	this->evaluatePopulation(currentPopulation);
	this->evaluationTime = clock() - this->evaluationTime;

	// Stop counting time for the statistic values and debug mode
	this->totalRuntime = clock() - this->totalRuntime;

	// Statistics....
	if (this->bestSoFar != NULL)
		delete this->bestSoFar;
	this->bestSoFar =
		currentPopulation->getBest(this->sharedVariables)->clone();
	this->computeStatistics(currentPopulation);

#if DEBUG_LEVEL >= 2
	std::cout << "Generation 0:" << std::endl;
	std::cout << "Best fitness: " << this->bestSoFar->getFitness()->toString() << std::endl;
	std::cout << std::endl;
#endif // DEBUG

	// While not stopping criteria is met...
	algorithmTime = clock();
	while (!this->stop()) {
		// Stop counting time for the statistic values and debug mode
		this->totalRuntime += clock() - algorithmTime;

		// Save detailed data...
		if (this->printPopulation &&
			this->generation % this->printPopGenerations == 0)
			DataPrinter::printPopulation(this->generation, signature, logFolder,
				currentPopulation);

		this->generation++;

		algorithmTime = clock();
		// Select individuals for mating  -----------------
		timePoint = clock();
		offspring = this->selection->apply(currentPopulation, this->populationSize,
			this->sharedVariables);
		this->selectionTime += clock() - timePoint;

		// Crossover  -------------------------------------
		timePoint = clock();
		this->crossover->apply(offspring, this->crossoverProb, this->sharedVariables);
		this->crossoverTime += clock() - timePoint;

		// Mutation  --------------------------------------
		timePoint = clock();
		this->mutation->apply(offspring, this->mutationProb, this->sharedVariables);
		this->mutationTime += clock() - timePoint;

		// Evaluation.
		this->evaluatePopulation(offspring);

		// Generational Replacement  ----------------------
		timePoint = clock();
		this->replacement->apply(currentPopulation, offspring,
			this->sharedVariables);
		currentPopulation = offspring;
		this->replacementTime += clock() - timePoint;

		if (currentPopulation->getBest(sharedVariables)->getFitness()
			->isBetterThan(this->bestSoFar->getFitness()))
			this->iterationsNI = 0;
		else
			this->iterationsNI++;

		if (this->bestSoFar != NULL
			&& currentPopulation->getBest(this->sharedVariables)->getFitness()->
			isBetterThan(this->bestSoFar->getFitness())) {
			delete this->bestSoFar;
			this->bestSoFar =
				currentPopulation->getBest(this->sharedVariables)->clone();
		}
		else
			this->bestSoFar =
			currentPopulation->getBest(this->sharedVariables)->clone();

		// Stop counting time for the statistic values and debug mode
		this->totalRuntime += clock() - algorithmTime;

		// Statistics....
		this->computeStatistics(currentPopulation);

#if DEBUG_LEVEL >= 2
		std::cout << "Generation " << this->generation << ":" << std::endl;
		std::cout << "Best fitness: " << this->bestSoFar->getFitness()->toString();
		std::cout << std::endl << std::endl;
#endif // DEBUG
		algorithmTime = clock();
	}

	this->finished = true;

	algorithmTime = clock();
	this->evaluatePopulation(currentPopulation);
	this->totalRuntime += clock() - algorithmTime;
	delete currentPopulation;

	std::pair<Solution *, Objective *> returnValue;
	returnValue.first = this->bestSoFar->getPhenotype()->clone();
	returnValue.second = this->bestSoFar->getFitness()->clone();
	return returnValue;
}



//-----  Evaluate the population  ---------------------------------------------
void MemeticAlgorithm::evaluatePopulation(Population *current) {
	clock_t timePoint;

	timePoint = clock();
	this->evaluator->evaluatePopulation(this->sharedVariables,
		current);
	this->evaluations += current->size();
	this->evaluationTime += clock() - timePoint;

	// Conditions to apply the local search
	timePoint = clock();
	if (this->lsFrequency == LS_Frequency::MALS_INITIAL
		&& this->generation == 0)
		this->applyLocalSearch(current);
	else if (this->lsFrequency == LS_Frequency::MALS_FINAL
		&& this->finished)
		this->applyLocalSearch(current);
	else if (!this->finished &&
		this->lsFrequency == LS_Frequency::MALS_PERIOD
		&& this->generation % this->lsPeriod == 0)
		this->applyLocalSearch(current);
	else if (!this->finished
		&& this->lsFrequency == LS_Frequency::MALS_STUCK
		&& (this->iterationsNI + 1) % this->lsPeriod == 0
		&& current->getBest(this->sharedVariables)->getFitness()
		->isWorseOrEqualTo(this->bestSoFar->getFitness()))
		this->applyLocalSearch(current);
	this->localSearchTime += clock() - timePoint;
}



//-----  Apply the local search to the individuals  ---------------------------
void MemeticAlgorithm::applyLocalSearch(Population *population) {
	std::vector<unsigned int> selection(population->size());
	unsigned int targetIndividuals, chosen, best, position;
	best = NULL;

	if (this->lsTarget == LS_Target::MALS_BEST
		|| this->lsTarget == LS_Target::MALS_SOME) {
		best = chosen = population->whoIsBest(this->sharedVariables);
		this->applyLocalSearch(population, chosen);
	}
	if (this->lsTarget == LS_Target::MALS_WORST) {
		chosen = population->whoIsBest(this->sharedVariables,
			population->size() - 1);
		this->applyLocalSearch(population, chosen);
	}

	if (this->lsTarget == LS_Target::MALS_ALL) {
		for (unsigned int i = 0; i < population->size(); i++) {
			this->applyLocalSearch(population, i);
		}
	}

	if (this->lsTarget == LS_Target::MALS_SOME) {

		// Take the best individual and then random individuals until
		// filling the quota
		for (size_t i = 0; i < selection.size(); i++)
			selection[i] = i;

		// We substract one because it has been applied to the best already
		targetIndividuals = truncateToInteger(
			this->lsPercentage * population->size()) - 1;

		for (unsigned int i = 0; i < targetIndividuals && selection.size() > 0; i++) {
			// Pick a random one
			position = this->sharedVariables->rng->getInteger(0,
				selection.size() - 1);
			chosen = selection[position];
			std::swap(selection[position], selection[selection.size() - 1]);
			selection.pop_back();

			if (chosen == best)
				i--;
			else {
				this->applyLocalSearch(population, i);
			}
		}
	}
}



//-----  Apply the local search to one individual  ----------------------------
void MemeticAlgorithm::applyLocalSearch(Population *population,
	const unsigned int individualIdx) {

	Individual *target;
	FullSolution optimised;

	target = population->getIndividual(individualIdx);
	optimised = this->localSearch->apply(
		target->getPhenotype(), target->getFitness(), this->sharedVariables);

	this->evaluationsLS += this->localSearch->getEvaluations();
	this->neighboursLS += this->localSearch->getNeighbours();
	this->iterationsLS += this->localSearch->getIterations();
	this->callsLS++;

	// Lamarckism
	if (this->lsLamarckism)
		this->sharedVariables->encoder->encode(optimised.first,
			target, this->sharedVariables);

	target->updatePhenotype(optimised.first);
	target->updateFitness(optimised.second);
	population->setSorted(false);
}


}

