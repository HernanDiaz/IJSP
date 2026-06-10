/*
 * IPRTS.cpp
 *
 *  Created on: Jun 10, 2026
 *      Author: hdiaz
 */

#include "IPRTS.h"

namespace FuzzyFW {

//=============================================================================
//
//	Class IPRTS
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
IPRTS::IPRTS(ParameterDB *params)
	: EvolutiveAlgorithm(params) {

	GeneticClassRegister::registerClasses();
	LocalSearchClassRegister::registerClasses();

	this->creation = NULL;
	this->localSearch = NULL;
	this->lsLamarckism = true;
	this->lsMaxRounds = 1;
	this->pathRelinking = NULL;

	this->poolSize = 0;
	this->poolMinDistance = 0.0;
	this->restartPatience = -1;
	this->cyclesSinceInsertion = 0;
	this->seedRetries = 0;

	this->maxCycles = -1;
	this->maxPlateau = -1;
	this->maxEvaluations = -1;
	this->maxRuntime = -1.0;

	this->showEvolutionTime = false;
	this->evolutionSpan = 1.0;

	this->cycles = 0;
	this->cyclesNI = 0;
	this->evaluations = 0;
	this->prSteps = 0;
	this->prCalls = 0;
	this->lsCalls = 0;
	this->poolInsertions = 0;
	this->poolRestarts = 0;
	this->bestSoFar = NULL;

	this->totalRuntime = 0;
	this->creationTime = 0;
	this->localSearchTime = 0;
	this->pathRelinkingTime = 0;
	this->nextSplit = 0.0;
}


//-----  Destructor  ----------------------------------------------------------
IPRTS::~IPRTS() {
	delete this->creation;
	delete this->localSearch;
	delete this->pathRelinking;
	delete this->bestSoFar;
}


//-----  clearAll  ------------------------------------------------------------
void IPRTS::clearAll() {
	delete this->creation;
	this->creation = NULL;
	delete this->localSearch;
	this->localSearch = NULL;
	delete this->pathRelinking;
	this->pathRelinking = NULL;
	delete this->bestSoFar;
	this->bestSoFar = NULL;
	this->pool.clear();
	this->evolutionStats.clear();
	EvolutiveAlgorithm::clearAll();

	this->maxCycles = -1;
	this->maxPlateau = -1;
	this->maxEvaluations = -1;
	this->maxRuntime = -1.0;
	this->poolSize = 0;
}



//=============================================================================
//		GET/SET METHODS
//=============================================================================
//-----  printSetupTree  ------------------------------------------------------
void IPRTS::printSetupTree(std::ofstream &output) const {
	output << "Interval Path Relinking Tabu Search" << std::endl;

	output << "Problem to solve:;"
		<< this->sharedVariables->problem->getName() << std::endl;

	std::vector< std::string > names;
	names = this->evaluator->getName();
	output << ";Objective Function:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	output << ";Pool Size:;" << this->poolSize << std::endl;
	output << ";;Min. Distance:;"
		<< valueToString(this->poolMinDistance) << std::endl;
	output << ";;Restart Patience:;"
		<< valueToString(this->restartPatience) << std::endl;

	output << ";Stopping criteria:" << std::endl;
	output << ";;Max.Cycles:;";
	if (this->maxCycles < 0) output << "none" << std::endl;
	else output << this->maxCycles << std::endl;

	output << ";;Max.Cycles without improvement:;";
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
	output << ";Pool Seeding:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->pathRelinking->getName();
	output << ";Path Relinking:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;

	names = this->localSearch->getName();
	output << ";Local Search:;" << names[0] << std::endl;
	for (int i = 1; i < (int)names.size(); i++)
		output << ";" + names[i] << std::endl;
}


//-----  getStatistics  -------------------------------------------------------
std::vector< std::pair<std::string, double> > IPRTS::getStatistics()
	const {
	std::vector< std::pair<std::string, double> > stats;
	stats.push_back(std::pair<std::string, double>
		("Number of Cycles", this->cycles));
	stats.push_back(std::pair<std::string, double>
		("Theoretical Evaluations", (double)this->evaluations));
	stats.push_back(std::pair<std::string, double>
		("PR Calls", (double)this->prCalls));
	stats.push_back(std::pair<std::string, double>
		("PR Steps", (double)this->prSteps));
	stats.push_back(std::pair<std::string, double>
		("Pool Insertions", (double)this->poolInsertions));
	stats.push_back(std::pair<std::string, double>
		("Pool Restarts", (double)this->poolRestarts));
	stats.push_back(std::pair<std::string, double>
		("Best solution", this->bestSoFar->getFitness()->toDouble()));
	return stats;
}


//-----  getRuntime  ----------------------------------------------------------
std::vector< std::pair<std::string, double> > IPRTS::getRuntime()
	const {
	std::vector< std::pair<std::string, double> > times;
	double percentage, totalPerc = 0.0;

	times.push_back(std::pair<std::string, double>
		("Total runtime", (1.0 * totalRuntime) / CLOCKS_PER_SEC));

	percentage = (double)creationTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Creation", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)localSearchTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Local Search", 100 * percentage));
	totalPerc += percentage;

	percentage = (double)pathRelinkingTime / totalRuntime;
	times.push_back(std::pair<std::string, double>
		("Path Relinking", 100 * percentage));
	totalPerc += percentage;

	times.push_back(std::pair<std::string, double>
		("Others", 100 - 100 * totalPerc));
	return times;
}


