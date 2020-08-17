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
#include "PostExecutionException.h"
#include "PostExecutionClassRegister.h"
#include "PostExecutionAnalyzer.h"

namespace PostExecution {
class PostExecutionManager {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	PostExecutionManager();

	void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature, const FuzzyFW::ParameterDB *params);

	void close();
		
	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective , const  FuzzyFW::ParameterDB *params, int numRun);

private:

	PostExecutionAnalyzer* robustnessAnalyzer;

	PostExecutionAnalyzer* IJSPTardinessAnalyzer;

	void loadAnalyzers(const FuzzyFW::ParameterDB *params);
};
}
