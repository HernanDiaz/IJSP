/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "ScheduleIJSP.h"
#include "EvaluationIJSP.h"
#include "Random.h"
#include "RobustnessException.h"
#include "ScenarioManager.h"
#include "RobustnessFileWriter.h"
#include "RobustnessAnalyzer.h"


namespace PostExecution {
#define ROBUSTNESS_NUM_ITERATIONS 1000
#define ROBUSTNESS_INITIAL_SEED 1
#define	ROBUSTNESS_ROW_PREFIX "R"
	   
class IJSPRobustnessAnalyzer: public RobustnessAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	FuzzyFW::Random rnd;

	ScenarioManager scenarioManager;

	RobustnessFileWriter writer;

	IJSPRobustnessAnalyzer();

	unsigned int getRandomDuration(const IJSP::TaskIJSP* task, unsigned int rep);

	unsigned int getDueDate(const unsigned int numJob, unsigned int rep);

	void close();

	void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature);

	virtual void analyzeObjectiveFunction(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun) = 0;

	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun);
	
	IJSP::ScheduleIJSP * IJSPRobustnessAnalyzer::castSchedule(FuzzyFW::Solution* solution);

	IJSP::ProblemIJSP* IJSPRobustnessAnalyzer::castProblem(FuzzyFW::Problem* problem);

	FuzzyFW::FitnessInterval* castFitness(FuzzyFW::Fitness* objective);
};
}
