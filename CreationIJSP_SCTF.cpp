/*
 * CreationIJSP_SCTF.cpp
 *
 *  Created on: June 25, 2019
 *      Author: hdiaz
 */

#include "CreationIJSP_SCTF.h"
#include "CreationIJSP_helpers.h"
#include <cmath>

namespace IJSP {

//=============================================================================
//		METHODS
//=============================================================================
//-----  create Individual  ---------------------------------------------------
FuzzyFW::Individual * CreationSCTFIntervalMkSchedule::createIndividual(
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
	std::vector<FuzzyFW::Interval> jobRemainingTime(fuzzyProb->getNumberJobs());
	std::vector<int> jobOrderByRemainingTime;
	for (unsigned int i = 0; i < fuzzyProb->getNumberJobs(); i++) {
		unsigned int numberOfTasks = fuzzyProb->getNumberTasks(i);
		if (numberOfTasks > 0) {
			count.push_back(fuzzyProb->getTaskId(i, 0));
			for (unsigned int j = 0; j < numberOfTasks; j++) {
				jobRemainingTime[i] += fuzzyProb->getTask(fuzzyProb->getTaskId(i, j))->p;
			}
			int k = 0;
			while (k < jobOrderByRemainingTime.size() &&
				fiBetter(jobRemainingTime[i], jobRemainingTime[jobOrderByRemainingTime[k]])) {
				k++;
			}
			jobOrderByRemainingTime.insert(jobOrderByRemainingTime.begin() + k, i);
		}
	}
	while (count.size() > 0) {
		int max = 3 < count.size() ? 3 : count.size();
		rand = svars->rng->getInteger(0, max*max - 1);
		int k = int(sqrt(rand));
		rand = jobOrderByRemainingTime[k];
		int taskid = count[rand];
		permutation.push_back(taskid);

		jobRemainingTime[rand] -= fuzzyProb->getTask(taskid)->p;

		const IJSP::TaskIJSP * task = fuzzyProb->getTask(taskid);
		FuzzyFW::Interval localMaxMkspan = maximum(mMkspan[task->machine], jMkspan[task->job], FuzzyFW::Interval::M_COMPONENT) + task->p;
		mMkspan[task->machine] = localMaxMkspan;
		jMkspan[task->job] = localMaxMkspan;

		while (k < jobOrderByRemainingTime.size() - 1 &&
			fiBetter(jobRemainingTime[jobOrderByRemainingTime[k]] + jMkspan[jobOrderByRemainingTime[k]], jobRemainingTime[jobOrderByRemainingTime[k + 1]] + jMkspan[jobOrderByRemainingTime[k + 1]])) {
			int aux = jobOrderByRemainingTime[k];
			jobOrderByRemainingTime[k] = jobOrderByRemainingTime[k + 1];
			jobOrderByRemainingTime[k + 1] = aux;
			k++;
		}

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