//-----  getEvolution  --------------------------------------------------------
std::vector< std::vector<double> > IPRTS::getEvolution(
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
void IPRTS::prepareToRun(ParameterDB *params) {
	// Loads the common parameters (evaluator and statistics)
	EvolutiveAlgorithm::prepareToRun(params);

	ParameterDB *p = this->sharedVariables->parameters;
	std::string value;

	// Loads the encoding and decoding strategies
	value = p->getStringLower(IPRTS_ENCODING);
	this->sharedVariables->encoder = GeneticClassRegister::getEncoderObject(value);

	value = p->getStringLower(IPRTS_DECODING);
	this->sharedVariables->decoder = GeneticClassRegister::getDecoderObject(value);

	// Gets the creation strategy used for pool seeding
	value = p->getStringLower(IPRTS_CREATION);
	this->creation = GeneticClassRegister::getCreationObject(value);

	// Loads the Local Search strategy and its neighbourhood
	value = p->getStringLower(IPRTS_LOCAL_SEARCH);
	this->localSearch = LocalSearchClassRegister::getLocalSearchObject(value);

	value = p->getStringLower(IPRTS_LS_NEIGHBOURHOOD);
	Neighbourhood *neighbourhood =
		LocalSearchClassRegister::getNeighbourhoodObject(value);
	if (neighbourhood == NULL) {
		std::string errorMsg = "Invalid neighbourhood structure";
		errorMsg += " or ommited value.";
		throw FuzzyFWException("IPRTS", errorMsg);
	}

	this->lsLamarckism = p->getBoolean(IPRTS_LS_LAMARCKISM, true);
	this->lsMaxRounds = p->getInteger(IPRTS_LS_ROUNDS, 1);

	// Path relinking operator
	this->pathRelinking = new IJSP::PathRelinkIJSP();

	// Pool parameters
	this->poolSize = p->getInteger(POOL_SIZE, 10);
	this->poolMinDistance = p->getDouble(POOL_MIN_DISTANCE, 0.05);
	this->restartPatience = p->getInteger(IPRTS_RESTART_PATIENCE, 50);
	this->seedRetries = p->getInteger(IPRTS_SEED_RETRIES, 5);

	// Stopping criteria
	this->maxCycles = p->getInteger(IPRTS_CYCLES, -1);
	this->maxPlateau = p->getInteger(IPRTS_NOIMPROVE, -1);
	this->maxEvaluations = p->getInteger(IPRTS_EVALUATIONS, -1);
	this->maxRuntime = p->getDouble(IPRTS_TIME, -1.0);

	// Checks the unit and span to show evolution
	value = p->getStringLower(IPRTS_EVOL_METRIC);
	if (value.compare(IPRTS_EVOL_UNIT_GEN) == 0)
		this->showEvolutionTime = false;
	else if (value.compare(IPRTS_EVOL_UNIT_TIME) == 0)
		this->showEvolutionTime = true;
	else {
		std::cout << "WARNING: Unit foe showing evolution values not found.";
		std::cout << " Using generations by default." << std::endl;
		this->showEvolutionTime = false;
	}
	this->evolutionSpan = p->getDouble(IPRTS_EVOL_SPAN, 1);

	this->checkSetup();

	this->sharedVariables->encoder->setup(p);
	this->sharedVariables->decoder->setup(p);
	this->creation->setup(p);
	neighbourhood->setup(p);
	this->localSearch->setup(p);
	this->localSearch->setNeighbourhood(neighbourhood);
	this->pathRelinking->setup(p);
}


//-----  checkSetup  ----------------------------------------------------------
bool IPRTS::checkSetup() {
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
	if (this->localSearch == NULL) {
		err = "Invalid local search strategy.";
		correct = false;
	}
	if (this->poolSize < 2) {
		err = "Invalid pool size (must be at least 2).";
		correct = false;
	}
	if (this->poolMinDistance < 0.0 || this->poolMinDistance >= 1.0) {
		err = "Invalid pool minimum distance.";
		correct = false;
	}
	if (this->maxCycles < 0 && this->maxEvaluations < 0
		&& this->maxRuntime < 0 && this->maxPlateau < 0) {
		err = "No stopping criteria defined.";
		correct = false;
	}

	if (!correct) {
		err += " Incorrect value or missing parameter";
		throw FuzzyFWException("IPRTS", err);
	}

	return true;
}


//-----  RUN !!  --------------------------------------------------------------
//=============================================================================
std::pair<Solution *, Objective *> IPRTS::run(Problem *problem,
	std::string signature, std::string logFolder, int rngSeed) {

	clock_t runStart, timePoint;

	// Initialize dynamic variables
	this->cycles = 0;
	this->cyclesNI = 0;
	this->evaluations = 0;
	this->prSteps = 0;
	this->prCalls = 0;
	this->lsCalls = 0;
	this->poolInsertions = 0;
	this->poolRestarts = 0;
	this->cyclesSinceInsertion = 0;
	this->nextSplit = 0.0;
	this->finished = false;
	this->evolutionStats.clear();

	this->totalRuntime = 0;
	this->creationTime = 0;
	this->localSearchTime = 0;
	this->pathRelinkingTime = 0;

	// Initialize the RNG
	this->sharedVariables->rng->init(rngSeed);

	// Set the problem to solve
	this->sharedVariables->problem = problem;

	runStart = clock();

	// Seed the elite pool  -------------------------------
	this->pool.configure(this->poolSize, this->poolMinDistance);
	this->fillPool();
	this->totalRuntime = clock() - runStart;

	if (this->bestSoFar != NULL)
		delete this->bestSoFar;
	this->bestSoFar = this->pool.get(this->pool.bestIndex())->clone();
	this->computeStatistics();

	// Main loop  -----------------------------------------
	while (!this->stop()) {
		// Pick the relinking pair: guide must be the better one
		Individual *from, *guide;
		unsigned int i = this->sharedVariables->rng
			->getInteger(0, this->pool.size() - 1);
		unsigned int j = this->sharedVariables->rng
			->getInteger(0, this->pool.size() - 2);
		if (j >= i)
			j++;
		from = this->pool.get(i);
		guide = this->pool.get(j);
		if (from->getFitness()->isBetterThan(guide->getFitness()))
			std::swap(from, guide);

		const IJSP::ScheduleIJSP *guideSchedule =
			dynamic_cast<const IJSP::ScheduleIJSP *>(guide->getPhenotype());

		// Relink towards the guide  ----------------------
		timePoint = clock();
		FullSolution intermediate = this->pathRelinking->relink(
			from->getPhenotype(), from->getFitness(), guideSchedule,
			this->sharedVariables);
		this->prCalls++;
		this->prSteps += this->pathRelinking->getSteps();
		this->evaluations += this->pathRelinking->getEvaluations();
		this->pathRelinkingTime += clock() - timePoint;

		// Wrap the intermediate and re-decode it through the SGS (the
		// insertion SGS repairs and improves the ordering, exactly as it
		// does for crossover offspring in the population algorithms)
		Individual *candidate = from->clone();
		this->sharedVariables->encoder->encode(intermediate.first, candidate,
			this->sharedVariables);
		Solution *redecoded = this->sharedVariables->decoder
			->decode(candidate, this->sharedVariables);
		candidate->updatePhenotype(redecoded->clone());
		candidate->updateFitness(
			this->evaluator->evaluate(this->sharedVariables, candidate));
		this->evaluations++;
		delete intermediate.first;
		delete intermediate.second;

		// Improve it with tabu search  --------------------
		this->applyLocalSearch(candidate);

		// Best-so-far and plateau bookkeeping  ------------
		if (candidate->getFitness()->isBetterThan(this->bestSoFar->getFitness())) {
			delete this->bestSoFar;
			this->bestSoFar = candidate->clone();
			this->cyclesNI = 0;
		}
		else
			this->cyclesNI++;

		// Pool update (the pool owns the candidate now)  --
		if (this->pool.tryInsert(candidate)) {
			this->poolInsertions++;
			this->cyclesSinceInsertion = 0;
		}
		else
			this->cyclesSinceInsertion++;

		// Stagnation restart (scout phase analogue)  ------
		if (this->restartPatience >= 0
			&& (int)this->cyclesSinceInsertion >= this->restartPatience) {
			this->pool.dropWorstHalf();
			this->fillPool();
			this->poolRestarts++;
			this->cyclesSinceInsertion = 0;
		}

		this->cycles++;
		this->totalRuntime = clock() - runStart;
		this->computeStatistics();
	}

	this->finished = true;

	std::pair<Solution *, Objective *> returnValue;
	returnValue.first = this->bestSoFar->getPhenotype()->clone();
	returnValue.second = this->bestSoFar->getFitness()->clone();
	return returnValue;
}


//-----  stop  ----------------------------------------------------------------
bool IPRTS::stop() {
	if (this->maxCycles >= 0 && (int)this->cycles >= this->maxCycles)
		return true;
	if (this->maxEvaluations >= 0
		&& (long int)this->evaluations >= this->maxEvaluations)
		return true;
	if (this->maxPlateau >= 0 && (int)this->cyclesNI >= this->maxPlateau)
		return true;

	double currentRuntime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (this->maxRuntime >= 0) {
		if (currentRuntime >= this->maxRuntime)
			return true;

		if (this->cycles > 0) {
			double runtimePerCycle = currentRuntime / this->cycles;
			if (currentRuntime + runtimePerCycle / 2 >= this->maxRuntime)
				return true;
		}
	}

	return false;
}


//-----  applyLocalSearch  ----------------------------------------------------
void IPRTS::applyLocalSearch(Individual *individual) {
	clock_t timePoint = clock();
	FullSolution optimised;

	// Chain rounds: each apply() runs a fresh tabu chain from the previous
	// result; keep going while the rounds improve the individual
	int round = 0;
	do {
		optimised = this->localSearch->apply(individual->getPhenotype(),
			individual->getFitness(), this->sharedVariables);

		this->evaluations += this->localSearch->getEvaluations();
		this->lsCalls++;
		round++;

		const bool improved =
			optimised.second->isBetterThan(individual->getFitness());

		individual->updatePhenotype(optimised.first);
		individual->updateFitness(optimised.second);

		if (!improved)
			break;
	} while (this->lsMaxRounds < 0 || round < this->lsMaxRounds);

	if (this->lsLamarckism)
		this->sharedVariables->encoder->encode(individual->getPhenotype(),
			individual, this->sharedVariables);

	this->localSearchTime += clock() - timePoint;
}


//-----  newSeedIndividual  ---------------------------------------------------
Individual * IPRTS::newSeedIndividual() {
	clock_t timePoint = clock();

	Individual *seed = this->creation->createIndividual(this->sharedVariables);
	seed->updateFitness(this->evaluator->evaluate(this->sharedVariables, seed));
	this->evaluations++;

	this->creationTime += clock() - timePoint;
	return seed;
}


//-----  fillPool  ------------------------------------------------------------
void IPRTS::fillPool() {
	int tries = 0;
	const int maxTries = this->seedRetries
		* (int)(this->poolSize - this->pool.size());

	while (this->pool.size() < this->poolSize && tries < maxTries) {
		Individual *seed = this->newSeedIndividual();
		this->applyLocalSearch(seed);
		this->pool.tryInsert(seed);
		tries++;
	}

	// Diversity could not be honoured (e.g. tiny instances): relax it
	while (this->pool.size() < this->poolSize) {
		Individual *seed = this->newSeedIndividual();
		this->applyLocalSearch(seed);
		this->pool.forceInsert(seed);
	}
}


//-----  computeStatistics  ---------------------------------------------------
void IPRTS::computeStatistics() {
	std::vector<double> stats;
	double runtime = this->totalRuntime / (double)CLOCKS_PER_SEC;

	if (!this->showEvolutionTime && this->cycles < this->nextSplit)
		return;
	if (this->showEvolutionTime && runtime < this->nextSplit)
		return;

	stats.push_back(this->cycles);	// Iteration
	stats.push_back(runtime);	// Runtime
	stats.push_back(this->bestSoFar->getFitness()->toDouble());	// Best
	stats.push_back(this->pool.averageFitness());	// Pool average

	this->evolutionStats.push_back(stats);

	if (this->showEvolutionTime) {
		while (runtime >= this->nextSplit)
			this->nextSplit += this->evolutionSpan;
	}
	else
		this->nextSplit += this->evolutionSpan;
}

}
