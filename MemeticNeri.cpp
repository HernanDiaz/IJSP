/*
* MemeticAlgorithm.cpp
*
*  Created on: Jun 14, 2018
*      Author: jjpalacios
*/

#include "MemeticNeri.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class MemeticNeri
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
MemeticNeri::MemeticNeri(ParameterDB *params)
	: MemeticAlgorithm(params) {

	this->diversityMetric = NULL;
	minPopSize = 0;
	maxPopSize = 0;
	minCrossover = 0.0;
	maxCrossover = 0.0;
	minMutation = 0.0;
	maxMutation = 0.0;
	minLS = 0.0;
	maxLS = 0.0;
}


//-----  clearAll  ------------------------------------------------------------
void MemeticNeri::clearAll() {
	MemeticAlgorithm::clearAll();
	delete this->diversityMetric;
	this->minPopSize = 0;
	this->maxPopSize = 0;
	this->minCrossover = 0.0;
	this->maxCrossover = 0.0;
	this->minMutation = 0.0;
	this->maxMutation = 0.0;
	this->minLS = 0.0;
	this->maxLS = 0.0;
}




//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  printSetupTree  ------------------------------------------------------
void MemeticNeri::printSetupTree(std::ofstream & output) const {
	output << "Neri Memetic Algorithm" << std::endl;

	output << "Problem to solve:;"
		<< this->sharedVariables->problem->getName() << std::endl;

	std::vector< std::string > names;
	names = this->evaluator->getName();
	output << ";Objective Function:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	output << ";Population Size:" << std::endl;
	output << ";;Min:;" << this->minPopSize << std::endl;
	output << ";;Max:;" << this->minPopSize << std::endl;

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

	output << ";Diversity Metric:;" << this->diversityMetric->getName() << std::endl;

	names = this->crossover->getName();
	output << ";Crossover:;" << names[0] << std::endl;
	output << ";;Min.Number:;" << this->minCrossover << std::endl;
	output << ";;Max.Number:;" << this->maxCrossover << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->mutation->getName();
	output << ";Mutation:;" << names[0] << std::endl;
	output << ";;Min.Probability:;" << valueToString(this->minMutation) << std::endl;
	output << ";;Max.Probability:;" << valueToString(this->maxMutation) << std::endl;
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
		output << ";;;Min.Percentage:;" << 100 * this->minLS << std::endl;
		output << ";;;Max.Percentage:;" << 100 * this->maxLS << std::endl;
	}
	if (this->lsLamarckism)
		output << ";;Lamarckism;Yes" << std::endl;
	else
		output << ";;Lamarckism;No" << std::endl;
}



//-----  getStatistics  -------------------------------------------------------
std::vector< std::pair<std::string, double> > MemeticNeri::getStatistics()
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
std::vector< std::pair<std::string, double> > MemeticNeri::getRuntime()
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

	percentage = (double)diversityTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Diversity", 100 * percentage));
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
void MemeticNeri::prepareToRun(ParameterDB *params) {
	// Loads the specific parameters
	std::string value;

	// Loads the minimum/maximum population sizes
	this->minPopSize = params->getInteger(NERI_POP_MINSIZE);
	this->maxPopSize = params->getInteger(NERI_POP_MAXSIZE);

	// Loads the minimum/maximum number of crossovers
	this->minCrossover = params->getInteger(NERI_CROSS_MIN);
	this->maxCrossover = params->getInteger(NERI_CROSS_MAX);

	// Loads the minimum/maximum number of crossovers
	this->minMutation = params->getDouble(NERI_MUTATION_MIN);
	this->maxMutation = params->getDouble(NERI_MUTATION_MAX);

	// Loads the minimum/maximum number of crossovers
	this->minLS = params->getDouble(NERI_LS_MIN);
	this->maxLS = params->getDouble(NERI_LS_MAX);

	// Gets the diversity metric
	value = params->getString(NERI_DIVERSITY_METRIC);
	this->diversityMetric = StatisticsClassRegister::getStatsObject(value);

	// Loads the common parameters
	MemeticAlgorithm::prepareToRun(params);

	//this->checkSetup();
}



