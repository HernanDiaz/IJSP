/*
 * CreationIJSP_LRTF.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CreationIJSP_LRTF.h"
#include "CreationIJSP_helpers.h"
#include <cmath>

namespace IJSP {

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationLRTFIntervalMkSchedule::createIndividual(
	const FuzzyFW::SharedVarsEvolutionary *svars) const {
	if (shouldUseRandom(svars))
		return CreationRandomSchedule::createIndividual(svars);
	int rand;
	std::vector<int> permutation, count;
	FuzzyFW::Individual * indiv;

	// Convert the problem type
	ProblemIJSP * fuzzyProb =
		dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This enconding function works only with ";
		errorMsg += "interval problems.";
		throw IJSPException("Creation", errorMsg);
	}

	// Find the first task of each job and count remaining time for each job
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			//count remaining time for each job
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			//Order the jobs from higher to lower remaining time
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	// Build an array with repetitions
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = max - int(sqrt(rand)) - 1;
		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		// Update remaining time for the job
		jobRemainingTime[rand] -= fuzzyProb->getTask(count[rand])->p;

		// Update order of the jobs from higher to lower remaining time
		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}
		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				jobRemainingTime[i - 1] = jobRemainingTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			jobRemainingTime.pop_back();
			jobOrderByRemainingTime.pop_back();
			count.pop_back();
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());
	return indiv;
}

} // namespace IJSP
