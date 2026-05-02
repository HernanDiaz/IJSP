/*
 * CreationIJSP_SPJFInverse.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CreationIJSP_SPJFInverse.h"
#include "CreationIJSP_helpers.h"
#include <cmath>

namespace IJSP {

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationSPJFInverseIntervalMkSchedule::createIndividual(
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
	std::vector<FuzzyFW::Interval> mMkspan(fuzzyProb->getNumberMachines());
	std::vector<FuzzyFW::Interval> jMkspan(fuzzyProb->getNumberJobs());

	// Find the first task of each job and count remaining time for each job
	std::vector<int> jobOrderByRemainingTime(fuzzyProb->getNumberJobs());
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
		}
		jobOrderByRemainingTime[i] = i;
	}
	// Build an array with repetitions
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = int(sqrt(rand));

		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		// Update machine and job makespan.
		const IJSP::TaskIJSP * task = fuzzyProb->getTask(taskid);
		FuzzyFW::Interval localMaxMkspan = maximum(mMkspan[task->machine], jMkspan[task->job], FuzzyFW::Interval::M_COMPONENT) + task->p;
		mMkspan[task->machine] = localMaxMkspan;
		jMkspan[task->job] = localMaxMkspan;

		//Order from better to worst
		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiWorse(jMkspan[jobOrderByRemainingTime[k]], jMkspan[jobOrderByRemainingTime[k + 1]])) {
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
			}
			for (size_t i = 0; i < jobOrderByRemainingTime.size(); i++) {
				if (jobOrderByRemainingTime[i] > rand) {
					jobOrderByRemainingTime[i]--;
				}
			}
			int job = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime.erase(jobOrderByRemainingTime.begin() + k);
			jMkspan.erase(jMkspan.begin() + k);
			mMkspan.erase(mMkspan.begin() + k);
			count.pop_back();
		}

	}
	sgs->buildSchedule(svars, permutation);
	indiv = svars->encoder->encode(sgs->getSchedule(), svars);
	indiv->updatePhenotype(sgs->getSchedule()->clone());

	return indiv;
}

} // namespace IJSP
