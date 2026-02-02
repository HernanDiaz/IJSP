/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "FJSPRobustnessAnalyzerMakespan.h"

namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	FJSPRobustnessAnalyzerMakespan::FJSPRobustnessAnalyzerMakespan():FJSPRobustnessAnalyzer(){}

	void FJSPRobustnessAnalyzerMakespan::analyzeObjectiveFunction(const FJSP::ProblemFJSP *problemFJSP, FJSP::ScheduleFJSP * schedule, FuzzyFW::FitnessTFN * fitness, const FuzzyFW::ParameterDB *params, int numRun)
	{
		std::vector<unsigned int> mMkspan(problemFJSP->getNumberMachines());
		std::vector<unsigned int> jMkspan(problemFJSP->getNumberJobs());
		std::vector<int> taskOrder = schedule->getTaskOrder();
		double avg = 0;
		this->writer.write(ROBUSTNESS_ROW_PREFIX + valueToString(numRun + 1));
		for (int rep = 0; rep < ROBUSTNESS_NUM_ITERATIONS; rep++) {
			//Reset data structures to 0
			std::fill(mMkspan.begin(), mMkspan.end(), 0);
			std::fill(jMkspan.begin(), jMkspan.end(), 0);
			//We calculate random durations for each task and the makespan for each job and machine
			for (int i = 0; i < taskOrder.size(); i++) {
				const FJSP::TaskFJSP * task = problemFJSP->getTask(taskOrder[i]);
				int localMaxMkspan = std::max(mMkspan[task->machine], jMkspan[task->job]) + this->getRandomDuration(task, rep);
				mMkspan[task->machine] = localMaxMkspan;
				jMkspan[task->job] = localMaxMkspan;
			}
			//We get the maximum makespan
			int maxMkspan = 0;
			for (int i = 0; i < problemFJSP->getNumberJobs(); i++) {
				if (jMkspan[i] > maxMkspan) {
					maxMkspan = jMkspan[i];
				}
			}
			avg += maxMkspan;
			this->writer.write(maxMkspan);

		}
		this->writer.write(double(avg) / ROBUSTNESS_NUM_ITERATIONS);
		//Print fitness of the solution

		this->writer.write(fitness->getValue().a);
		this->writer.write(fitness->getValue().b);
		this->writer.write(fitness->getValue().c);
		this->writer.endline();

	}
}