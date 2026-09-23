/*
* MemeticAlgorithm.cpp
*
*  Created on: Oct 13, 2022
*      Author: hdiaz
*/

#include "ArtificialBeeColonyPSO.h"
#include "LS_Tabu.h"
#include <iostream>
#include <set>


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
	ArtificialBeeColonyPSO::ArtificialBeeColonyPSO(ParameterDB *params)
		: GeneticAlgorithm(params) {

		LocalSearchClassRegister::registerClasses();
		NonMonSimulatedCoolingClassRegister::registerClasses();

		localSearch = NULL;
		lsFrequency = LS_Frequency::MALS_PERIOD;
		lsPeriod = 1;
		lsTarget = LS_Target::MALS_SOME;
		lsPercentage = 1.0;
		evaluationsLS = 0;
		neighboursLS = 0;
		iterationsLS = 0;
		callsLS = 0;
		improvementsLS = 0;
		enworstmentsLS = 0;
		neutralLS = 0;
	}


	//-----  Destructor  ----------------------------------------------------------
	ArtificialBeeColonyPSO::~ArtificialBeeColonyPSO() {
		delete localSearch;
	}


	//-----  clearAll  ------------------------------------------------------------
	void ArtificialBeeColonyPSO::clearAll() {
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
		improvementsLS = 0;
		enworstmentsLS = 0;
		neutralLS = 0;
	}





	//=============================================================================
	//		GET/SET METHODS
	//=============================================================================
	//-----  printSetupTree  ------------------------------------------------------
	void ArtificialBeeColonyPSO::printSetupTree(std::ofstream & output) const {
		output << "Artificial Bee Colony PSO" << std::endl;

		output << "Problem to solve:;"
			<< this->sharedVariables->problem->getName() << std::endl;

		std::vector< std::string > names;
		names = this->evaluator->getName();
		output << ";Objective Function:;" << names[0] << std::endl;
		for (int i = 1; i < (int)names.size(); i++)
			output << ";" + names[i] << std::endl;

		output << ";Population Size:;" << this->populationSize << std::endl;

		output << "; Maximum Trials per food source:;" << this->sharedVariables->parameters->getInteger(MAX_NUM_TRIALS) << std::endl;

		output << "; Elite selection:;" << this->sharedVariables->parameters->getInteger(ELITE_SELECTION) << std::endl;

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

		if (this->simulatedCooling) {
			names = this->simulatedCooling->getName();
			output << "; Simulated Cooling : ; "<< std::endl; 
			output << ";; Non Monotonic Cooling:;" << names[0] << std::endl;
			output << ";; Monotonic Cooling:;" << names[1] << std::endl;
			for (int i = 2; i < (int)names.size(); i+=2)
				output << ";;;" + names[i] <<";"<<names[i+1]<<std::endl;
		}
	}


	//-----  getStatistics  -------------------------------------------------------
	std::vector< std::pair<std::string, double> > ArtificialBeeColonyPSO::getStatistics()
		const {
		std::vector< std::pair<std::string, double> > stats;

		stats.push_back(std::pair<std::string, double>
			("Number of Generations", this->generation));
		stats.push_back(std::pair<std::string, double>
			("Evaluations (ABC)", this->evaluations));
		stats.push_back(std::pair<std::string, double>
			("Evaluations (Local Search)", this->evaluationsLS));
		stats.push_back(std::pair<std::string, double>
			("Evaluations (Total)", this->evaluationsLS + this->evaluations));
		stats.push_back(std::pair<std::string, double>
			("Avg. Neighbours visited per LS",
			(double)this->neighboursLS / this->callsLS));
		stats.push_back(std::pair<std::string, double>
			("Number of Calls to LS",
			(double) this->callsLS));
		stats.push_back(std::pair<std::string, double>
			("Number of Solutions improved by LS",
			(double)this->improvementsLS));
		stats.push_back(std::pair<std::string, double>
			("Avg of solutions improved by LS",
			(double)this->improvementsLS/this->callsLS));
		stats.push_back(std::pair<std::string, double>
			("Number of Solutions worsened by LS",
			(double)this->enworstmentsLS));
		stats.push_back(std::pair<std::string, double>
			("Avg of solutions worsened by LS",
			(double)this->enworstmentsLS / this->callsLS));
		stats.push_back(std::pair<std::string, double>
			("Number of Solutions equal fitness after LS",
			(double)this->neutralLS));
		stats.push_back(std::pair<std::string, double>
			("Avg of solutions equal fitness after LS",
			(double)this->neutralLS / this->callsLS));
		stats.push_back(std::pair<std::string, double>
			("Avg. Iterations per LS",
			(double)this->iterationsLS / this->callsLS));
		stats.push_back(std::pair<std::string, double>
			("Avg replacements per generation",
			(double)this->abc_replacements / this->generation));
		stats.push_back(std::pair<std::string, double>
			("Total replacements in ABC",
			(double)this->abc_replacements));
		stats.push_back(std::pair<std::string, double>
			("N2 ties at best per tabu iteration",
			LS_Tabu::diagIters ? (double)LS_Tabu::diagTieSum / LS_Tabu::diagIters : 0.0));
		stats.push_back(std::pair<std::string, double>
			("N2 largest tie seen", (double)LS_Tabu::diagTieMax));
		stats.push_back(std::pair<std::string, double>
			("N2 neighbours evaluated", (double)LS_Tabu::diagScanned));
		stats.push_back(std::pair<std::string, double>
			("N2 neighbours offered", (double)LS_Tabu::diagGenerated));
		stats.push_back(std::pair<std::string, double>
			("N2 fraction of neighbourhood scanned %",
			LS_Tabu::diagGenerated
				? 100.0 * (double)LS_Tabu::diagScanned / (double)LS_Tabu::diagGenerated
				: 0.0));
		stats.push_back(std::pair<std::string, double>
			("N2 neighbours offered per tabu iteration",
			LS_Tabu::diagIters
				? (double)LS_Tabu::diagGenerated / (double)LS_Tabu::diagIters
				: 0.0));
		stats.push_back(std::pair<std::string, double>
			("N2 estimate above real value", (double)LS_Tabu::diagBoundBreaks));
		stats.push_back(std::pair<std::string, double>
			("Stall restarts", (double)this->stallRestarts));
		stats.push_back(std::pair<std::string, double>
			("Plateau moves admitted in crossover", (double)this->plateauAdmittedCross));
		stats.push_back(std::pair<std::string, double>
			("Plateau moves admitted in local search", (double)this->plateauAdmittedLS));
		stats.push_back(std::pair<std::string, double>
			("N2 moves by first improvement", (double)LS_Tabu::firstHits));
		stats.push_back(std::pair<std::string, double>
			("N2 sweeps with no improving move", (double)LS_Tabu::fallbackHits));
		stats.push_back(std::pair<std::string, double>
			("N2 share of moves by first improvement %",
			(LS_Tabu::firstHits + LS_Tabu::fallbackHits)
				? 100.0 * (double)LS_Tabu::firstHits
					/ (double)(LS_Tabu::firstHits + LS_Tabu::fallbackHits)
				: 0.0));
		stats.push_back(std::pair<std::string, double>
			("Deep LS calls", (double)this->deepLsCalls));
		stats.push_back(std::pair<std::string, double>
			("Deep iters per call",
			LS_Tabu::deepCalls ? (double)LS_Tabu::deepIters / LS_Tabu::deepCalls : 0.0));
		stats.push_back(std::pair<std::string, double>
			("Deep stops: dead end", (double)LS_Tabu::deepDeadEnd));
		stats.push_back(std::pair<std::string, double>
			("Deep escapes from all-tabu", (double)LS_Tabu::deepEscapes));
		stats.push_back(std::pair<std::string, double>
			("Deep stops: counter", (double)LS_Tabu::deepBadStop));
		stats.push_back(std::pair<std::string, double>
			("Deep stops: time cap", (double)LS_Tabu::deepTimeStop));
		stats.push_back(std::pair<std::string, double>
			("Deep LS share of LS time %",
			this->localSearchTime > 0
				? 100.0 * (double)this->deepLsTime / (double)this->localSearchTime
				: 0.0));
		stats.push_back(std::pair<std::string, double>
			("Plateau vetoes in crossover",
			(double)this->plateauVetoesCross));
		stats.push_back(std::pair<std::string, double>
			("Plateau vetoes in local search",
			(double)this->plateauVetoesLS));
		stats.push_back(std::pair<std::string, double>
			("Improvements kept by local search",
			(double)this->improvementsLS));
		stats.push_back(std::pair<std::string, double>
			("Best solution", this->bestSoFar->getFitness()->toDouble()));
		return stats;
	}



	//-----  getRuntime  ----------------------------------------------------------
	std::vector< std::pair<std::string, double> > ArtificialBeeColonyPSO::getRuntime()
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
	void ArtificialBeeColonyPSO::prepareToRun(ParameterDB *params) {
		// Loads the specific parameters
		std::string value;

		// Loads the Local Search strategy to use
		value = params->getStringLower(MA_LOCAL_SEARCH);
		this->localSearch =
			LocalSearchClassRegister::getLocalSearchObject(value);

		//Loads the simulated cooling structure
		value = params->getStringLower(NON_MONOTONIC_COOLING_STRATEGY);
		this->simulatedCooling =
			NonMonSimulatedCoolingClassRegister::getNMACObject(value);
		if (this->simulatedCooling) {
			this->simulatedCooling->setUp(params);
		}

		// Loads the Neighbourhood structure
		value = params->getStringLower(MA_LOCAL_SEARCH_NEIGHBOURHOOD);
		this->neighbourhood =
			LocalSearchClassRegister::getNeighbourhoodObject(value);
		if (this->neighbourhood == NULL) {
			std::string errorMsg = "Invalid neighbourhood structure";
			errorMsg += " or ommited value.";
			throw FuzzyFWException("Memetic Algorithm", errorMsg);
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
		else if (value.length() >= 1)
			this->lsPercentage = atof(value.c_str());
		else
			this->lsPercentage = 0.0;


		// Loads the lamarckism flag
		this->lsLamarckism =
			params->getBoolean(MA_LOCAL_SEARCH_LAMARCKISM, true);

		// Loads the common parameters
		// Scout phase. Default is the classical ABC, a fresh random solution, so
		// a setup that says nothing behaves exactly as before.
		this->deepLsTrigger = (unsigned int)params->getInteger(DEEP_LS_TRIGGER, 0);
		this->deepLsShare = (unsigned int)params->getInteger(DEEP_LS_SHARE, 0);
		// I-014: restart the population once the stall is old enough that the
		// measured hazard of a further improvement is zero.
		this->stallRestart =
			(params->getStringLower(STALL_RESTART).compare("stall") == 0);
		// I-015: keep improvements that land exactly on the incumbent's value.
		this->plateauAllow =
			(params->getStringLower(PLATEAU_MODE).compare("allow") == 0);
		this->scoutKick = false;
		this->scoutKicks = 0;
		std::string scoutValue = params->getStringLower(SCOUT_MODE);
		if (scoutValue.compare(SCOUT_MODE_KICK) == 0) {
			this->scoutKick = true;
			this->scoutKicks = (unsigned int)params->getInteger(SCOUT_KICKS, 3);
			if (this->scoutKicks == 0)
				throw FuzzyFWException("Artificial Bee Colony PSO",
					"abc.scout = kick needs abc.scout.kicks greater than zero");
		}

		GeneticAlgorithm::prepareToRun(params);

		this->neighbourhood->setup(params);
		this->localSearch->setup(params);
		this->localSearch->setNeighbourhood(neighbourhood);
	}



	//-----  checkSetup  ----------------------------------------------------------
	bool ArtificialBeeColonyPSO::checkSetup() {
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
			throw FuzzyFWException("Memetic Algorithm", err);
		}

		return true;
	}

	struct cmp {
		bool operator() (Fitness* a, Fitness* b) const {
			return a->isWorseThan(b);
		}
	};


	//-----  RUN !!  --------------------------------------------------------------
	//=============================================================================
	std::pair<Solution *, Objective *> ArtificialBeeColonyPSO::run(Problem *problem,
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
		this->abc_replacements = 0;
		this->plateauVetoesCross = 0;
		this->plateauVetoesLS = 0;
		this->plateauAdmittedCross = 0;
		this->plateauAdmittedLS = 0;
		this->deepLsDone = false;
		this->deepLsCalls = 0;
		this->deepLsTime = 0;
		this->lastImprovementSec = 0.0;
		this->stallRestarts = 0;

		evolutionStats.clear();

		this->generation = 0;
		this->evaluations = 0;
		this->nextSplit = 0.0;
		this->evaluationsLS = 0;
		this->neighboursLS = 0;
		this->iterationsLS = 0;
		this->callsLS = 0;
		this->improvementsLS = 0;
		this->enworstmentsLS = 0;
		this->neutralLS = 0;
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

		// Evaluate the initial population  -------------------
		this->evaluationTime = clock();
		this->evaluatePopulation(currentPopulation);
		this->evaluationTime = clock() - this->evaluationTime;

		if (this->bestSoFar != NULL)
			delete this->bestSoFar;
		this->bestSoFar = currentPopulation->getBest(this->sharedVariables)->clone();

		// Local search to replacement population
		this->creationTime = clock() - this->creationTime;
		timePoint = clock();

		if (this->lsFrequency == LS_Frequency::MALS_INITIAL) {
			this->applyLocalSearch(currentPopulation);
		}
		// Stop counting time for the statistic values and debug mode
		this->totalRuntime = clock() - this->totalRuntime;

		// Statistics....

		this->computeStatistics(currentPopulation);

#if DEBUG_LEVEL >= 2
		std::cout << "Generation 0:" << std::endl;
		std::cout << "Best fitness: " << this->bestSoFar->getFitness()->toString() << std::endl;
		std::cout << std::endl;
#endif // DEBUG


		algorithmTime = clock();

		std::set<Fitness*, cmp> cross_indiv;

		// While not stopping criteria is met...
		while (!this->stop()) {
			// Stop counting time for the statistic values and debug mode
			this->totalRuntime += clock() - algorithmTime;

			// Save detailed data...
			if (this->printPopulation &&
				this->generation % this->printPopGenerations == 0)
				DataPrinter::printPopulation(this->generation, signature, logFolder,
					currentPopulation);

			algorithmTime = clock();
			//Employed bee phase
			Individual* bestFoodSource = this->bestSoFar->clone();

			int elite_value = this->sharedVariables->parameters->getInteger(ELITE_SELECTION);
			int elite_size = this->sharedVariables->parameters->getInteger(ELITE_SIZE);

			// Wall-clock start for timelimit enforcement within a generation
			struct timespec _wallGenStart;
			clock_gettime(CLOCK_MONOTONIC, &_wallGenStart);

			for (int i = 0; i < currentPopulation->size(); i++) {
				// Wall-clock timelimit check inside the generation loop
				if (this->maxRuntime >= 0) {
					struct timespec _wallNow;
					clock_gettime(CLOCK_MONOTONIC, &_wallNow);
					double _elapsed = (_wallNow.tv_sec - _wallGenStart.tv_sec)
						+ (_wallNow.tv_nsec - _wallGenStart.tv_nsec) * 1e-9;
					if (_elapsed >= this->maxRuntime)
						break;
				}
				//Coger de forma aleatoria uno de los 20 primeros como best food source
				timePoint = clock();
				unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, elite_size);
				this->evaluator->evaluatePopulation(this->sharedVariables, currentPopulation, false);
				currentPopulation->sort(this->sharedVariables->rng, true);
				if (bestFoodSource != NULL) {
					delete bestFoodSource;
				}
				if (foodSourceID == 0) {
					bestFoodSource = this->bestSoFar->clone();
				}
				else {
					bestFoodSource = currentPopulation->getBest(this->sharedVariables, foodSourceID - 1)->clone();
				}
				this->selectionTime += clock() - timePoint;
				timePoint = clock();
				Population currentFoodSources;
				Individual* currentFoodSource = currentPopulation->getIndividual(i);
				Individual* destiny = currentPopulation->getIndividual(i)->clone();
				this->mutation->apply(destiny, this->sharedVariables);
				this->mutationTime += clock() - timePoint;
				timePoint = clock();
				currentFoodSources.addIndividual(bestFoodSource->clone());
				currentFoodSources.addIndividual(destiny->clone());
				if (destiny != NULL) {
					delete destiny;
				}
				this->crossover->apply(&currentFoodSources, this->crossoverProb, this->sharedVariables);
				this->evaluator->evaluatePopulation(this->sharedVariables, &currentFoodSources, false);
				// Conditions to apply the local search
				this->crossoverTime += clock() - timePoint;
				timePoint = clock();

				if (this->lsFrequency == LS_Frequency::MALS_PERIOD
					|| this->lsFrequency == LS_Frequency::MALS_INITIAL
					&& this->generation % this->lsPeriod == 0)
				{
					this->applyLocalSearch(&currentFoodSources);
				}
				this->localSearchTime += clock() - timePoint;
				timePoint = clock();
				this->evaluator->evaluatePopulation(this->sharedVariables, &currentFoodSources, false);
				Individual* bestLocal = currentFoodSources.getBest(this->sharedVariables);
				//If the best local food source is better than the currentFoodSource we replace it
				// INSTRUMENTED 2026-09-21: how often does the second clause, the
				// plateau veto, decide this branch on its own?
				if (bestLocal->getFitness()->isBetterThan(currentFoodSource->getFitness())
					&& bestLocal->getFitness()->isEqualTo(this->bestSoFar->getFitness())) {
					if (this->plateauAllow) this->plateauAdmittedCross++;
					else this->plateauVetoesCross++;
				}
				// I-015: with abc.plateau = allow the tie with the incumbent no
				// longer vetoes the replacement.
				if (bestLocal->getFitness()->isBetterThan(currentFoodSource->getFitness())
					&& (this->plateauAllow
						|| !bestLocal->getFitness()->isEqualTo(this->bestSoFar->getFitness()))) {
					Individual* bestlocalClone = bestLocal->clone();
					delete currentPopulation->replaceIndividual(i, bestlocalClone);
					bestlocalClone->setNumTrials(0);
					currentFoodSource = bestlocalClone; // Fix: avoid dangling pointer after replace

				}
				else if (this->simulatedCooling != NULL
					&& this->simulatedCooling->isSelected(currentFoodSource, bestLocal, this->sharedVariables, this->generation + 1)) {
					Individual* bestlocalClone = bestLocal->clone();
					delete currentPopulation->replaceIndividual(i, bestlocalClone);
					bestlocalClone->setNumTrials(0);
					currentFoodSource = bestlocalClone; // Fix: avoid dangling pointer after replace
					}
				else {
					currentFoodSource->setNumTrials(currentFoodSource->getNumTrials() + 1);
				}

				if (currentFoodSource->getNumTrials() >= this->sharedVariables->parameters->getInteger(MAX_NUM_TRIALS)) {
					// INSTRUMENTED 2026-09-21. This is the abandonment of an
					// exhausted food source, and it is the only one in this class.
					// The inherited counter abc_replacements was initialised here
					// and never incremented, so the statistic "Total replacements
					// in ABC" read 0 in every run and measured nothing at all.
					this->abc_replacements++;
					// I-003. The classical ABC injects a fresh random solution
					// here, and this fires 496 times per run on ta29 and 815 on
					// ta41 (measured 2026-09-21). I-001 measured that a random
					// start is 294 makespan units worse at generation 0 and that
					// 0.3 % of that gap survives to the end of a run, so each of
					// those hundreds of injections lands outside every basin the
					// population occupies, with no budget left to catch up.
					// abc.scout = kick replaces it with a clone of a random elite
					// under abc.scout.kicks mutations: the same abandonment, but
					// restarting a trajectory inside a promising basin.
					Population* newPopulation;
					if (this->scoutKick) {
						int eliteCount =
							this->sharedVariables->parameters->getInteger(ELITE_SIZE);
						if (eliteCount < 1) eliteCount = 1;
						unsigned int pick =
							this->sharedVariables->rng->getInteger(0, eliteCount);
						Individual *kicked =
							currentPopulation->getBest(this->sharedVariables, pick)->clone();
						kicked->setNumTrials(0);
						for (unsigned int k = 0; k < this->scoutKicks; k++)
							this->mutation->apply(kicked, this->sharedVariables);
						kicked->id = 0;
						newPopulation = new Population();
						newPopulation->addIndividual(kicked);
					}
					else
						newPopulation = this->creation->createPopulation(1, this->sharedVariables);
					this->evaluator->evaluatePopulation(this->sharedVariables, newPopulation, false);

					if (this->lsFrequency == LS_Frequency::MALS_INITIAL) {
						this->applyLocalSearch(newPopulation);
					}
					delete currentPopulation->replaceIndividual(i, newPopulation->getBest(this->sharedVariables, 0));
					newPopulation->clear(false);
					delete newPopulation;
					this->evaluator->evaluatePopulation(this->sharedVariables, currentPopulation, true);
				}
				this->replacementTime += clock() - timePoint;
			}
			this->generation++;

			double nowSec =
				(this->totalRuntime + (clock() - algorithmTime))
					/ (double)CLOCKS_PER_SEC;
			if (currentPopulation->getBest(sharedVariables)->getFitness()
				->isBetterThan(this->bestSoFar->getFitness())) {
				this->iterationsNI = 0;
				this->lastImprovementSec = nowSec;
				// A new episode may fire again.
				if (this->deepLsShare > 0) this->deepLsDone = false;
			}
			else
				this->iterationsNI++;

			// I-014. The run has gone STALL_RESTART_SHARE of its budget with
			// no new global best, and at that age the measured hazard of one
			// arriving is zero. Rebuild the population from the creation
			// operator, carry the incumbent into it so nothing is lost, and
			// spend the rest of the budget searching somewhere else. The
			// budget, the number of runs and the endpoint are untouched.
			if (this->stallRestart && this->maxRuntime > 0
				&& nowSec - this->lastImprovementSec
					>= STALL_RESTART_SHARE * this->maxRuntime) {
				// Around the incumbent, not from scratch. A cold population
				// cannot catch up in what the budget leaves after a stall,
				// measured on 2026-09-23 and the reason the cold form was
				// withdrawn: +4.5 to +22 at 40 % of the budget, +27 to +55 at
				// 20 %. Every individual here starts at the incumbent's
				// quality, so what is injected is diversity, not a handicap.
				Population *fresh = new Population();
				fresh->addIndividual(this->bestSoFar->clone());
				for (unsigned int i = 1; i < this->populationSize; i++) {
					Individual *kicked = this->bestSoFar->clone();
					kicked->setNumTrials(0);
					kicked->id = (int)i;
					unsigned int kicks = 1 + (i % 10);
					for (unsigned int k = 0; k < kicks; k++)
						this->mutation->apply(kicked, this->sharedVariables);
					fresh->addIndividual(kicked);
				}
				this->evaluator->evaluatePopulation(
					this->sharedVariables, fresh, true);
				if (this->lsFrequency == LS_Frequency::MALS_INITIAL)
					this->applyLocalSearch(fresh);
				delete currentPopulation;
				currentPopulation = fresh;
				this->lastImprovementSec = nowSec;
				this->iterationsNI = 0;
				this->stallRestarts++;
			}

			// I-009. The local search of this solver is about 242 shallow dips
			// per generation, each ending after 15 non-improving iterations
			// having made 26 to 41 moves in total (measured 2026-09-21). There
			// is never ONE deep trajectory. irace swept the depth between 5
			// and 40 and chose 15 (H-5), but in its space one depth applied to
			// the whole population, so the uneven allocation was never
			// explored. This fires a single deep call on the incumbent the
			// first time the run stalls, and never again in that run, so the
			// cost is bounded by the existing per-call time cap.
			// With a share declared, the QUOTA is the only limiter: fire on
			// every stagnant generation and let the time budget stop it. The
			// once-per-episode gate was measured on 2026-09-22 to cap the
			// mechanism at 0.03 % of local-search time, because episodes are
			// few (1.3 on ta29, 7.7 on ta41 per run) while the quota asks for
			// 25 %. Limiting opportunities and limiting cost are different
			// things, and only the second one belongs here.
			bool deepAllowed = this->deepLsTrigger > 0
				&& this->iterationsNI >= this->deepLsTrigger
				&& (this->deepLsShare > 0 || !this->deepLsDone);
			if (deepAllowed && this->deepLsShare > 0) {
				// Keep the deep calls under their declared share of all
				// local-search time, so the mechanism gets a real slice of the
				// search and the cost limits itself.
				double spent = (double)this->deepLsTime;
				double allLS = (double)this->localSearchTime;
				if (allLS > 0.0
					&& spent >= allLS * (double)this->deepLsShare / 100.0)
					deepAllowed = false;
			}
			if (deepAllowed) {
				LS_Tabu *tabu = dynamic_cast<LS_Tabu *>(this->localSearch);
				if (tabu != NULL) {
					int saved = tabu->getMaxBadIterations();
					tabu->setMaxBadIterations(DEEP_LS_DEPTH);
					clock_t before = clock();
					this->applyLocalSearch(currentPopulation,
						currentPopulation->whoIsBest(this->sharedVariables));
					this->deepLsTime += clock() - before;
					tabu->setMaxBadIterations(saved);
					this->deepLsCalls++;
				}
				this->deepLsDone = true;
			}

			if (this->bestSoFar != NULL) {
				if (currentPopulation->getBest(this->sharedVariables)->getFitness()->isBetterThan(this->bestSoFar->getFitness())) {
					delete this->bestSoFar;
					this->bestSoFar = currentPopulation->getBest(this->sharedVariables)->clone();
				}
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
	void ArtificialBeeColonyPSO::evaluatePopulation(Population *current) {
		clock_t timePoint;

		timePoint = clock();
		this->evaluator->evaluatePopulation(this->sharedVariables,
			current);
		this->evaluations += current->size();
		this->evaluationTime += clock() - timePoint;
		/*
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
		*/
	}
	//TODO
	/*
	bool isSelected(const SharedVarsEvolutionary * sharedVariables, const Individual*origin,cont Individual*destiny,const int generation) {
		return (this->sharedVariables->rng->getInteger(0, 150) < (150 - generation));
	}*/

	//-----  Apply the local search to the individuals  ---------------------------
	void ArtificialBeeColonyPSO::applyLocalSearch(Population *population) {
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
	void ArtificialBeeColonyPSO::applyLocalSearch(Population *population,
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

		// INSTRUMENTED 2026-09-21: the same veto inside the Lamarckian write-back.
		// When the tabu search improves an individual to exactly the incumbent
		// makespan, the improvement is discarded and a trial failure is counted.
		if (optimised.second->isBetterThan(target->getFitness())
			&& optimised.second->isEqualTo(this->bestSoFar->getFitness())) {
			if (this->plateauAllow) this->plateauAdmittedLS++;
			else this->plateauVetoesLS++;
		}
		// I-015: with abc.plateau = allow an improvement that lands exactly on
		// the incumbent's makespan is written back instead of discarded.
		if (optimised.second->isBetterThan(target->getFitness())
			&& (this->plateauAllow
				|| !optimised.second->isEqualTo(this->bestSoFar->getFitness()))) {
			// Lamarckism
			if (this->lsLamarckism)
				this->sharedVariables->encoder->encode(optimised.first,
					target, this->sharedVariables);

			target->updatePhenotype(optimised.first);
			target->updateFitness(optimised.second);
			population->setSorted(false);
			target->setNumTrials(0);
			this->improvementsLS++;
		}
		else if (target->getFitness()->isBetterThan(optimised.second)) {
			this->enworstmentsLS++;
			target->setNumTrials(target->getNumTrials() + 1);
			if (optimised.first != NULL) delete optimised.first;
			if (optimised.second != NULL) delete optimised.second;
		}

		else {
			this->neutralLS++;
			target->setNumTrials(target->getNumTrials() + 1);
			if (optimised.first != NULL) delete optimised.first;
			if (optimised.second != NULL) delete optimised.second;
		}
	}


}

