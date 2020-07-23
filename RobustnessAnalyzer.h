/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "ScheduleIJSP.h"
#include "EvaluationIJSP.h"

namespace PostExecution {
	   
class RobustnessAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:

	virtual void close() = 0;

	virtual void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature) = 0;

	virtual void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun) = 0;
};
}
