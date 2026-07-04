/*
 * EvaluationIJSP_Energy.cpp
 *
 *  Created on: Jul 4, 2026
 *      Author: hdiaz
 */

#include "EvaluationIJSP_Energy.h"

namespace IJSP {

//=============================================================================
//		METHODS
//=============================================================================
//----- Compute the non-processing energy of a schedule  ----------------------
FuzzyFW::Interval EvaluationIJSP_Energy::computeNPE(
	const ScheduleIJSP *schedule, const ProblemIJSP *problem) {

	FuzzyFW::Interval npe(0, 0);

	for (size_t k = 0; k < schedule->lastTaskMachine.size(); k++) {
		int t = schedule->lastTaskMachine[k];
		if (t < 0)
			continue;

		// Sum the idle gaps between consecutive tasks on machine k,
		// component-wise (both are >= 0 in a semi-active component schedule)
		double gapLow = 0.0, gapUpp = 0.0;
		int mp = schedule->taskInfo[t].mp;
		while (mp != -1) {
			const ScheduledTaskInfo &cur = schedule->taskInfo[t];
			const ScheduledTaskInfo &prev = schedule->taskInfo[mp];
			gapLow += cur.head.a - (prev.head.a + prev.task->p.a);
			gapUpp += cur.head.b - (prev.head.b + prev.task->p.b);
			t = mp;
			mp = schedule->taskInfo[t].mp;
		}

		double power = problem->getPassivePower((unsigned int)k);
		npe.a += power * gapLow;
		npe.b += power * gapUpp;
	}
	return npe;
}


//----- Get Objective function  -----------------------------------------------
FuzzyFW::Objective * EvaluationIJSP_Energy::getObjectiveFunction(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;

	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Interval Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Interval problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	return new FuzzyFW::FitnessInterval(computeNPE(schedule, fuzzyProb), false);
}


//----- Evaluate  -------------------------------------------------------------
FuzzyFW::Fitness * EvaluationIJSP_Energy::evaluate(
	const FuzzyFW::SharedVarsEvolutionary * const svars,
	FuzzyFW::Individual *individual) const {

	FuzzyFW::Solution * solution;
	ScheduleIJSP * schedule;
	ProblemIJSP *fuzzyProb;

	if (individual->isPhenotypeUpdated())
		solution = individual->getPhenotype();
	else
		solution = svars->decoder->decode(individual, svars);

	schedule = dynamic_cast<ScheduleIJSP *>(solution);
	if (schedule == NULL) {
		std::string errorMsg = "This evaluation function is valid only ";
		errorMsg += "for Interval Job Shop Problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	fuzzyProb = dynamic_cast<ProblemIJSP *>(svars->problem);
	if (fuzzyProb == NULL) {
		std::string errorMsg = "This evaluation function works only with ";
		errorMsg += "Interval problems.";
		throw IJSPException("Evaluation", errorMsg);
	}

	if (this->lamarckism)
		svars->encoder->encode(schedule, individual, svars);
	if (!individual->isPhenotypeUpdated())
		individual->updatePhenotype(schedule->clone());

	return new FuzzyFW::FitnessInterval(computeNPE(schedule, fuzzyProb), false);
}

} // namespace IJSP
