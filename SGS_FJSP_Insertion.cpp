/*
* SGS_FJSP_Insertion.cpp
*
*  Created on: June 1, 2017
*      Author: Juan Jose Palacios
*/

#include "SGS_FJSP_Insertion.h"

namespace FJSP {

//=============================================================================
//
//	Class SGS_FJSP_Insertion
//
//=============================================================================
//=============================================================================
//		METHODS
//=============================================================================
//=====  Build schedule  ======================================================
ScheduleFJSP * SGS_FJSP_Insertion::buildSchedule(
	const FuzzyFW::SharedVars * svars, std::vector<int> &order) {

	if (svars->problem == NULL)
		throw new FJSPException("SGS", "Problem instance not created");

	ProblemFJSP * fjspProb =
		dynamic_cast<ProblemFJSP *>(svars->problem);
	if (fjspProb == NULL) {
		std::string errorMsg = "This SGS can be only used on Fuzzy Problems.";
		throw new FJSPException("SGS", errorMsg);
	}

	if (this->isCreated)
		this->schedule->reset();
	else {
		this->schedule = new ScheduleFJSP(fjspProb);
		this->isCreated = true;
	}

	for (size_t i = 0; i < order.size(); i++) {
		this->scheduleTask((*fjspProb)[order[i]], order[i]);
	}

	return this->schedule;
}


//=====  Schedule a task  =====================================================
FuzzyFW::TFN SGS_FJSP_Insertion::scheduleTask(const TaskFJSP *task,
	const int taskIdx) {

	int mp, ms; // Machine predecessor and successor
	FuzzyFW::TFN Stime;	// Starting time
	FuzzyFW::TFN mtHead, mtPT;
	char found;	// Big gap found in the schedule
	// Comparisons are made component by component
	FuzzyFW::TFN::Compare cpComp = FuzzyFW::TFN::C_COMPONENT;
	FuzzyFW::TFN::Maximum maxComp = FuzzyFW::TFN::M_COMPONENT;

	int mac = task->machine;
	int job = task->job;

	// Check if this task can be scheduled
	if (task->jp != this->schedule->lastTaskJob[job]) {
		std::string errorMsg;
		errorMsg = "Job precedence constraint is being violated. Scheduling ";
		errorMsg += "task " + valueToString(taskIdx) + " after task ";
		errorMsg += valueToString(this->schedule->lastTaskJob[job]);
		throw new FJSPException("SGS", errorMsg);
	}

	// If I could schedule the task just after its job predecessor, who
	// would be the machine predecessor and successors...
	Stime = this->schedule->getCTJob(job);
	ms = -1;
	mp = this->schedule->lastTaskMachine[mac];
	if (mp != -1)
		mtHead = this->schedule->taskInfo[mp].head;

	while (mp != -1 && mtHead.isGreaterEqualTo(Stime, cpComp)) {
		ms = mp;
		mp = this->schedule->taskInfo[ms].mp;
		if (mp != -1)
			mtHead = this->schedule->taskInfo[mp].head;
	}

	// Update the heuristic Starting time that task could take
	if (mp != -1) {
		mtHead = this->schedule->taskInfo[mp].head;
		mtPT = this->schedule->taskInfo[mp].task->p;
		Stime = maximum(Stime, mtHead + mtPT, maxComp);
	}

	// Look for the actual minimum starting time for the task
	found = false;
	while (!found && ms != -1) {
		mtHead = this->schedule->taskInfo[ms].head;
		mtPT = this->schedule->taskInfo[ms].task->p;

		if (mtHead.isGreaterEqualTo(Stime + task->p, cpComp))
			found = true;
		else {
			mp = ms;
			Stime = maximum(Stime, mtHead + mtPT, maxComp);
			ms = this->schedule->taskInfo[mp].ms;
		}
	}

	// Update the schedule
	this->schedule->addTask(taskIdx, Stime, ms);
	return Stime;
}

}
