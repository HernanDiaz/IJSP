/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "RobustnessAnalyzer.h"




namespace Robustness {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	RobustnessAnalyzer::RobustnessAnalyzer() {
		rnd.init(STURDINESS_INITIAL_SEED);
	}

	void RobustnessAnalyzer::analyze(FuzzyFW::Problem *problem, FuzzyFW::Solution * solution, const FuzzyFW::ParameterDB *params, int numRun) {
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

		std::vector<unsigned int> mMkspan(problemIJSP->getNumberMachines());
		std::vector<unsigned int> jMkspan(problemIJSP->getNumberJobs());
		//change solution for taskOrder as parameter
		std::vector<int> taskOrder = schedule->getTaskOrder();
		double avg = 0;
		this->writer.write(STURDINESS_ROW_PREFIX + valueToString(numRun +1));
		for (int rep = 0; rep < STURDINESS_NUM_ITERATIONS; rep++) {
			//Reset data structures to 0
			std::fill(mMkspan.begin(), mMkspan.end(), 0);
			std::fill(jMkspan.begin(), jMkspan.end(), 0);
			//We calculate random durations for each task and the makespan for each job and machine
			for (int i = 0; i < taskOrder.size(); i++) {
				const IJSP::TaskIJSP * task = problemIJSP->getTask(taskOrder[i]);
				int localMaxMkspan = std::max(mMkspan[task->machine], jMkspan[task->job]) + this->getRandomDuration(task);
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
		this->writer.write(double(avg)/STURDINESS_NUM_ITERATIONS);
		this->writer.endline();
	}


	int RobustnessAnalyzer::getRandomDuration(const IJSP::TaskIJSP* task) {
		return this->rnd.getInteger(task->p.a, task->p.b);
	}

	void RobustnessAnalyzer::open(std::string outputName) {
		this->writer.open(outputName);
		this->writer.writeHeader(STURDINESS_NUM_ITERATIONS);
	}

	void RobustnessAnalyzer::close() {
		this->writer.close();
	}
}