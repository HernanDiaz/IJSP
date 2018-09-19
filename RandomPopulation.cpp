/*
* RandomPopulation.cpp
*
*  Created on: Jun 12, 2018
*      Author: jjpalacios
*/

#include "RandomPopulation.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class RandomPopulation
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
RandomPopulation::RandomPopulation(ParameterDB *params)
	: EvolutiveAlgorithm(params) {

	GeneticClassRegister::registerClasses();

	this->creation = NULL;
	this->maxEvaluations = Infi;
	this->maxRuntime = Infi;
	this->evaluations = 0;

	this->totalRuntime = 0;
	this->creationTime = 0;
	this->evaluationTime = 0;
	this->iterationsNI = 0;

	this->bestSoFar = NULL;
}


//-----  Destructor  ----------------------------------------------------------
RandomPopulation::~RandomPopulation() {
	delete this->creation;
	delete this->bestSoFar;
	delete this->diversityStats;
}


//-----  clearAll  ------------------------------------------------------------
void RandomPopulation::clearAll() {
	delete this->creation;
	delete this->bestSoFar;
	this->evolutionStats.clear();
	EvolutiveAlgorithm::clearAll();

	this->maxEvaluations = Infi;
	this->maxRuntime = Infi;
}





//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  printSetupTree  ------------------------------------------------------
void RandomPopulation::printSetupTree(std::ofstream & output) const {
	output << "Random Search" << std::endl;

	output << "Problem to solve:;"
		<< this->sharedVariables->problem->getName() << std::endl;

	std::vector< std::string > names;
	names = this->evaluator->getName();
	output << ";Objective Function:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	output << ";Stopping criteria:" << std::endl;
	output << ";;Max.Iterations without improvement:;";
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
	output << ";Generation of Individuals:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;
}



//-----  getStatistics  -------------------------------------------------------
std::vector< std::pair<std::string, double> > RandomPopulation::getStatistics()
	const {
	std::vector< std::pair<std::string, double> > stats;
	stats.push_back(std::pair<std::string, double>
		("Theoretical Evaluations", this->evaluations));
	stats.push_back(std::pair<std::string, double>
		("Best solution", this->bestSoFar->getFitness()->toDouble()));
	if (this->diversityStats != NULL) {
		this->diversityStats->setStat(Statistics::STAT_BEST);
		stats.push_back(std::pair<std::string, double>
			(this->diversityStats->getName(), this->shortestDistance));
		this->diversityStats->setStat(Statistics::STAT_WORST);
		stats.push_back(std::pair<std::string, double>
			(this->diversityStats->getName(), this->longestDistance));
		this->diversityStats->setStat(Statistics::STAT_AVG);
		stats.push_back(std::pair<std::string, double>
			(this->diversityStats->getName(), this->avgDistance));
		this->diversityStats->setStat(Statistics::STAT_SDEV);
		stats.push_back(std::pair<std::string, double>
			(this->diversityStats->getName(), this->stdevDistance));
	}
	return stats;
}