//-----  checkSetup  ----------------------------------------------------------
bool MemeticNeri::checkSetup() {
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
	if (this->diversityMetric == NULL) {
		err = "Invalid diversity metric.";
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

	if (this->minCrossover < 0 || this->maxCrossover < this->minCrossover) {
		err = "Invalid range for number of crossovers.";
		correct = false;
	}

	if (compareDouble(this->minMutation, 0.0) < 0
		|| compareDouble(this->maxMutation, 1.0) > 0
		|| compareDouble(this->minMutation, this->maxMutation) > 0) {
		err = "Invalid range of mutation probabilities.";
		correct = false;
	}

	if (this->maxGenerations < 0 && this->maxEvaluations < 0
		&& this->maxRuntime < 0 && this->maxPlateau < 0) {
		err = "No stopping criteria defined.";
		correct = false;
	}

	if (this->minPopSize < 0 || this->maxPopSize < this->minPopSize) {
		err = "Invalid range od population sizes.";
		correct = false;
	}
	
	if (this->localSearch == NULL) {
		err = "Invalid Local Search algorithm.";
		correct = false;
	}
	if (this->lsPeriod < 0) {
		err = "Invalid Frequency of application of Local Search.";
		correct = false;
	}
	
	if (this->lsTarget == MemeticAlgorithm::LS_Target::MALS_SOME &&
		(compareDouble(this->minLS, 0.0) < 0
		|| compareDouble(this->maxLS, 1.0) > 0
		|| compareDouble(this->minLS, this->maxLS) > 0)) {
		err = "Invalid range of Local Search probabilities.";
		correct = false;
	}

	if (!correct) {
		err += " Incorrect value or missing parameter";
		throw new FuzzyFWException("Neri Algorithm", err);
	}

	return true;
}



//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> MemeticNeri::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	Population *currentPopulation;
	Population *offspring;
	clock_t timePoint, algorithmTime;
	unsigned int numCrossovers;
	double diversity;
	

	// Initialize dynamic variables
	this->creationTime = 0;
	this->selectionTime = 0;
	this->crossoverTime = 0;
	this->mutationTime = 0;
	this->diversityTime = 0;
	this->replacementTime = 0;
	this->evaluationTime = 0;
	this->localSearchTime = 0;
	this->iterationsNI = 0;
	evolutionStats.clear();

	this->crossoverProb = 1.0;
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
	currentPopulation = this->creation->createPopulation(this->minPopSize,
		this->sharedVariables);
	this->creationTime = clock() - this->creationTime;


	// Evaluate the initial population  -------------------
	this->evaluationTime = clock();
	this->evaluatePopulation(currentPopulation);
	this->evaluationTime = clock() - this->evaluationTime;

	// Compute diversity and update probabilities  --------
	this->diversityTime = clock();
	diversity = this->diversityMetric->getValue(this->sharedVariables, currentPopulation);
	numCrossovers = truncateToInteger(currentPopulation->size() *
		(this->maxCrossover - diversity * (this->maxCrossover - this->minCrossover)));
	this->mutationProb = this->maxMutation - diversity * (this->maxMutation - this->minMutation);
	this->populationSize = truncateToInteger(this->maxPopSize - diversity * (this->maxPopSize - this->minPopSize));
	this->lsPercentage = this->minLS + diversity * (this->maxLS - this->minLS);
	this->diversityTime = clock() - this->diversityTime;

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
		if(this->populationSize % 2 == 0)
			offspring = this->selection->apply(currentPopulation, this->populationSize,
				this->sharedVariables);
		else {
			offspring = this->selection->apply(currentPopulation, this->populationSize+1,
				this->sharedVariables);
		}
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

		this->diversityTime = clock();
		diversity = this->diversityMetric->getValue(this->sharedVariables, currentPopulation);
		numCrossovers = truncateToInteger(currentPopulation->size() *
			(this->maxCrossover - diversity * (this->maxCrossover - this->minCrossover)));
		this->mutationProb = this->maxMutation - diversity * (this->maxMutation - this->minMutation);
		this->populationSize = this->maxPopSize - diversity * (this->maxPopSize - this->minPopSize);
		this->lsPercentage = this->minLS + diversity * (this->maxLS - this->minLS);
		this->diversityTime = clock() - this->diversityTime;

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
void MemeticNeri::evaluatePopulation(Population *current) {
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
void MemeticNeri::applyLocalSearch(Population *population) {
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
void MemeticNeri::applyLocalSearch(Population *population,
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

