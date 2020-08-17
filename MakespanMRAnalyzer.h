/*
* SturdinessAnalyzer.h
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#pragma once
#include "heading.h"
#include "ScheduleIJSP.h"
#include "EvaluationIJSP.h"
#include "PostExecutionException.h"
#include "MakespanMRFileWriter.h"
#include "PostExecutionAnalyzer.h"


namespace PostExecution {
	   
class MakespanMRAnalyzer: public PostExecutionAnalyzer {
	//=====================================================================
	//		FIELDS
	//=====================================================================

public:
	/**
	* Default constructor
	*/
	MakespanMRFileWriter writer;

	MakespanMRAnalyzer();

	void close();

	void open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature);

	void analyzeObjectiveFunction(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun);

	void analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const  FuzzyFW::ParameterDB *params, int numRun);
	
	IJSP::ScheduleIJSP * castSchedule(FuzzyFW::Solution* solution);

	IJSP::ProblemIJSP* castProblem(FuzzyFW::Problem* problem);

	FuzzyFW::FitnessInterval* castFitness(FuzzyFW::Fitness* objective);

private:
	void calculateTailDistribution(int taskId, IJSP::ScheduleIJSP * schedule, std::vector<std::vector<double>*> & tailsDistro);

	std::vector<double> getPreviousTailDistribution(IJSP::ScheduleIJSP * schedule, const int taskId, std::vector<std::vector<double>*> & tailsDistro);

	std::vector<double> combineDistributions(const std::vector<double> & distA, const  std::vector<double> & distB, const FuzzyFW::Interval & tailA, const FuzzyFW::Interval & tailB);

	void calculateMakespanDistribution(const IJSP::ScheduleIJSP * schedule, const std::vector<std::vector<double>*> & tailsDistro, std::vector<double> & makespanDistro);

	std::vector<int> orderLastTasks(const IJSP::ScheduleIJSP * schedule);

};
}
