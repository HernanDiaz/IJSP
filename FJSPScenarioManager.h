/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "Problem.h"
#include "Solution.h"
#include "ScheduleFJSP.h"
#include "FJSPException.h"
#include "RobustnessFileWriter.h"
#include "Random.h"


namespace PostExecution {
#define ROBUSTNESS_INITIAL_SEED 1
#define ROBUSTNESS_OUTPUT_SCEN_NAME "Scenarios"

	class FJSPScenarioManager {
		//=====================================================================
		//		FIELDS
		//=====================================================================

	public:
		/**
		* Default constructor
		*/
		FJSPScenarioManager();

		void init(const FJSP::ProblemFJSP *problem, std::string outputPrefix, const int numScenarios);

		std::string getFileName(const FJSP::ProblemFJSP *problem);

		void generateScenarios(const FJSP::ProblemFJSP *problem, std::string outputPrefix, const int numScenarios);

		void loadScenarios(const FJSP::ProblemFJSP *problem, std::string outputPrefix, const int numScenarios);

		inline bool fileExists(const std::string& name);

		const unsigned int getDuration(const unsigned int numTask, const unsigned int numSimulation);

		const unsigned int getDueDate(const unsigned int numJob, const unsigned int numSimulation);

	protected:

		FuzzyFW::Random rnd;

		std::vector< std::vector<unsigned int> > crispTimes;
		std::vector< std::vector<unsigned int> > crispDueDates;

		int getRandomDuration(const FJSP::TaskFJSP* task);

		int getRandomDuration(int a, int b);

	};
}
