/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "IJSPRobustnessAnalyzer.h"



namespace PostExecution {
	
class IJSPRobustnessAnalyzerMakespan: public IJSPRobustnessAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	IJSPRobustnessAnalyzerMakespan();

protected:

	void analyzeObjectiveFunction(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun);

};
}
