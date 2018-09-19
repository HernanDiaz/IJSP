/*
* GeneticAlgorithm.cpp
*
*  Created on: Sep 15, 2015
*      Author: jjpalacios
*/

#include "GeneticAlgorithm.h"

namespace FJSP {

//=============================================================================
//
//	Class GeneticAlgorithm
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
GeneticAlgorithm::GeneticAlgorithm(ParameterDB *params)
	: EvolutiveAlgorithm(params) {

	GeneticClassRegister::registerClasses();

	this->creation = NULL;
	this->selection = NULL;
	this->crossover = NULL;
	this->mutation = NULL;
	this->replacement = NULL;
	
	this->maxGenerations = Infi;
	this->maxEvaluations = Infi;
	this->maxRuntime = Infi;
	this->populationSize = 0;
	this->printPopulation = false;
	this->printPopGenerations = 0;

	this->generation = 0;
	this->evaluations = 0;

	this->totalRuntime = 0;
	this->creationTime = 0;
	this->selectionTime = 0;
	this->crossoverTime = 0;
	this->mutationTime = 0;
	this->replacementTime = 0;
	this->evaluationTime = 0;
	this->iterationsNI = 0;

	this->bestSoFar = NULL;
}


//-----  Destructor  ----------------------------------------------------------
GeneticAlgorithm::~GeneticAlgorithm() {
	delete this->creation;
	delete this->selection;
	delete this->crossover;
	delete this->mutation;
	delete this->replacement;
	delete this->bestSoFar;
}


//-----  clearAll  ------------------------------------------------------------
void GeneticAlgorithm::clearAll() {
	delete this->creation;
	delete this->selection;
	delete this->crossover;
	delete this->mutation;
	delete this->replacement;
	delete this->bestSoFar;
	this->evolutionStats.clear();
	EvolutiveAlgorithm::clearAll();
}





//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  printSetupTree  ------------------------------------------------------
void GeneticAlgorithm::printSetupTree(std::ofstream & output) const {
	output << "Genetic Algorithm" << std::endl;

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
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->mutation->getName();
	output << ";Mutation:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->replacement->getName();
	output << ";Replacement:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;
}



//-----  getStatistics  -------------------------------------------------------
std::vector< std::pair<std::string, double> > GeneticAlgorithm::getStatistics()
	const {
	std::vector< std::pair<std::string, double> > stats;
	stats.push_back(std::pair<std::string, double>
		("Number of Generations", this->generation));
	stats.push_back(std::pair<std::string, double>
		("Theoretical Evaluations", this->evaluations));
	stats.push_back(std::pair<std::string, double>
		("Best solution", this->bestSoFar->getFitness()->toDouble()));
	return stats;
}



//-----  getRuntime  ----------------------------------------------------------
std::vector< std::pair<std::string, double> > GeneticAlgorithm::getRuntime()
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

	times.push_back(std::pair<std::string, double>
		("Others", 100 - 100 * totalPerc));
	return times;
}



//-----  getEvolution  ----------------------------------------------------------
std::vector< std::vector<double> > GeneticAlgorithm::getEvolution(
	std::vector< std::string > &labels) const {
	labels.clear();
	labels.push_back("Generation");
	labels.push_back("Runtime");
	labels.push_back("Best");
	labels.push_back("Avg");
	return this->evolutionStats;
}





