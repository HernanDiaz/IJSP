/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "Problem.h"
#include "Solution.h"
#include "ScheduleIJSP.h"
#include "IJSPException.h"
#include "RobustnessFileWriter.h"
#include "Random.h"


namespace Robustness {
#define STURDINESS_NUM_ITERATIONS 1000
#define STURDINESS_INITIAL_SEED 1
#define STURDINESS_OUTPUT_SCEN_NAME "Scenarios"

	class ScenarioManager {
		//=====================================================================
		//		FIELDS
		//=====================================================================

	public:
		/**
		* Default constructor
		*/
		ScenarioManager();

		void init(const IJSP::ProblemIJSP *problem, std::string outputPrefix, const int numScenarios);

		std::string getFileName(const IJSP::ProblemIJSP *problem);

		void generateScenarios(const IJSP::ProblemIJSP *problem, std::string outputPrefix, const int numScenarios);

		void loadScenarios(const IJSP::ProblemIJSP *problem, std::string outputPrefix, const int numScenarios);

		inline bool fileExists(const std::string& name);

		const unsigned int getDuration(const unsigned int numTask, const unsigned int numSimulation);

		const unsigned int getDueDate(const unsigned int numJob, const unsigned int numSimulation);

	protected:

		FuzzyFW::Random rnd;

		std::vector< std::vector<unsigned int> > crispTimes;
		std::vector< std::vector<unsigned int> > crispDueDates;

		int getRandomDuration(const IJSP::TaskIJSP* task);

		int getRandomDuration(int a, int b);

	};
}
