/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "PostExecutionAnalyzer.h"

#include <iostream>
using namespace std;


namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	PostExecutionAnalyzer::PostExecutionAnalyzer() {
		PostExecution::RobustnessClassRegister::registerClasses();
	}

	void PostExecutionAnalyzer::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const FuzzyFW::ParameterDB *params, int numRun) {
		this->robustnessAnalyzer->analyze(problem, solution, objective, params, numRun);
	}
		
	void PostExecutionAnalyzer::open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature, const FuzzyFW::ParameterDB *params) {
	    this->loadRobustnessAnalyzer(params);
		this->robustnessAnalyzer->open(problem, outputPrefix, signature);
	}

	void PostExecutionAnalyzer::loadRobustnessAnalyzer(const FuzzyFW::ParameterDB *params) {
		std::string sgsType = params->getStringUpper("objective");
		if (sgsType.length() == 0) {
			std::string errorMsg = "Objective function not found. Please, specify a obkective function to use";
			errorMsg += " during the robustness analysis";
			throw new RobustnessException("Robustness", errorMsg);
		}
		this->robustnessAnalyzer = RobustnessClassRegister::getRobustnessObject(sgsType);
		if (this->robustnessAnalyzer == NULL) {
			std::string errorMsg = "The introduced objective function is not";
			errorMsg += " recognised: \'" + sgsType + "\'";
			throw new RobustnessException("Robustness", errorMsg);
		}
	}


	void PostExecutionAnalyzer::close() {
		this->robustnessAnalyzer->close();
	}
}