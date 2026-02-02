/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "heading.h"
#include "FJSPScenarioManager.h"

namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	FJSPScenarioManager::FJSPScenarioManager() {
		rnd.init(ROBUSTNESS_INITIAL_SEED);
	}

	void FJSPScenarioManager::init(const FJSP::ProblemFJSP *problem, std::string outputPrefix, const int numScenarios) {
		this->crispDueDates.resize(numScenarios);
		this->crispTimes.resize(numScenarios);
		if (!this->fileExists(outputPrefix + this->getFileName(problem))) {
			this->generateScenarios(problem, outputPrefix, numScenarios);
		}
		else this->loadScenarios(problem, outputPrefix, numScenarios);
	}

	 std::string FJSPScenarioManager::getFileName(const FJSP::ProblemFJSP *problem) {
		return problem->getName() + "_" + ROBUSTNESS_OUTPUT_SCEN_NAME + ".csv";
	}

	void FJSPScenarioManager::generateScenarios(const FJSP::ProblemFJSP *problem, std::string outputPrefix, const int numScenarios){
		// Open output file for scenarios
		std::ofstream outputFile;
		outputFile.open((outputPrefix + this->getFileName(problem)).c_str());
		if (!outputFile.is_open()) {
			std::string err = "It was impossible to generate the output ";
			err += "files. They may be opened or the logFolder does ";
			err += "not exist";
			throw new FuzzyFW::FuzzyFWException("Environment", err);
		}

		for (int rep = 0; rep < numScenarios; rep++) {
			//We calculate random durations for each task
			for (int i = 0; i < problem->getNumberTasks(); i++) {
				unsigned int randomDuration = getRandomDuration(problem->getTask(i));
				this->crispTimes[rep].push_back(randomDuration);
				outputFile << randomDuration <<" ";
			}
			//We calculate random durations for due dates
			for (int i = 0; i < problem->getNumberJobs(); i++) {
				const FuzzyFW::TimeWindowLinear * dueDate = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(problem->getTimeWindow(i));
				if (dueDate != NULL) {
					unsigned int randomDuration = getRandomDuration(dueDate->d1, dueDate->d2);
					this->crispDueDates[rep].push_back(randomDuration);
					outputFile << randomDuration << " ";
				}
			}
			outputFile << std::endl;
		}
		outputFile.close();
	}

	void FJSPScenarioManager::loadScenarios(const FJSP::ProblemFJSP *problem, std::string outputPrefix, const int numScenarios) {
		// Open output file for scenarios
		std::ifstream inputFile;
		inputFile.open((outputPrefix + this->getFileName(problem)).c_str());
		if (!inputFile.is_open()) {
			std::string err = "It was impossible to open the file ";
			err += "files. They are not accesible or the logFolder does ";
			err += "not exist";
			throw new FuzzyFW::FuzzyFWException("Environment", err);
		}

		for (int rep = 0; rep < numScenarios; rep++) {
			//We load durations for each task
			for (int i = 0; i < problem->getNumberTasks(); i++) {
				unsigned int randomDuration;
				inputFile >> randomDuration;
				this->crispTimes[rep].push_back(randomDuration);
			}
			//We calculate random durations for due dates
			for (int i = 0; i < problem->getNumberJobs(); i++) {
				const FuzzyFW::TimeWindowLinear * dueDate = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(problem->getTimeWindow(i));
				if (dueDate != NULL) {
					unsigned int randomDuration;
					inputFile >> randomDuration;
					this->crispDueDates[rep].push_back(randomDuration);
				}
			}
		}
		inputFile.close();
	}

	int FJSPScenarioManager::getRandomDuration(const FJSP::TaskFJSP* task) {
		return this->rnd.getInteger(task->p.a, task->p.c);
	}

	int FJSPScenarioManager::getRandomDuration(int a, int b) {
		return this->rnd.getInteger(a,b);
	}

	inline bool FJSPScenarioManager::fileExists(const std::string& name) {
		struct stat buffer;
		return (stat(name.c_str(), &buffer) == 0);
	}


	const unsigned int FJSPScenarioManager::getDuration(const unsigned int numTask, const unsigned int numSimulation) {
		return this->crispTimes[numSimulation][numTask];
	}

	const unsigned int FJSPScenarioManager::getDueDate(const unsigned int numJob, const unsigned int numSimulation) {
		return this->crispDueDates[numSimulation][numJob];
	}

}