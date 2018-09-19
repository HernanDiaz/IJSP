/*
* EvoLauncher.cpp
*
*  Created on: Sep 20, 2017
*      Author: jjpalacios
*/


#include "EvoLauncher.h"

namespace FJSP {

//=============================================================================
//
//	Class EvoLauncher
//
//=============================================================================
//=============================================================================
//		CONSTRUCTORS / INITIALIZERS
//=============================================================================
//-----  Main constructor  ----------------------------------------------------
EvoLauncher::EvoLauncher(const char * problemPath, const char* algorithmSetup) {
	this->numRuns = 0;
	this->algorithm = NULL;
	this->seed = 1;
	this->logFolder = "." + FSEP;

	this->loadConfiguration(algorithmSetup);

	this->problem = new FuzzyProblem(this->setup, problemPath);
}


//-----  Destructor  ----------------------------------------------------------
EvoLauncher::~EvoLauncher() {
	delete this->algorithm;
	delete this->problem;
	for (size_t i = 0; i < this->fitnessValues.size(); i++)
		delete this->fitnessValues[i];
}





//=============================================================================
//		MAIN METHODS
//=============================================================================
//-----  Optimise method  -----------------------------------------------------
std::string EvoLauncher::optimise() {
	std::pair<Solution *, Objective *> solution, bestSolution;
	std::string signature = this->generateSignature();

	std::cout << "Experiment " << signature << std::endl << std::endl;
	std::cout << "Solving instance " << this->problem->getName() << std::endl;

	// Initialize the algorithm
	this->algorithm->prepareToRun(this->setup);

	// Prepare average data
	this->avgTimes.clear();
	this->avgStats.clear();
	this->avgEvolStats.clear();
	this->fitnessValues.clear();

	// Open output file for solutions
	std::ofstream outputFile;
	std::string outputName = this->logFolder + FSEP;
	outputName += signature + "_" + EVO_OUTPUT_SOLS + ".csv";

	outputFile.open(outputName.c_str());
	if (!outputFile.is_open()) {
		std::string err = "It was impossible to generate the output ";
		err += "files. They may be opened or the logFolder does ";
		err += "not exist";
		throw new FJSPException("Environment", err);
	}
	outputFile << "Run;Solution;Objective Value" << std::endl;

	for (int run = 0; run < numRuns; run++) {
		std::cout << "Run " << run + 1 << "..." << std::endl;
		// Run the algorithm
		solution = this->algorithm->run(this->problem,
			signature + valueToString(run+1), this->logFolder, this->seed + run);

		// Print setup
		this->avgTimes.push_back(algorithm->getRuntime());

		// Print basic statistics
		this->avgStats.push_back(algorithm->getStatistics());

		// Keep the fitness value
		this->fitnessValues.push_back(solution.second);

		// Information about the set of solutions
		outputFile << run + 1 << std::endl;
		outputFile << solution.first->toString() << ";";
		outputFile << solution.second->toString() << std::endl;

		// Print evolution data
		std::vector< std::vector<double> > evolStats;
		evolStats = algorithm->getEvolution(this->avgEvolLabels);
		this->avgEvolStats.push_back(evolStats);

		// Look for the best solution
		if (run == 0
			|| solution.second->isBetterThan(bestSolution.second)) {
			bestSolution.first = solution.first;
			bestSolution.second = solution.second;
		}
		else {
			delete solution.first;
		}
	}
	outputFile.close();

	// Open output file
	outputName = this->logFolder + FSEP;
	outputName += signature + ".csv";

	outputFile.open(outputName.c_str());
	if (!outputFile.is_open()) {
		std::string err = "It was impossible to generate the output ";
		err += "files. They may be opened or the logFolder does ";
		err += "not exist";
		throw new FJSPException("Environment", err);
	}

	algorithm->printSetupTree(outputFile);
	outputFile << std::endl;
	this->printRuntimes(outputFile);
	outputFile << std::endl;
	this->printStatistics(outputFile);
	outputFile << std::endl;
	this->printObjectiveValues(outputFile);
	outputFile << std::endl;
	this->printEvolutionTrace(outputFile);

	outputFile.close();

	std::cout << "The process has finished!" << std::endl;
	return bestSolution.first->toString();
}





//=============================================================================
//		CONFIGURATION METHODS
//=============================================================================
//-----  setFolder method  ----------------------------------------------------
void EvoLauncher::setLogFolder(const char *path) {
	this->logFolder = std::string(path);
	if (this->logFolder.length() == 0) {
		std::string err = "The path to the log folder has not been found";
		throw new FJSPException("Loading", err);
	}
	makeDir(path);
}



//-----  loadConfiguration method  --------------------------------------------
void EvoLauncher::loadConfiguration(const char* paramsFile) {
	this->setup = new ParameterDB(paramsFile);

	// Read the seed
	this->seed = this->setup->getInteger(EVO_SEED, time(NULL));
	if (this->seed < 0) {
		throw new FJSPException("Loading", "The seed must be a positive value");
	}

	// Read the number of runs for the algorithm
	this->numRuns = this->setup->getInteger(EVO_RUNS, 1);
	if (this->numRuns <= 0) {
		std::string err = "The number of runs is not valid. It must be a ";
		err += "positive value";
		throw new FJSPException("Loading", err);
	}

	// Read the name of the algorithm to use
	std::string algorithmName = this->setup->getString(EVO_ALGORITHM);
	if (algorithmName.length() == 0) {
		std::string err = "The name of the algorithm to use has not been found";
		throw new FJSPException("Loading", err);
	}

	this->setupAlgorithm(algorithmName);
}



//-----  setupAlgorithm method  -----------------------------------------------
void EvoLauncher::setupAlgorithm(const std::string name) {
	// Genetic algorithm
	if (toUpper(name).compare("GENETIC") == 0
		|| toUpper(name).compare("GA") == 0)
		this->algorithm = new GeneticAlgorithm(this->setup);
	else if (toUpper(name).compare("MEMETIC") == 0
		|| toUpper(name).compare("MA") == 0)
		this->algorithm = new MemeticAlgorithm(this->setup);
	else {
		std::string err = "Solving algorithm \'" + name + "\' unknown";
		throw FJSPException("Loading", err);
	}
}



//-----  generateSignature method  --------------------------------------------
std::string EvoLauncher::generateSignature() const {
	time_t now;
	time(&now);
	struct tm timeinfo;
	localtime_s(&timeinfo, &now);

	const int bufferSize = 80;
	char buffer[bufferSize];
	strftime(buffer, bufferSize, "%Y%m%d%H%M%S", &timeinfo);
	return this->problem->getName() + "_" + std::string(buffer);
}





//=============================================================================
//		LOG METHODS
//=============================================================================
//-----  printRuntimes method  ------------------------------------------------
void EvoLauncher::printRuntimes(std::ofstream &outputFile) {
	if (this->avgTimes.size() <= 0) return;
	std::string separator = "------------------------------------------------";
	outputFile << std::endl << "Runtimes" << std::endl;
	outputFile << separator << std::endl;

	outputFile << "Time;Average;Best;Worst";
	for (unsigned int i = 0; i < avgTimes.size(); i++) {
		outputFile << ";Run " << i + 1;
	}
	outputFile << std::endl;

	std::vector<double> bestValues(avgTimes[0].size(), Infd);
	std::vector<double> worstValues(avgTimes[0].size(), -Infd);
	std::vector<double> avgValues(avgTimes[0].size(), 0.0);

	for (unsigned int i = 0; i < avgTimes[0].size(); i++) {
		for (unsigned int j = 0; j < avgTimes.size(); j++) {
			if (avgTimes[j][i].second < bestValues[i])
				bestValues[i] = avgTimes[j][i].second;
			if (avgTimes[j][i].second > worstValues[i])
				worstValues[i] = avgTimes[j][i].second;
			avgValues[i] += avgTimes[j][i].second;
		}
		avgValues[i] /= avgTimes.size();
	}

	for (unsigned int i = 0; i < avgTimes[0].size(); i++) {
		outputFile << avgTimes[0][i].first;
		outputFile << ";" << avgValues[i];
		outputFile << ";" << bestValues[i];
		outputFile << ";" << worstValues[i];
		for (unsigned int j = 0; j < avgTimes.size(); j++)
			outputFile << ";" << avgTimes[j][i].second;
		outputFile << std::endl;
	}
}



//-----  printStatistics method  ----------------------------------------------
void EvoLauncher::printStatistics(std::ofstream &outputFile) {
	if (this->avgStats.size() <= 0) return;
	std::string separator = "------------------------------------------------";
	outputFile << std::endl << "Basic statistics" << std::endl;
	outputFile << separator << std::endl;

	outputFile << "Field;Average;Best;Worst";
	for (unsigned int i = 0; i < this->avgStats.size(); i++) {
		outputFile << ";Run " << i + 1;
	}
	outputFile << std::endl;

	std::vector<double> bestValues(this->avgStats[0].size(), Infd);
	std::vector<double> worstValues(this->avgStats[0].size(), -Infd);
	std::vector<double> avgValues(this->avgStats[0].size(), 0.0);

	for (unsigned int i = 0; i < this->avgStats[0].size(); i++) {
		for (unsigned int j = 0; j < this->avgStats.size(); j++) {
			if (this->avgStats[j][i].second < bestValues[i])
				bestValues[i] = this->avgStats[j][i].second;
			if (this->avgStats[j][i].second > worstValues[i])
				worstValues[i] = this->avgStats[j][i].second;
			avgValues[i] += this->avgStats[j][i].second;
		}
		avgValues[i] /= this->avgStats.size();
	}

	outputFile << this->avgStats[0][0].first;
	outputFile << ";" << avgValues[0];
	outputFile << ";" << bestValues[0];
	outputFile << ";" << worstValues[0];
	for (unsigned int j = 0; j < this->avgStats.size(); j++)
		outputFile << ";" << this->avgStats[j][0].second;
	outputFile << std::endl;

	for (unsigned int i = 1; i < this->avgStats[0].size(); i++) {
		outputFile << this->avgStats[0][i].first;
		outputFile << ";" << avgValues[i];
		outputFile << ";" << bestValues[i];
		outputFile << ";" << worstValues[i];
		for (unsigned int j = 0; j < this->avgStats.size(); j++)
			outputFile << ";" << this->avgStats[j][i].second;
		outputFile << std::endl;
	}
}


//-----  printObjectiveValues method  -----------------------------------------
void EvoLauncher::printObjectiveValues(std::ofstream &outputFile) {
	if (this->fitnessValues.size() <= 0) return;
	std::string separator = "------------------------------------------------";
	outputFile << std::endl << "Objective function values" << std::endl;
	outputFile << separator << std::endl;

	outputFile << "Average;Best;Worst";
	for (unsigned int i = 0; i < this->avgStats.size(); i++) {
		outputFile << ";Run " << i + 1;
	}
	outputFile << std::endl;

	Fitness *best, *worst;
	double sum;
	best = worst = this->fitnessValues[0];
	sum = best->toDouble();

	for (size_t i = 1; i < this->fitnessValues.size(); i++) {
		if (this->fitnessValues[i]->isBetterThan(best))
			best = this->fitnessValues[i];
		if (this->fitnessValues[i]->isWorseThan(worst))
			worst = this->fitnessValues[i];
		sum += this->fitnessValues[i]->toDouble();
	}

	outputFile << sum / this->fitnessValues.size();
	outputFile << ";" << best->toString();
	outputFile << ";" << worst->toString();
	
	for (size_t i = 0; i < this->fitnessValues.size(); i++)
		outputFile << ";" << this->fitnessValues[i]->toString();
	outputFile << std::endl;
}


//-----  printEvolutionTrace method  ------------------------------------------
void EvoLauncher::printEvolutionTrace(std::ofstream &outputFile) {
	if (this->avgEvolStats.size() <= 0) return;
	std::string separator = "------------------------------------------------";
	outputFile << std::endl << "Evolution" << std::endl;
	outputFile << separator << std::endl;

	outputFile << "Average Evolution";
	for (unsigned int i = 0; i < avgEvolStats.size(); i++) {
		for (unsigned int j = 0; j < avgEvolStats[i][0].size() - 1; j++)
			outputFile << ";";
		outputFile << ";Run " << i+1;
	}
	outputFile << std::endl << "Step";

	for (unsigned int i = 0; i <= avgEvolStats.size(); i++) {
		for (unsigned int j = 0; j < avgEvolLabels.size(); j++)
			if (i > 0 || j > 0)
				outputFile << ";" << avgEvolLabels[j];
	}
	outputFile << std::endl;

	// Look for the shortest evolution
	unsigned int minSize = avgEvolStats[0].size();
	unsigned int maxSize = minSize;
	for (unsigned int i = 1; i < avgEvolStats.size(); i++) {
		if (avgEvolStats[i].size() < minSize)
			minSize = avgEvolStats[i].size();
		if (avgEvolStats[i].size() > maxSize)
			maxSize = avgEvolStats[i].size();
	}

	std::vector< std::vector<double> > avgValues(minSize);

	for (unsigned int row = 0; row < minSize; row++) {
		avgValues[row].resize(avgEvolStats[0][row].size(), 0.0);
		for (unsigned int col = 0; col < avgEvolStats[0][row].size(); col++) {
			for (unsigned int run = 0; run < avgEvolStats.size(); run++) {
					avgValues[row][col] += avgEvolStats[run][row][col];
			}
			avgValues[row][col] /= avgEvolStats.size();
		}
	}

	for (unsigned int i = 0; i < maxSize; i++) {
		if (i < avgValues.size())
			outputFile << avgValues[i][0];

		for (unsigned int j = 1; j < avgEvolLabels.size(); j++) {
			outputFile << ";";
			if (i < avgValues.size())
				outputFile << avgValues[i][j];
		}
		for (unsigned int run = 0; run < avgEvolStats.size(); run++) {
			for (unsigned int j = 0; j < avgEvolLabels.size(); j++) {
				outputFile << ";";
				if (i < avgEvolStats[run].size())
					outputFile << avgEvolStats[run][i][j];
			}
		}
		outputFile << std::endl;
	}
}

}

