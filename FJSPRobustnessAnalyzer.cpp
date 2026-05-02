/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "FJSPRobustnessAnalyzer.h"


namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	FJSPRobustnessAnalyzer::FJSPRobustnessAnalyzer() {
		rnd.init(ROBUSTNESS_INITIAL_SEED);
	}
	
	unsigned int FJSPRobustnessAnalyzer::getRandomDuration(const FJSP::TaskFJSP* task, unsigned int rep) {
		return this->scenarioManager.getDuration(task->id, rep);
	}

	unsigned int FJSPRobustnessAnalyzer::getDueDate(const unsigned int numJob, unsigned int rep) {
		return this->scenarioManager.getDueDate(numJob, rep);
	}

	void FJSPRobustnessAnalyzer::close() {
		this->writer.close();
	}

	void FJSPRobustnessAnalyzer::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun) {
		this->analyzeObjectiveFunction(
			this->castProblem(problem),
			this->castSchedule(solution),
			this->castFitness(objective),
			params,
			numRun
		);
	}

	void FJSPRobustnessAnalyzer::open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature) {
		FJSP::ProblemFJSP * problemFJSP =
			dynamic_cast<FJSP::ProblemFJSP *>(problem);
		if (problemFJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Fuzzy Problems.";
			throw FJSP::FJSPException("PostExecutionAnalyzer", errorMsg);
		}

		this->scenarioManager.init(problemFJSP, outputPrefix, ROBUSTNESS_NUM_ITERATIONS);
		this->writer.open(outputPrefix + signature);
		this->writer.writeHeader(ROBUSTNESS_NUM_ITERATIONS);
	}
	
	FJSP::ScheduleFJSP * FJSPRobustnessAnalyzer::castSchedule(FuzzyFW::Solution* solution) {
		FJSP::ScheduleFJSP * schedule =
			dynamic_cast<FJSP::ScheduleFJSP *>(solution);
		if (schedule == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Fuzzy Problems.";
			throw PostExecutionException("PostExecutionAnalyzer", errorMsg);
		}
		return schedule;
	}
	
	FJSP::ProblemFJSP* FJSPRobustnessAnalyzer::castProblem(FuzzyFW::Problem* problem) {
		FJSP::ProblemFJSP * problemFJSP =
			dynamic_cast<FJSP::ProblemFJSP *>(problem);
		if (problemFJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw PostExecutionException("PostExecutionAnalyzer", errorMsg);
		}
		return problemFJSP;
	}

	FuzzyFW::FitnessTFN * FJSPRobustnessAnalyzer::castFitness(FuzzyFW::Fitness* objective) {
		FuzzyFW::FitnessTFN * fitness =
			dynamic_cast<FuzzyFW::FitnessTFN *>(objective);
		if (fitness == NULL) {
			std::string errorMsg = "The Robustness analyzer can only use a FJSP tardiness evaluator to analyze FJSP tardiness.";
			throw PostExecutionException("PostExecutionAnalyzer", errorMsg);
		}
		return fitness;
	}

	
}