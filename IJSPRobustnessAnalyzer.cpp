/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "IJSPRobustnessAnalyzer.h"


namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	IJSPRobustnessAnalyzer::IJSPRobustnessAnalyzer() {
		rnd.init(ROBUSTNESS_INITIAL_SEED);
	}
	
	unsigned int IJSPRobustnessAnalyzer::getRandomDuration(const IJSP::TaskIJSP* task, unsigned int rep) {
		return this->scenarioManager.getDuration(task->id, rep);
	}

	unsigned int IJSPRobustnessAnalyzer::getDueDate(const unsigned int numJob, unsigned int rep) {
		return this->scenarioManager.getDueDate(numJob, rep);
	}

	void IJSPRobustnessAnalyzer::close() {
		this->writer.close();
	}

	void IJSPRobustnessAnalyzer::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun) {
		this->analyzeObjectiveFunction(
			this->castProblem(problem),
			this->castSchedule(solution),
			this->castFitness(objective),
			params,
			numRun
		);
	}

	void IJSPRobustnessAnalyzer::open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature) {
		IJSP::ProblemIJSP * problemIJSP =
			dynamic_cast<IJSP::ProblemIJSP *>(problem);
		if (problemIJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw IJSP::IJSPException("PostExecutionAnalyzer", errorMsg);
		}

		this->scenarioManager.init(problemIJSP, outputPrefix, ROBUSTNESS_NUM_ITERATIONS);
		this->writer.open(outputPrefix + signature);
		this->writer.writeHeader(ROBUSTNESS_NUM_ITERATIONS);
	}
	
	IJSP::ScheduleIJSP * IJSPRobustnessAnalyzer::castSchedule(FuzzyFW::Solution* solution) {
		IJSP::ScheduleIJSP * schedule =
			dynamic_cast<IJSP::ScheduleIJSP *>(solution);
		if (schedule == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw PostExecutionException("PostExecutionAnalyzer", errorMsg);
		}
		return schedule;
	}
	
	IJSP::ProblemIJSP* IJSPRobustnessAnalyzer::castProblem(FuzzyFW::Problem* problem) {
		IJSP::ProblemIJSP * problemIJSP =
			dynamic_cast<IJSP::ProblemIJSP *>(problem);
		if (problemIJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw PostExecutionException("PostExecutionAnalyzer", errorMsg);
		}
		return problemIJSP;
	}

	FuzzyFW::FitnessInterval * IJSPRobustnessAnalyzer::castFitness(FuzzyFW::Fitness* objective) {
		FuzzyFW::FitnessInterval * fitness =
			dynamic_cast<FuzzyFW::FitnessInterval *>(objective);
		if (fitness == NULL) {
			std::string errorMsg = "The Robustness analyzer can only use a IJSP tardiness evaluator to analyze IJSP tardiness.";
			throw PostExecutionException("PostExecutionAnalyzer", errorMsg);
		}
		return fitness;
	}

	
}