/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "RobustnessAnalyzer.h"

#include <iostream>
using namespace std;


namespace Robustness {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	RobustnessAnalyzer::RobustnessAnalyzer() {
		rnd.init(STURDINESS_INITIAL_SEED);
	}

	void RobustnessAnalyzer::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, FuzzyFW::Fitness* objective, const FuzzyFW::ParameterDB *params, int numRun) {
		IJSP::ScheduleIJSP * schedule =
			dynamic_cast<IJSP::ScheduleIJSP *>(solution);
		if (schedule == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw new IJSP::IJSPException("RobustnessAnalyzer", errorMsg);
		}
		IJSP::ProblemIJSP * problemIJSP =
			dynamic_cast<IJSP::ProblemIJSP *>(problem);
		if (problemIJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw new IJSP::IJSPException("RobustnessAnalyzer", errorMsg);
		}
		FuzzyFW::FitnessInterval * fitness =
			dynamic_cast<FuzzyFW::FitnessInterval *>(objective);
		if (fitness == NULL) {
			std::string errorMsg = "The Robustness analyzer can only use a IJSP tardiness evaluator to analyze IJSP tardiness.";
			throw new IJSP::IJSPException("RobustnessAnalyzer", errorMsg);
		}

		if (params->getStringUpper("objective").compare("IJSP.TARDINESS")==0) {
			
			this->analyzeTardiness(problemIJSP, schedule, fitness, params, numRun);
		}
		else this->analyzeMakespan(problemIJSP, schedule, fitness, params, numRun);
	}

	
	void RobustnessAnalyzer::analyzeTardiness(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness,  const FuzzyFW::ParameterDB *params, int numRun) {
		std::vector<unsigned int> mMkspan(problemIJSP->getNumberMachines());
		std::vector<unsigned int> jMkspan(problemIJSP->getNumberJobs());
		std::vector<int> taskOrder = schedule->getTaskOrder();
		double avg = 0;
		this->writer.write(STURDINESS_ROW_PREFIX + valueToString(numRun + 1));
		for (int rep = 0; rep < STURDINESS_NUM_ITERATIONS; rep++) {
			//Reset data structures to 0
			std::fill(mMkspan.begin(), mMkspan.end(), 0);
			std::fill(jMkspan.begin(), jMkspan.end(), 0);
			//We calculate random durations for each task and the makespan for each job and machine
			for (int i = 0; i < taskOrder.size(); i++) {
				const IJSP::TaskIJSP * task = problemIJSP->getTask(taskOrder[i]);
				int localMaxMkspan = std::max(mMkspan[task->machine], jMkspan[task->job]) + this->getRandomDuration(task, rep);
				mMkspan[task->machine] = localMaxMkspan;
				jMkspan[task->job] = localMaxMkspan;
			}
			
			//We get the tardiness
			double tardiness = 0;
			for (int i = 0; i < problemIJSP->getNumberJobs(); i++) {
				// Uncomment for CRISP duedates
				//const FuzzyFW::TimeWindowLinear* dueDate = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(problemIJSP->getTimeWindow(i));
				tardiness += std::max(0, (int)jMkspan[i] - (int)getDueDate(i, rep));
				//tardiness += std::max(0.0, (double)jMkspan[i] - (dueDate->d1+ dueDate->d2)/2);
			}
			avg += tardiness;
			this->writer.write(tardiness);
		}
		this->writer.write(double(avg) / STURDINESS_NUM_ITERATIONS);
		//Print fitness of the solution

		this->writer.write(fitness->getValue().a);
		this->writer.write(fitness->getValue().b);
		this->writer.endline();
	}

	void RobustnessAnalyzer::analyzeMakespan(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun) {
		std::vector<unsigned int> mMkspan(problemIJSP->getNumberMachines());
		std::vector<unsigned int> jMkspan(problemIJSP->getNumberJobs());
		std::vector<int> taskOrder = schedule->getTaskOrder();
		double avg = 0;
		this->writer.write(STURDINESS_ROW_PREFIX + valueToString(numRun + 1));
		for (int rep = 0; rep < STURDINESS_NUM_ITERATIONS; rep++) {
			//Reset data structures to 0
			std::fill(mMkspan.begin(), mMkspan.end(), 0);
			std::fill(jMkspan.begin(), jMkspan.end(), 0);
			//We calculate random durations for each task and the makespan for each job and machine
			for (int i = 0; i < taskOrder.size(); i++) {
				const IJSP::TaskIJSP * task = problemIJSP->getTask(taskOrder[i]);
				int localMaxMkspan = std::max(mMkspan[task->machine], jMkspan[task->job]) + this->getRandomDuration(task, rep);
				mMkspan[task->machine] = localMaxMkspan;
				jMkspan[task->job] = localMaxMkspan;
			}
			//We get the maximum makespan
			int maxMkspan = 0;
			for (int i = 0; i < problemIJSP->getNumberJobs(); i++) {
				if (jMkspan[i] > maxMkspan) {
					maxMkspan = jMkspan[i];
				}
			}
			avg += maxMkspan;
			this->writer.write(maxMkspan);

		}
		this->writer.write(double(avg) / STURDINESS_NUM_ITERATIONS);
		//Print fitness of the solution

		this->writer.write(fitness->getValue().a);
		this->writer.write(fitness->getValue().b);
		this->writer.endline();
	}


	unsigned int RobustnessAnalyzer::getRandomDuration(const IJSP::TaskIJSP* task, unsigned int rep) {
		return this->scenarioManager.getDuration(task->id, rep);
	}

	unsigned int RobustnessAnalyzer::getDueDate(const unsigned int numJob, unsigned int rep) {
		return this->scenarioManager.getDueDate(numJob, rep);
	}

	void RobustnessAnalyzer::open(FuzzyFW::Problem *problem, std::string outputPrefix, std::string signature) {
		IJSP::ProblemIJSP * problemIJSP =
			dynamic_cast<IJSP::ProblemIJSP *>(problem);
		if (problemIJSP == NULL) {
			std::string errorMsg = "The Robustness analyzer can be only used on Interval Problems.";
			throw new IJSP::IJSPException("RobustnessAnalyzer", errorMsg);
		}

		this->scenarioManager.init(problemIJSP, outputPrefix, STURDINESS_NUM_ITERATIONS);
		this->writer.open(outputPrefix +signature);
		this->writer.writeHeader(STURDINESS_NUM_ITERATIONS);
	}

	void RobustnessAnalyzer::close() {
		this->writer.close();
	}
}