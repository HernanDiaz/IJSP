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

	RobustnessAnalyzer();

	void open(std::string outputName);

	void close();
	
	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, const FuzzyFW::ParameterDB *params, int numRun);
	
	int getRandomDuration(const IJSP::TaskIJSP* task);

};
}
