/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "ScheduleFJSP.h"
#include "EvaluationFJSP.h"
#include "Random.h"
#include "PostExecutionException.h"
#include "FJSPScenarioManager.h"
#include "RobustnessFileWriter.h"
#include "PostExecutionAnalyzer.h"


namespace PostExecution {
#define ROBUSTNESS_NUM_ITERATIONS 10000
#define ROBUSTNESS_INITIAL_SEED 1
#define	ROBUSTNESS_ROW_PREFIX "R"
	   
class FJSPRobustnessAnalyzer: public PostExecutionAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	FuzzyFW::Random rnd;

	FJSPScenarioManager scenarioManager;

	RobustnessFileWriter writer;

	FJSPRobustnessAnalyzer();

	unsigned int getRandomDuration(const FJSP::TaskFJSP* task, unsigned int rep);

	unsigned int getDueDate(const unsigned int numJob, unsigned int rep);

	void close();

	void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature);

	virtual void analyzeObjectiveFunction(const FJSP::ProblemFJSP *problemFJSP, FJSP::ScheduleFJSP * schedule, FuzzyFW::FitnessTFN * fitness, const FuzzyFW::ParameterDB *params, int numRun) = 0;

	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun);
	
	FJSP::ScheduleFJSP * castSchedule(FuzzyFW::Solution* solution);

	FJSP::ProblemFJSP* castProblem(FuzzyFW::Problem* problem);

	FuzzyFW::FitnessTFN* castFitness(FuzzyFW::Fitness* objective);
};
}
