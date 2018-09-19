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
}





//=============================================================================
//		MAIN METHODS
//=============================================================================
//-----  Optimise method  -----------------------------------------------------
std::string EvoLauncher::optimise() {
	std::pair<Solution *, Objective *> solution, bestSolution;
	std::string signature = this->generateSignature();

	std::cout << "Experiment " << signature << std::endl << std::endl;

	// Initialize the algorithm
	this->algorithm->prepareToRun();

	// Prepare average data
	avgTimes.clear();
	avgStats.clear();
	avgEvolStats.clear();

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

	for (int run = 0; run < numRuns; run++) {
		// Run the algorithm
		solution = this->algorithm->run(this->problem,
			signature + valueToString(run), this->logFolder, this->seed + run);

		// Print setup
		this->avgTimes.push_back(algorithm->getRuntime());

		// Print basic statistics
		this->avgStats.push_back(algorithm->getStatistics());

		// Information about the set of solutions
		outputFile << "Run " << run << std::endl;
		outputFile << "------------------------------------------------------";
		outputFile << std::endl;
		//this->printSetData(outputFile, solution.first, solution.second);
		//outputFile << std::endl;

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
			delete solution.second;
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
	this->printEvolutionTrace(outputFile);

	outputFile.close();

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
	return std::string(buffer);
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
	for (unsigned int i = 0; i < avgStats.size(); i++) {
		outputFile << ";Run " << i + 1;
	}
	outputFile << std::endl;

	std::vector<double> bestValues(avgStats[0].size(), Infd);
	std::vector<double> worstValues(avgStats[0].size(), -Infd);
	std::vector<double> avgValues(avgStats[0].size(), 0.0);

	for (unsigned int i = 0; i < avgStats[0].size(); i++) {
		for (unsigned int j = 0; j < avgStats.size(); j++) {
			if (avgStats[j][i].second < bestValues[i])
				bestValues[i] = avgStats[j][i].second;
			if (avgStats[j][i].second > worstValues[i])
				worstValues[i] = avgStats[j][i].second;
			avgValues[i] += avgStats[j][i].second;
		}
		avgValues[i] /= avgStats.size();
	}

	outputFile << avgStats[0][0].first;
	outputFile << ";" << avgValues[0];
	outputFile << ";" << bestValues[0];
	outputFile << ";" << worstValues[0];
	for (unsigned int j = 0; j < avgStats.size(); j++)
		outputFile << ";" << avgStats[j][0].second;
	outputFile << std::endl;

	for (unsigned int i = 1; i < avgStats[0].size(); i++) {
		outputFile << avgStats[0][i].first;
		outputFile << ";" << avgValues[i];
		outputFile << ";" << bestValues[i];
		outputFile << ";" << worstValues[i];
		for (unsigned int j = 0; j < avgStats.size(); j++)
			outputFile << ";" << avgStats[j][i].second;
		outputFile << std::endl;
	}
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
		outputFile << ";Run " << i;
	}
	outputFile << std::endl << "Step";

	for (unsigned int i = 0; i <= avgEvolStats.size(); i++) {
		for (unsigned int j = 0; j < avgEvolLabels.size(); j++)
			if (i > 0 || j > 0)
				outputFile << ";" << avgEvolLabels[j];
	}
	outputFile << std::endl;

	std::vector< std::vector<double> > avgValues(avgEvolStats[0].size());

	for (unsigned int i = 0; i < avgEvolStats[0].size(); i++) {
		avgValues[i].resize(avgEvolStats[0][i].size(), 0.0);
		for (unsigned int j = 0; j < avgEvolStats[0][i].size(); j++) {
			for (unsigned int k = 0; k < avgEvolStats.size(); k++) {
				avgValues[i][j] += avgEvolStats[k][i][j];
			}
			avgValues[i][j] /= avgEvolStats.size();
		}
	}

	for (unsigned int i = 0; i < avgEvolStats[0].size(); i++) {
		outputFile << avgEvolStats[0][i][0];

		for (unsigned int j = 1; j < avgEvolStats[0][i].size(); j++) {
			outputFile << ";" << avgValues[i][j];
		}
		for (unsigned int k = 0; k < avgEvolStats.size(); k++) {
			for (unsigned int j = 0; j < avgEvolStats[0][i].size(); j++) {
				outputFile << ";" << avgEvolStats[k][i][j];
			}
		}
		outputFile << std::endl;
	}
}

}

