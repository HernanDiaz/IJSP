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
#include "ScenarioManager.h"
#include "Evaluation.h"
#include "EvaluationIJSP.h"
#include "Random.h"


namespace Robustness {
#define STURDINESS_NUM_ITERATIONS 1000
#define STURDINESS_INITIAL_SEED 1
#define	STURDINESS_ROW_PREFIX "R"
	   
class RobustnessAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	FuzzyFW::Random rnd;

	RobustnessFileWriter writer;

	ScenarioManager scenarioManager;

	RobustnessAnalyzer();

	void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature);

	void close();
	
	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective , const  FuzzyFW::ParameterDB *params, int numRun);

	unsigned int getRandomDuration(const IJSP::TaskIJSP* task, unsigned int rep);

	unsigned int getDueDate(const unsigned int numJob, unsigned int rep);

protected:

	void analyzeMakespan(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun);
	
	void analyzeTardiness(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun);

};
}
