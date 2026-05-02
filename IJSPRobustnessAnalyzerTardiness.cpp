/*
* SturdinessAnalyzer.cpp
*
*  Created on: Nov 20, 2019
*      Author: Hernan Diaz Rodriguez
*/
#include "IJSPRobustnessAnalyzerTardiness.h"

namespace PostExecution {


	//=============================================================================
	//		CONSTRUCTORS / INITIALIZERS
	//=============================================================================
	//====  Default constructor  ==================================================
	IJSPRobustnessAnalyzerTardiness::IJSPRobustnessAnalyzerTardiness():IJSPRobustnessAnalyzer(){}

	void IJSPRobustnessAnalyzerTardiness::analyzeObjectiveFunction(const IJSP::ProblemIJSP *problemIJSP, IJSP::ScheduleIJSP * schedule, FuzzyFW::FitnessInterval * fitness, const FuzzyFW::ParameterDB *params, int numRun)
	{
		std::vector<unsigned int> mMkspan(problemIJSP->getNumberMachines());
		std::vector<unsigned int> jMkspan(problemIJSP->getNumberJobs());
		std::vector<int> taskOrder = schedule->getTaskOrder();
		long double avg = 0;
		this->writer.write(ROBUSTNESS_ROW_PREFIX + valueToString(numRun + 1));
		for (int rep = 0; rep < ROBUSTNESS_NUM_ITERATIONS; rep++) {
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
		this->writer.write(avg / ROBUSTNESS_NUM_ITERATIONS);
		//Print fitness of the solution

		this->writer.write(fitness->getValue().a);
		this->writer.write(fitness->getValue().b);

		/*
	   // Count (0,X) tardiness
	   int numJobs = 0;
	   for (unsigned int i = 0; i < problemIJSP->getNumberJobs(); i++) {
		   const FuzzyFW::TimeWindowLinear* dueDate = dynamic_cast<const FuzzyFW::TimeWindowLinear *>(problemIJSP->getTimeWindow(i));
		   if (dueDate == NULL) {
			   std::string errorMsg = "This evaluation function works only with ";
			   errorMsg += "Linear Timewidows.";
			   throw IJSP::IJSPException("EvaluationIJSP", errorMsg);
		   }

		   FuzzyFW::Interval I = FuzzyFW::Interval(std::max(0.0, schedule->getCTJob(i).a - dueDate->d2), std::max(0.0, schedule->getCTJob(i).b - dueDate->d1));
		   if (I.a == 0) numJobs++;
	   }
	   this->writer.write(problemIJSP->getNumberJobs());
	   this->writer.write(numJobs);
	   */
		this->writer.endline();

	}
}