//=============================================================================
//		METHODS
//=============================================================================
//-----  prepareToRun  --------------------------------------------------------
void GeneticAlgorithm::prepareToRun(ParameterDB *params) {
	// Loads the common parameters
	EvolutiveAlgorithm::prepareToRun(params);

	// Loads the specific parameters
	std::string value;

	// Loads the encoding strategy
	value = this->sharedVariables->parameters->getString(GA_ENCODING);
	this->sharedVariables->encoder = GeneticClassRegister::getEncoderObject(value);

	// Loads the decoding strategy
	value = this->sharedVariables->parameters->getString(GA_DECODING);
	this->sharedVariables->decoder = GeneticClassRegister::getDecoderObject(value);

	// Gets the creation strategy
	value = this->sharedVariables->parameters->getString(GA_CREATION);
	this->creation = GeneticClassRegister::getCreationObject(value);

	// Gets the crossover operator
	value = this->sharedVariables->parameters->getString(GA_CROSSOVER);
	this->crossover = GeneticClassRegister::getCrossoverObject(value);

	// Gets the mutation operator
	value = this->sharedVariables->parameters->getString(GA_MUTATION);
	this->mutation = GeneticClassRegister::getMutationObject(value);

	// Gets the selection operator
	value = this->sharedVariables->parameters->getString(GA_SELECTION);
	this->selection = GeneticClassRegister::getSelectionObject(value);

	// Gets the replacement strategy
	value = this->sharedVariables->parameters->getString(GA_REPLACE);
	this->replacement = GeneticClassRegister::getReplacementObject(value);

	// Gets the maximum number of generations
	this->maxGenerations = this->sharedVariables->parameters->
		getInteger(GA_GENERATIONS, -1);

	// Gets the maximum number of generations without improvement
	this->maxPlateau = this->sharedVariables->parameters->
		getInteger(GA_NOIMPROVE, -1);

	// Gets the maximum number of evaluations
	this->maxEvaluations = this->sharedVariables->parameters->
		getInteger(GA_EVALUATIONS, -1);

	// Gets the time limit
	this->maxRuntime = this->sharedVariables->parameters->
		getDouble(GA_TIME, -1.0);

	// Gets the population size
	this->populationSize = this->sharedVariables->parameters->
		getInteger(GA_POP_SIZE, -1);

	// Checks if the populations must be printed
	this->printPopulation = this->sharedVariables->parameters->
		getBoolean(GA_PRINT_POPULATION, false);
	this->printPopGenerations = this->sharedVariables->parameters->
		getInteger(GA_POP_INTERVAL, 1);

	this->checkSetup();

	this->sharedVariables->encoder->setup(this->sharedVariables->parameters);
	this->sharedVariables->decoder->setup(this->sharedVariables->parameters);
	this->creation->setup(this->sharedVariables->parameters);
	this->selection->setup(this->sharedVariables->parameters);
	this->crossover->setup(this->sharedVariables->parameters);
	this->mutation->setup(this->sharedVariables->parameters);
	this->replacement->setup(this->sharedVariables->parameters);
}



//-----  checkSetup  ----------------------------------------------------------
bool GeneticAlgorithm::checkSetup() {
	bool correct = true;
	std::string err = "";
	if (this->sharedVariables->encoder == NULL) {
		err = "Invalid codification method.";
		correct = false;
	}
	if (this->sharedVariables->decoder == NULL) {
		err = "Invalid decodification method.";
		correct = false;
	}
	if (this->creation == NULL) {
		err = "Invalid creation strategy.";
		correct = false;
	}
	if (this->crossover == NULL) {
		err = "Invalid crossover operator.";
		correct = false;
	}
	if (this->mutation == NULL) {
		err = "Invalid mutation operator.";
		correct = false;
	}
	if (this->replacement == NULL) {
		err = "Invalid replacement operator.";
		correct = false;
	}
	if (this->selection == NULL) {
		err = "Invalid selection operator.";
		correct = false;
	}

	if (this->maxGenerations < 0 && this->maxEvaluations < 0
		&& this->maxRuntime < 0 && this->maxPlateau < 0) {
		err = "No stopping criteria defined.";
		correct = false;
	}

	if (this->populationSize < 0) {
		err = "Invalid population size.";
		correct = false;
	}

	if (!correct) {
		err += " Incorrect value or missing parameter";
		throw new FJSPException("Genetic Algorithm", err);
	}

	return true;
}



//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> GeneticAlgorithm::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	Population *currentPopulation;
	std::vector<double> stats;
	clock_t timePoint;

	// Initialize dynamic variables
	this->creationTime = 0;
	this->selectionTime = 0;
	this->crossoverTime = 0;
	this->mutationTime = 0;
	this->replacementTime = 0;
	this->evaluationTime = 0;
	this->iterationsNI = 0;
	evolutionStats.clear();

	this->generation = 0;
	this->evaluations = 0;

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
	this->evaluator->evaluatePopulation(this->sharedVariables, currentPopulation);
	this->evaluationTime = clock() - this->evaluationTime;

	// Statistics....
	if (this->bestSoFar != NULL)
		delete this->bestSoFar;
	this->bestSoFar =
		currentPopulation->getBest(this->sharedVariables)->clone();

	stats.push_back(this->generation);	// Iteration
	stats.push_back(
		(clock() - this->totalRuntime) / (double)CLOCKS_PER_SEC); // Runtime
	stats.push_back(this->bestSoFar->getFitness()->toDouble());	// Best solution
	stats.push_back(currentPopulation->getAverageFitness()); // Average quality
	evolutionStats.push_back(stats);