//-----  getRuntime  ----------------------------------------------------------
std::vector< std::pair<std::string, double> > RandomPopulation::getRuntime()
	const {
	std::vector< std::pair<std::string, double> > times;
	double percentage, totalPerc = 0.0;

	times.push_back(std::pair<std::string, double>
		("Total runtime", (1.0*totalRuntime) / CLOCKS_PER_SEC));

	percentage = (double)creationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Creation", 100 * percentage));
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
std::vector< std::vector<double> > RandomPopulation::getEvolution(
	std::vector< std::string > &labels) const {
	labels.clear();
	labels.push_back("Generation");
	labels.push_back("Runtime");
	labels.push_back("Best");
	for (size_t i = 0; i < this->statistics.size(); i++)
		labels.push_back(this->statistics[i]->getName());
	return this->evolutionStats;
}





//=============================================================================
//		METHODS
//=============================================================================
//-----  prepareToRun  --------------------------------------------------------
void RandomPopulation::prepareToRun(ParameterDB *params) {
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

	// Gets the maximum number of generations without improvement
	this->maxPlateau = params->getInteger(GA_NOIMPROVE, -1);

	// Gets the maximum number of evaluations
	this->maxEvaluations = params->getInteger(GA_EVALUATIONS, -1);

	// Gets the time limit
	this->maxRuntime = params->getDouble(GA_TIME, -1.0);

	// Checks the unit and span to show evolution
	value = params->getStringLower(GA_EVOL_METRIC);
	if (value.compare(GA_EVOL_UNIT_GEN) == 0)
		showEvolutionTime = false;
	else if (value.compare(GA_EVOL_UNIT_TIME) == 0)
		showEvolutionTime = true;
	else {
		std::cout << "WARNING: Unit foe showing evolution values not found.";
		std::cout << " Using generations by default." << std::endl;
	}
	this->evolutionSpan = params->getDouble(GA_EVOL_SPAN, 1);
	
	value = params->getStringLower(RS_DISTANCE_METRIC);
	if (value.length() > 0)
		this->diversityStats = StatisticsClassRegister::getStatsObject(value);
	else
		this->diversityStats = NULL;
	
	// Gets the maximum number of evaluations
	this->maxEvaluations = params->getInteger(GA_EVALUATIONS, -1);

	this->checkSetup();

	this->sharedVariables->encoder->setup(this->sharedVariables->parameters);
	this->sharedVariables->decoder->setup(this->sharedVariables->parameters);
	this->creation->setup(this->sharedVariables->parameters);
}



//-----  checkSetup  ----------------------------------------------------------
bool RandomPopulation::checkSetup() {
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

	if (this->maxEvaluations < 0 && this->maxRuntime < 0 && this->maxPlateau < 0) {
		err = "No stopping criteria defined.";
		correct = false;
	}

	if (!correct) {
		err += " Incorrect value or missing parameter";
		throw new FuzzyFWException("Random Search", err);
	}

	return true;
}



//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> RandomPopulation::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	Population *currentPopulation;
	Individual *newIndividual;
	Fitness *fitness;
	clock_t timePoint, algorithmTime;

	// Initialize dynamic variables
	this->creationTime = 0;
	this->evaluationTime = 0;
	this->iterationsNI = 0;
	evolutionStats.clear();

	this->evaluations = 0;
	this->nextSplit = 0.0;
	this->finished = false;
	currentPopulation = new Population();

	// Initialize the RNG
	this->sharedVariables->rng->init(rngSeed);

	// Set the problem to solve
	this->sharedVariables->problem = problem;

	this->totalRuntime = clock();

	// Create a first individual
	timePoint = clock();
	newIndividual = this->creation->createIndividual(this->sharedVariables);
	currentPopulation->addIndividual(newIndividual);
	this->creationTime = clock() - timePoint;

	timePoint = clock();
	fitness = this->evaluator->evaluate(this->sharedVariables, newIndividual);
	newIndividual->updateFitness(fitness);
	this->evaluations++;
	this->evaluationTime = clock() - timePoint;
	this->totalRuntime = clock() - this->totalRuntime;

	// Statistics....
	if (this->bestSoFar != NULL)
		delete this->bestSoFar;
	this->bestSoFar = newIndividual->clone();
	this->computeStatistics(newIndividual);


	// Start creating individuals
	algorithmTime = clock();
	while (!this->stop()) {
		totalRuntime += clock() - algorithmTime;
		algorithmTime = clock();

		timePoint = clock();
		newIndividual = this->creation->createIndividual(this->sharedVariables);
		currentPopulation->addIndividual(newIndividual);
		this->creationTime += clock() - timePoint;

		timePoint = clock();
		fitness = this->evaluator->evaluate(this->sharedVariables, newIndividual);
		newIndividual->updateFitness(fitness);
		this->evaluations++;
		this->evaluationTime += clock() - timePoint;
		totalRuntime += clock() - algorithmTime;

		// Statistics....
		this->computeStatistics(newIndividual);

		if (newIndividual->getFitness()->isBetterThan(this->bestSoFar->getFitness())) {
			delete this->bestSoFar;
			this->bestSoFar = newIndividual->clone();
			this->iterationsNI = 0;
		}
		else {
			this->iterationsNI++;
		}
		
		algorithmTime = clock();
	}

	if (this->diversityStats != NULL) {
		this->diversityStats->setStat(Statistics::STAT_BEST);
		shortestDistance = this->diversityStats->getValue(this->sharedVariables, currentPopulation);
		this->diversityStats->setStat(Statistics::STAT_WORST);
		longestDistance = this->diversityStats->getValue(this->sharedVariables, currentPopulation);
		this->diversityStats->setStat(Statistics::STAT_AVG);
		avgDistance = this->diversityStats->getValue(this->sharedVariables, currentPopulation);
		this->diversityStats->setStat(Statistics::STAT_SDEV);
		stdevDistance = this->diversityStats->getValue(this->sharedVariables, currentPopulation);
	}

	this->finished = true;
	delete currentPopulation;

	std::pair<Solution *, Objective *> returnValue;
	returnValue.first = this->bestSoFar->getPhenotype()->clone();
	returnValue.second = this->bestSoFar->getFitness()->clone();
	return returnValue;
}



//-----  stop  ----------------------------------------------------------------
bool RandomPopulation::stop() {
	if (this->evaluations >= this->maxEvaluations && this->maxEvaluations >= 0)
		return true;
	if (this->iterationsNI >= this->maxPlateau && this->maxPlateau >= 0)
		return true;

	double currentRuntime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (this->maxRuntime >= 0) {
		if (currentRuntime >= this->maxRuntime)
			return true;

		if (this->evaluations > 0) {
			double runtimePerGen = currentRuntime / this->evaluations;
			if (currentRuntime + runtimePerGen / 2 >= this->maxRuntime)
				return true;
		}
	}

	return false;
}



//-----  compute Statistics  --------------------------------------------------
void RandomPopulation::computeStatistics(Individual *newIndividual) {
	std::vector<double> stats;
	double runtime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (!this->showEvolutionTime && this->evaluations < this->nextSplit)
		return;
	if (this->showEvolutionTime && runtime < this->nextSplit)
		return;

	stats.push_back(this->evaluations);	// Iteration
	stats.push_back(runtime); // Runtime
	stats.push_back(newIndividual->getFitness()->toDouble());	// Current solution

	for (size_t i = 0; i < this->statistics.size(); i++)
		stats.push_back(this->statistics[i]->
			getValue(this->sharedVariables, newIndividual));
	evolutionStats.push_back(stats);

	if (this->showEvolutionTime) {
		while(runtime >= this->nextSplit)
			this->nextSplit += this->evolutionSpan;
	}
	else
		this->nextSplit += this->evolutionSpan;
}

}

