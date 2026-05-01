/*
* MemeticAlgorithm.cpp
*
*  Created on: Oct 13, 2015
*/

#include "ArtificialBeeColony.h"
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
	ArtificialBeeColony::ArtificialBeeColony(ParameterDB *params)
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
	ArtificialBeeColony::~ArtificialBeeColony() {
		delete localSearch;
	}


	//-----  clearAll  ------------------------------------------------------------
	void ArtificialBeeColony::clearAll() {
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
	void ArtificialBeeColony::printSetupTree(std::ofstream & output) const {
		output << "Artificial Bee Colony" << std::endl;

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
	std::vector< std::pair<std::string, double> > ArtificialBeeColony::getStatistics()
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
			("Best solution", this->bestSoFar->getFitness()->toDouble()));
		return stats;
	}



	//-----  getRuntime  ----------------------------------------------------------
	std::vector< std::pair<std::string, double> > ArtificialBeeColony::getRuntime()
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
	void ArtificialBeeColony::prepareToRun(ParameterDB *params) {
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
		else if (value.length() >= 1)
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
	bool ArtificialBeeColony::checkSetup() {
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

	struct cmp {
		bool operator() (Fitness* a, Fitness* b) const {
			return a->isWorseThan(b);
		}
	};


	//-----  RUN !!  --------------------------------------------------------------
	//=============================================================================
	std::pair<Solution *, Objective *> ArtificialBeeColony::run(Problem *problem,
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
			std::vector<int> elite;

			int elite_value = this->sharedVariables->parameters->getInteger(ELITE_SELECTION);
			int elite_size = this->sharedVariables->parameters->getInteger(ELITE_SIZE);
			switch (elite_value) {
			case 8: {//Estrategia 8, coger el que tenga mas intentos de mejor a peor que no haya sido escogido previamente
				if (bestFoodSource) {
					delete bestFoodSource;
				}
				unsigned int max_trials = 0;
				int k = 0;
				int num_ind = 0;
				while (k < currentPopulation->size()) {
					if (currentPopulation->getIndividual(k)->getNumTrials() > max_trials) {
						//&&	!cross_indiv.count(currentPopulation->getBest(this->sharedVariables, k)->getFitness())) {
						max_trials = currentPopulation->getIndividual(k)->getNumTrials();
						num_ind = k;
					}
					k++;
				}

				for (k = 0; k < currentPopulation->size(); k++) {
					if (currentPopulation->getIndividual(k)->getNumTrials() == max_trials) {
						elite.push_back(k);
					}
				}
				unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, elite.size() - 1);
				bestFoodSource = currentPopulation->getIndividual(elite[foodSourceID])->clone();
				break;
			}
			case 4: {//Estrategia 4, coger el que tenga mas intentos de mejor a peor que no haya sido escogido previamente
				if (bestFoodSource) {
					delete bestFoodSource;
				}
				unsigned int max_trials = 0;
				int k = 0;
				int num_ind = 0;
				while (k < currentPopulation->size()) {
					if (currentPopulation->getBest(this->sharedVariables, k)->getNumTrials() > max_trials) {
						//&&	!cross_indiv.count(currentPopulation->getBest(this->sharedVariables, k)->getFitness())) {
						max_trials = currentPopulation->getBest(this->sharedVariables, k)->getNumTrials();
						num_ind = k;
					}
					k++;
				}
				bestFoodSource = currentPopulation->getBest(this->sharedVariables, num_ind)->clone();
				//cross_indiv.insert(bestFoodSource->getFitness());
				break;
			}
			case 7:
			case 3: {
				//Estrategia 3, coger el que no haya sido cogido previamente de mejor a peor
				if (bestFoodSource) {
					delete bestFoodSource;
				}
				int k = 0;
				while (k < currentPopulation->size() && cross_indiv.count(currentPopulation->getBest(this->sharedVariables, k)->getFitness())) { k++; }
				if (k == currentPopulation->size()) {
					unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, 20);
					bestFoodSource = currentPopulation->getBest(this->sharedVariables, foodSourceID)->clone();
				}
				else {
					bestFoodSource = currentPopulation->getBest(this->sharedVariables, k)->clone();
				}
				cross_indiv.insert(bestFoodSource->getFitness()->clone());
				break;
			}
			case 6:
			case 2: {
				if (bestFoodSource) {
					delete bestFoodSource;
				}
				//estrategia 2, coger de forma aleatoria uno de los 20 primeros
				unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, elite_size);
				if (foodSourceID == 0) {
					bestFoodSource = this->bestSoFar->clone();
				}
				else {
					bestFoodSource = currentPopulation->getBest(this->sharedVariables, foodSourceID-1)->clone();
				}
				break;
			}
			case 5:
			case 1: {
				if (bestFoodSource) {
					delete bestFoodSource;
				}
				//estrategia por defecto coger el mejor
				bestFoodSource = this->bestSoFar->clone();
				break;
			}
			}

			for (int i = 0; i < currentPopulation->size(); i++) {
				if (this->sharedVariables->rng->getProbability() < this->crossoverProb) {
					Population currentFoodSources;
					Individual* currentFoodSource = currentPopulation->getIndividual(i);
					//Add bestfoodSource and currentFoodSource 
					currentFoodSources.addIndividual(bestFoodSource->clone());
					currentFoodSources.addIndividual(currentFoodSource->clone());
					//Cross bestfoodSource and currentFoodSource 
					this->crossover->apply(&currentFoodSources, this->crossoverProb, this->sharedVariables);
					//Add currentFoodSource  (por lo que veo la operaci�n de crossover modifica los individuos)
					Individual* currentFoodSourceClone = currentFoodSource->clone();
					currentFoodSources.addIndividual(currentFoodSourceClone);
					//Evaluo solo los individuos resultantes del cruce (cuando se utiliza clone, se mantiene si los individuos estan evaludos o no?)
					this->evaluator->evaluatePopulation(this->sharedVariables, &currentFoodSources, false);
					Individual* bestLocal = currentFoodSources.getBest(this->sharedVariables);
					//If the best local food source is better than the currentFoodSource we replace it
					if (bestLocal->getFitness()->isBetterThan(currentFoodSourceClone->getFitness())
						&& !bestLocal->getFitness()->isEqualTo(this->bestSoFar->getFitness())) {
						Individual* bestlocalClone = bestLocal->clone();
						delete currentPopulation->replaceIndividual(i, bestlocalClone);
						bestlocalClone->setNumTrials(0);

					}
					else if (this->simulatedCooling != NULL
						&& this->simulatedCooling->isSelected(currentFoodSourceClone, bestLocal, this->sharedVariables, this->generation + 1)) {
						Individual* bestlocalClone = bestLocal->clone();
						delete currentPopulation->replaceIndividual(i, bestlocalClone);
						bestlocalClone->setNumTrials(0);
					}
					else {
						currentFoodSource->setNumTrials(currentFoodSource->getNumTrials() + 1);
					}

					//NEW
					if (elite_value > 4) {

						switch (elite_value) {
						case 5: {
							Population bestLocalbestGlobal;
							bestLocalbestGlobal.addIndividual(bestLocal->clone());
							Individual* bestFoodSourceClone = bestFoodSource->clone();
							bestLocalbestGlobal.addIndividual(bestFoodSourceClone);
							this->evaluator->evaluatePopulation(this->sharedVariables, &bestLocalbestGlobal, false);
							Individual* bestGlobal = bestLocalbestGlobal.getBest(this->sharedVariables);

							if (bestGlobal->getFitness()->isBetterThan(bestFoodSourceClone->getFitness())) {
								delete bestFoodSource;
								bestFoodSource = bestGlobal->clone();
							}
							bestLocalbestGlobal.clear(true);
							break;
						}
						case 6: {
							//estrategia 2, coger de forma aleatoria uno de los 20 primeros
							unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, elite_size);
							this->evaluator->evaluatePopulation(this->sharedVariables, currentPopulation, false);
							currentPopulation->sort(this->sharedVariables->rng, true);
							delete bestFoodSource;
							if (foodSourceID == 0) {
								bestFoodSource = this->bestSoFar->clone();
							}
							else {
								bestFoodSource = currentPopulation->getBest(this->sharedVariables, foodSourceID - 1)->clone();
							}
							break;
						}
						case 7: {
							//Estrategia 3, coger el que no haya sido cogido previamente de mejor a peor
							delete bestFoodSource;
							int k = 0;
							while (k < currentPopulation->size() && cross_indiv.count(currentPopulation->getBest(this->sharedVariables, k)->getFitness())) { k++; }
							if (k == currentPopulation->size()) {
								unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, 20);
								bestFoodSource = currentPopulation->getBest(this->sharedVariables, foodSourceID)->clone();
							}
							else {
								bestFoodSource = currentPopulation->getBest(this->sharedVariables, k)->clone();
							}
							cross_indiv.insert(bestFoodSource->getFitness()->clone());
							break;
						}
						case 8: {
							delete bestFoodSource;
							unsigned int foodSourceID = this->sharedVariables->rng->getInteger(0, elite.size() - 1);
							bestFoodSource = currentPopulation->getIndividual(elite[foodSourceID])->clone();
							break;
						}

						}
					}
					currentFoodSources.clear(true);
				}
			}
			// Onlooker bee phase
			if (bestFoodSource) {
				delete bestFoodSource;
			}
					
			for (unsigned int i = 0; i < currentPopulation->size(); i++) {
				if (this->sharedVariables->rng->getProbability() < this->mutationProb) {
					Individual* origin = currentPopulation->getIndividual(i);
					if (origin->getNumTrials() < this->sharedVariables->parameters->getInteger(MAX_NUM_TRIALS)) {
						Individual* destiny = currentPopulation->getIndividual(i)->clone();
						this->mutation->apply(destiny, this->sharedVariables);
						Fitness* fitness = this->evaluator->evaluate(this->sharedVariables, destiny);
						destiny->updateFitness(fitness);
						if (destiny->getFitness()->isBetterThan(origin->getFitness())
							&& !destiny->getFitness()->isEqualTo(this->bestSoFar->getFitness())) {
							delete currentPopulation->replaceIndividual(i, destiny);
							destiny->setNumTrials(0);
						}
						else if (this->simulatedCooling!=NULL 
							&& this->simulatedCooling->isSelected(origin,destiny,this->sharedVariables,this->generation+1)){
							/*
							std::cout << "Gen: " << this->generation+1 << "\tT: " << this->simulatedCooling->getTemperature(origin, destiny, generation+1);
							std::cout << "\tMonotonic: " << this->simulatedCooling->getMonotonicTemperature(generation+1) << "\tCorrective: " << this->simulatedCooling->getAdaptativeFactor(origin,destiny);
							std::cout << "\tOrigin: " << origin->getFitness()->toDouble() << "\tDest" << destiny->getFitness()->toDouble() << std::endl;
							*/
							//std::cout << log(1 + this->generation + 1) << std::endl;
							
							//else if (this->sharedVariables->rng->getInteger(0, 150) < (150 - this->generation)) {
							delete currentPopulation->replaceIndividual(i, destiny);
							destiny->setNumTrials(0);
						}
						else {
							delete destiny;
							origin->setNumTrials(origin->getNumTrials() + 1);
						}
					}
				}

			}
			
			//	Scout bee phase
			int numSourceAfterLimit = 0;
			//We check the number of food sources that surpass the maximum number of trials
			for (int i = 0; i < currentPopulation->size(); i++) {
				if (currentPopulation->getIndividual(i)->getNumTrials() >= this->sharedVariables->parameters->getInteger(MAX_NUM_TRIALS)) {
					numSourceAfterLimit++;
				}
			}

			this->abc_replacements += numSourceAfterLimit;
			//We replace those food sources with fresh ones.
			if (numSourceAfterLimit > 0) {
				Population* newPopulation = this->creation->createPopulation(numSourceAfterLimit, this->sharedVariables);
				this->evaluator->evaluatePopulation(this->sharedVariables, newPopulation, false);

				if (this->lsFrequency == LS_Frequency::MALS_INITIAL) {
					this->applyLocalSearch(newPopulation);
				}
				int j = 0;
				for (int i = 0; i < currentPopulation->size(); i++) {
					if (currentPopulation->getIndividual(i)->getNumTrials() >= this->sharedVariables->parameters->getInteger(MAX_NUM_TRIALS)) {
						delete currentPopulation->replaceIndividual(i, newPopulation->getBest(this->sharedVariables, j));
						j++;
					}
				}
				newPopulation->clear(false);
				delete newPopulation;
				this->evaluator->evaluatePopulation(this->sharedVariables, currentPopulation, true);
			}


			// Conditions to apply the local search
			
			timePoint = clock();
			//std::ofstream outfile;
			//outfile.open("LocalSearchdebugN1.txt", std::ios_base::app); // append instead of overwrite
			//outfile << "Localsearch generation " << this->generation << std::endl;
			//currentPopulation->printCSV(outfile);
			//outfile.close();
			if (this->lsFrequency == LS_Frequency::MALS_PERIOD
				|| this->lsFrequency == LS_Frequency::MALS_INITIAL
				&& this->generation % this->lsPeriod == 0)
			{
				this->applyLocalSearch(currentPopulation);
			}
			this->localSearchTime += clock() - timePoint;
			

			this->generation++;

			if (currentPopulation->getBest(sharedVariables)->getFitness()
				->isBetterThan(this->bestSoFar->getFitness()))
				this->iterationsNI = 0;
			else
				this->iterationsNI++;


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
	void ArtificialBeeColony::evaluatePopulation(Population *current) {
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
	void ArtificialBeeColony::applyLocalSearch(Population *population) {
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
	void ArtificialBeeColony::applyLocalSearch(Population *population,
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

		if (optimised.second->isBetterThan(target->getFitness())
			&& !optimised.second->isEqualTo(this->bestSoFar->getFitness())) {
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

