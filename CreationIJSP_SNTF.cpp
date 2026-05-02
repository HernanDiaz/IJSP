/*
 * CreationIJSP_SNTF.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CreationIJSP_SNTF.h"
#include "CreationIJSP_helpers.h"
#include <cmath>

namespace IJSP {

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationSNTFIntervalMkSchedule::createIndividual(
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
	std::vector<FuzzyFW::Interval> nextTaskTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			//count first task duration for each job
			nextTaskTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, 0))->p;

			//Order the jobs from higher to lower first task time
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(nextTaskTime[i], nextTaskTime[jobOrderByRemainingTime[k]])) {
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

		// Pass to the next task of the job
		count[rand] = (*fuzzyProb)[count[rand]]->js;
		if (count[rand] < 0) {
			for (size_t i = rand + 1; i < count.size(); i++) {
				count[i - 1] = count[i];
				nextTaskTime[i - 1] = nextTaskTime[i];
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			nextTaskTime.pop_back();
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			count.pop_back();
		}
		else {
			// Update next task processing time
			nextTaskTime[rand] = (*fuzzyProb)[count[rand]]->p;
			// Update order of the jobs from higher to lower remaining time
			int job = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin()+k);
			//Order the jobs from higher to lower first task time
			int l = 0;
			while (l < jobOrderByRemainingTime.size() &&
				fiBetter(nextTaskTime[job], nextTaskTime[jobOrderByRemainingTime[l]])) {
				l++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + l, job);
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

} // namespace IJSP