#if DEBUG
	std::cout << "Generation 0:" << std::endl;
	std::cout << "Best fitness: " << this->bestSoFar->getFitness() << std::endl;
#endif // DEBUG

	// While not stopping criteria is met...
	Population *offspring;
	while (!this->stop()) {
		// Save detailed data...
		if (this->printPopulation &&
			this->generation % this->printPopGenerations == 0)
			Statistics::printPopulation(this->generation, signature, logFolder,
				currentPopulation);

		// Select individuals for mating  -----------------
		timePoint = clock();
		offspring = this->selection->apply(currentPopulation,
			this->sharedVariables);
		this->selectionTime += clock() - timePoint;

		// Crossover  -------------------------------------
		timePoint = clock();
		this->crossover->apply(offspring, this->sharedVariables);
		this->crossoverTime += clock() - timePoint;

		// Mutation  --------------------------------------
		timePoint = clock();
		this->mutation->apply(offspring, this->sharedVariables);
		this->mutationTime += clock() - timePoint;

		// Evaluation.
		timePoint = clock();
		this->evaluator->evaluatePopulation(this->sharedVariables,
			offspring);
		this->evaluations += currentPopulation->size();
		this->evaluationTime += clock() - timePoint;

		// Generational Replacement  ----------------------
		timePoint = clock();
		this->replacement->apply(currentPopulation, offspring,
			this->sharedVariables);
		currentPopulation = offspring;
		this->replacementTime += clock() - timePoint;

		this->generation++;
		if (currentPopulation->getBest(sharedVariables)->getFitness()
			->isBetterThan(this->bestSoFar->getFitness()))
			this->iterationsNI = 0;
		else
			this->iterationsNI++;

		// Statistics....
		stats.clear();
		delete this->bestSoFar;
		this->bestSoFar = currentPopulation->
			getBest(this->sharedVariables)->clone();
		stats.push_back(this->generation);	// Iteration
		stats.push_back(
			(clock() - this->totalRuntime) / (double)CLOCKS_PER_SEC); // Runtime
		stats.push_back(this->bestSoFar->getFitness()->toDouble());	// Best solution
		stats.push_back(currentPopulation->getAverageFitness()); // Average quality
		evolutionStats.push_back(stats);
#if DEBUG
		std::cout << "Generation " << this->generation << ":" << std::endl;
		std::cout << "Best fitness: " << this->bestSoFar->getFitness();
		std::cout << std::endl << std::endl;
#endif // DEBUG
	}

	this->totalRuntime = clock() - this->totalRuntime;
	this->finished = true;

	std::pair<Solution *, Objective *> returnValue;
	returnValue.first = this->bestSoFar->getPhenotype()->clone();
	returnValue.second = this->bestSoFar->getFitness()->clone();
	return returnValue;
}



//-----  stop  ----------------------------------------------------------------
bool GeneticAlgorithm::stop() {
	if (this->generation >= this->maxGenerations && this->maxGenerations >= 0)
		return true;
	if (this->evaluations >= this->maxEvaluations && this->maxEvaluations >= 0)
		return true;
	if (this->iterationsNI >= this->maxPlateau && this->maxPlateau >= 0)
		return true;

	clock_t clockTime = clock() - this->totalRuntime;
	double currentRuntime = clockTime / (double)CLOCKS_PER_SEC;

	if (this->maxRuntime >= 0 && currentRuntime >= this->maxRuntime)
		return true;

	if (this->generation > 0) {
		double runtimePerGen = currentRuntime / this->generation;
		if (currentRuntime + runtimePerGen / 2 >= this->maxRuntime)
			return true;
	}

	return false;
}


}

