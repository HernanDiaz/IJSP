/*
* GeneticAlgorithm.cpp
*
*  Created on: Sep 15, 2015
*/

#include "GeneticAlgorithm.h"

namespace FuzzyFW {

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

	this->crossoverProb = 0.0;
	this->mutationProb = 0.0;
	
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

	this->maxGenerations = Infi;
	this->maxEvaluations = Infi;
	this->maxRuntime = Infi;
	this->populationSize = 0;
	this->crossoverProb = 0.0;
	this->mutationProb = 0.0;
	this->printPopulation = false;
	this->printPopGenerations = 0;
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
	for (size_t i = 0; i < this->statistics.size(); i++)
		labels.push_back(this->statistics[i]->getName());
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
	value = params->getStringLower(GA_ENCODING);
	this->sharedVariables->encoder = GeneticClassRegister::getEncoderObject(value);

	// Loads the decoding strategy
	value = params->getStringLower(GA_DECODING);
	this->sharedVariables->decoder = GeneticClassRegister::getDecoderObject(value);

	// Gets the creation strategy
	value = params->getStringLower(GA_CREATION);
	this->creation = GeneticClassRegister::getCreationObject(value);

	// Gets the crossover operator
	value = params->getStringLower(GA_CROSSOVER);
	this->crossover = GeneticClassRegister::getCrossoverObject(value);

	// Gets the crossover probability
	this->crossoverProb = params->getDouble(GA_CROSSOVER_PROB, -1.0);

	// Gets the mutation operator
	value = params->getStringLower(GA_MUTATION);
	this->mutation = GeneticClassRegister::getMutationObject(value);

	// Gets the mutation probability
	this->mutationProb = params->getDouble(GA_MUTATION_PROB, -1.0);

	// Gets the selection operator
	value = params->getStringLower(GA_SELECTION);
	this->selection = GeneticClassRegister::getSelectionObject(value);

	// Gets the replacement strategy
	value = params->getStringLower(GA_REPLACE);
	this->replacement = GeneticClassRegister::getReplacementObject(value);

	// Gets the maximum number of generations
	this->maxGenerations = params->getInteger(GA_GENERATIONS, -1);

	// Gets the maximum number of generations without improvement
	this->maxPlateau = params->getInteger(GA_NOIMPROVE, -1);

	// Gets the maximum number of evaluations
	this->maxEvaluations = params->getInteger(GA_EVALUATIONS, -1);

	// Gets the time limit
	this->maxRuntime = params->getDouble(GA_TIME, -1.0);

	// Gets the population size
	this->populationSize = params->getInteger(GA_POP_SIZE, -1);
	this->printPopGenerations = params->getInteger(GA_POP_INTERVAL, 1);


	// Checks if the populations must be printed
	this->printPopulation = params->getBoolean(GA_PRINT_POPULATION, false);
	this->printPopGenerations = params->getInteger(GA_POP_INTERVAL, 1);

	// Checks the unit and span to show evolution
	value = params->getStringLower(GA_EVOL_METRIC);
	if (value.compare(GA_EVOL_UNIT_GEN) == 0)
		showEvolutionTime = false;
	else if (value.compare(GA_EVOL_UNIT_TIME) == 0)
		showEvolutionTime = true;
	else {
		std::cout << "WARNING: Unit foe showing evolution values not found.";
		std::cout << " Using generations by default." << std::endl;
		showEvolutionTime = false;  // default: generations mode
	}
	this->evolutionSpan = params->getDouble(GA_EVOL_SPAN, 1);
	
	// Gets the maximum number of evaluations
	this->maxEvaluations = params->getInteger(GA_EVALUATIONS, -1);

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

	if (compareDouble(this->crossoverProb, 0.0) < 0
		|| compareDouble(this->crossoverProb, 1.0) > 0) {
		err = "Invalid crossover probability.";
		correct = false;
	}

	if (compareDouble(this->mutationProb, 0.0) < 0
		|| compareDouble(this->mutationProb, 1.0) > 0) {
		err = "Invalid mutation probability.";
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
		throw new FuzzyFWException("Genetic Algorithm", err);
	}

	return true;
}



//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> GeneticAlgorithm::run(Problem *problem,
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
	this->iterationsNI = 0;
	evolutionStats.clear();

	this->generation = 0;
	this->evaluations = 0;
	this->nextSplit = 0.0;
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
	this->evaluator->evaluatePopulation(this->sharedVariables, currentPopulation);
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
	delete currentPopulation;

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

	double currentRuntime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (this->maxRuntime >= 0) {
		if (currentRuntime >= this->maxRuntime)
			return true;

		if (this->generation > 0) {
			double runtimePerGen = currentRuntime / this->generation;
			if (currentRuntime + runtimePerGen / 2 >= this->maxRuntime)
				return true;
		}
	}

	return false;
}



//-----  compute Statistics  --------------------------------------------------
void GeneticAlgorithm::computeStatistics(Population *currentPopulation) {
	std::vector<double> stats;
	double runtime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (!this->showEvolutionTime && this->generation < this->nextSplit)
		return;
	if (this->showEvolutionTime && runtime < this->nextSplit)
		return;

	stats.push_back(this->generation);	// Iteration
	stats.push_back(runtime); // Runtime

	stats.push_back(this->bestSoFar->getFitness()->toDouble());	// Best solution
	stats.push_back(currentPopulation->getAverageFitness()); // Average quality


	for (size_t i = 0; i < this->statistics.size(); i++)
		stats.push_back(this->statistics[i]->
			getValue(this->sharedVariables, currentPopulation));
	evolutionStats.push_back(stats);

	if (this->showEvolutionTime) {
		while(runtime >= this->nextSplit)
			this->nextSplit += this->evolutionSpan;
	}
	else
		this->nextSplit += this->evolutionSpan;
}

}

