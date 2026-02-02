/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "FJSPRobustnessAnalyzer.h"



namespace PostExecution {
	
class FJSPRobustnessAnalyzerMakespan: public FJSPRobustnessAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	FJSPRobustnessAnalyzerMakespan();

protected:

	void analyzeObjectiveFunction(const FJSP::ProblemFJSP *problemFJSP, FJSP::ScheduleFJSP * schedule, FuzzyFW::FitnessTFN * fitness, const FuzzyFW::ParameterDB *params, int numRun);

};
}
