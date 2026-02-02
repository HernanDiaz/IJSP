/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "PostExecutionManager.h"

#include <iostream>
using namespace std;


namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	PostExecutionManager::PostExecutionManager() {
		PostExecution::PostExecutionClassRegister::registerClasses();
	}

	void PostExecutionManager::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const FuzzyFW::ParameterDB *params, int numRun) {
		this->robustnessAnalyzer->analyze(problem, solution, objective, params, numRun);
		//this->IJSPTardinessAnalyzer->analyze(problem, solution, objective, params, numRun);
	}
		
	void PostExecutionManager::open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature, const FuzzyFW::ParameterDB *params) {
	    this->loadAnalyzers(params);
		this->robustnessAnalyzer->open(problem, outputPrefix, signature);
		//this->IJSPTardinessAnalyzer->open(problem, outputPrefix, signature);
	}

	void PostExecutionManager::loadAnalyzers(const FuzzyFW::ParameterDB *params) {
		std::string sgsType = params->getStringUpper("objective");
		if (sgsType.length() == 0) {
			std::string errorMsg = "Objective function not found. Please, specify a obkective function to use";
			errorMsg += " during the robustness analysis";
			throw new PostExecutionException("Robustness", errorMsg);
		}
		this->robustnessAnalyzer = PostExecutionClassRegister::getRobustnessObject(sgsType);
		if (this->robustnessAnalyzer == NULL) {
			std::string errorMsg = "The introduced objective function is not";
			errorMsg += " recognised: \'" + sgsType + "\'";
			throw new PostExecutionException("Robustness", errorMsg);
		}
		/*
		this->IJSPTardinessAnalyzer = PostExecutionClassRegister::getMakespanMRObject(sgsType);
		if (this->IJSPTardinessAnalyzer == NULL) {
			std::string errorMsg = "The introduced objective function is not";
			errorMsg += " recognised: \'" + sgsType + "\'";
			throw new PostExecutionException("Robustness", errorMsg);
		}*/
	}

	void PostExecutionManager::close() {
		this->robustnessAnalyzer->close();
		//this->IJSPTardinessAnalyzer->close();
	}
}