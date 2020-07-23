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

#include "Evaluation.h"
#include "EvaluationIJSP.h"
#include "RobustnessException.h"
#include "RobustnessClassRegister.h"
#include "RobustnessAnalyzer.h"

namespace PostExecution {
class PostExecutionAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	PostExecutionAnalyzer();

	void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature, const FuzzyFW::ParameterDB *params);

	void close();
		
	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective , const  FuzzyFW::ParameterDB *params, int numRun);

private:

	RobustnessAnalyzer* robustnessAnalyzer;

	void loadRobustnessAnalyzer(const FuzzyFW::ParameterDB *params);
};
}
